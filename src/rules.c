/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       rules.c
 Description:  implements rule-based controls
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 05/15/2019
 ******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "funcs.h"
#include "hash.h"
#include "text.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

enum Rulewords {
  r_RULE,
  r_IF,
  r_AND,
  r_OR,
  r_THEN,
  r_ELSE,
  r_PRIORITY,
  r_ERROR
};
char *Ruleword[] = {w_RULE, w_IF,   w_AND,      w_OR,
                    w_THEN, w_ELSE, w_PRIORITY, NULL};

enum Varwords {
  r_DEMAND,
  r_HEAD,
  r_GRADE,
  r_LEVEL,
  r_PRESSURE,
  r_FLOW,
  r_STATUS,
  r_SETTING,
  r_POWER,
  r_TIME,
  r_CLOCKTIME,
  r_FILLTIME,
  r_DRAINTIME
};
char *Varword[] = {w_DEMAND,    w_HEAD,     w_GRADE,     w_LEVEL, w_PRESSURE,
                   w_FLOW,      w_STATUS,   w_SETTING,   w_POWER, w_TIME,
                   w_CLOCKTIME, w_FILLTIME, w_DRAINTIME, NULL};

enum Objects {
  r_JUNC,
  r_RESERV,
  r_TANK,
  r_PIPE,
  r_PUMP,
  r_VALVE,
  r_NODE,
  r_LINK,
  r_SYSTEM
};
char *Object[] = {w_JUNC,  w_RESERV, w_TANK, w_PIPE,   w_PUMP,
                  w_VALVE, w_NODE,   w_LINK, w_SYSTEM, NULL};

// NOTE: place "<=" & ">=" before "<" & ">" so that findmatch() works correctly.
enum Operators { EQ, NE, LE, GE, LT, GT, IS, NOT, BELOW, ABOVE };
char *Operator[] = {"=",  "<>",  "<=",    ">=",    "<", ">",
                    w_IS, w_NOT, w_BELOW, w_ABOVE, NULL};

enum Values { IS_NUMBER, IS_OPEN, IS_CLOSED, IS_ACTIVE };
char *Value[] = {"XXXX", w_OPEN, w_CLOSED, w_ACTIVE, NULL};

// Local functions
static void newrule(Project *);
static int  newpremise(Project *, int);
static int  newaction(Project *);
static int  newpriority(Project *);

static int  evalpremises(Project *, int);
static int  checkpremise(Project *, Spremise *);
static int  checktime(Project *, Spremise *);
static int  checkstatus(Project *, Spremise *);
static int  checkvalue(Project *, Spremise *);

static int  onactionlist(Project *, int, Saction *);
static void updateactionlist(Project *, int, Saction *);
static int  takeactions(Project *);
static void clearactionlist(Rules *);
static void clearrule(Project *, int);

static void writepremise(Spremise *, FILE *, Network *);
static void writeaction(Saction *, FILE *, Network *);
static void getobjtxt(int, int, char *);
static void gettimetxt(double, char *);


void initrules(Project *pr)
//--------------------------------------------------------------
//    Initializes rule base.
//--------------------------------------------------------------
{
    pr->rules.RuleState = r_PRIORITY;
    pr->rules.LastPremise = NULL;
    pr->rules.LastThenAction = NULL;
    pr->rules.LastElseAction = NULL;
    pr->rules.ActionList = NULL;
    pr->network.Rule = NULL;
}

void addrule(Parser *parser, char *tok)
//--------------------------------------------------------------
//    Updates rule count if RULE keyword found in line of input.
//--------------------------------------------------------------
{
    if (match(tok, w_RULE)) parser->MaxRules++;
}

void deleterule(Project *pr, int index)
//-----------------------------------------------------------
//    Deletes a specific rule
//-----------------------------------------------------------
{
    Network *net = &pr->network;

    int i;
    Srule *lastRule;

    // Free memory allocated to rule's premises & actions
    clearrule(pr, index);

    // Shift position of higher indexed rules down one
    for (i = index; i <= net->Nrules - 1; i++)
    {
        net->Rule[i] = net->Rule[i + 1];
    }

    // Remove premises & actions from last (inactive) entry in Rule array
    lastRule = &net->Rule[net->Nrules];
    lastRule->Premises = NULL;
    lastRule->ThenActions = NULL;
    lastRule->ElseActions = NULL;

    // Reduce active rule count by one
    net->Nrules--;
}

int allocrules(Project *pr)
//--------------------------------------------------------------
//    Allocates memory for rule-based controls.
//--------------------------------------------------------------
{
    Network *net = &pr->network;
    int n = pr->parser.MaxRules + 1;

    net->Rule = (Srule *)calloc(n, sizeof(Srule));
    if (net->Rule == NULL) return 101;
    return 0;
}

void freerules(Project *pr)
//--------------------------------------------------------------
//    Frees memory used for rule-based controls.
//--------------------------------------------------------------
{
    int i;

    // Already freed
    if (pr->network.Rule == NULL)
        return;

    for (i = 1; i <= pr->network.Nrules; i++) clearrule(pr, i);
    free(pr->network.Rule);
    pr->network.Rule = NULL;
}

int ruledata(Project *pr)
//--------------------------------------------------------------
//    Parses a line from [RULES] section of input.
//--------------------------------------------------------------
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Rules   *rules = &pr->rules;

    int key,      // Keyword code
        err;
    char **Tok = parser->Tok;  // Tokenized line of a rule statement

    // Exit if current rule has an error
    if (rules->RuleState == r_ERROR) return 0;

    // Find the key word that begins the rule statement
    err = 0;
    key = findmatch(Tok[0], Ruleword);
    switch (key)
    {
        case -1:
          err = 201;   // Unrecognized keyword
          break;

        case r_RULE:
          // Missing the rule label
          if (parser->Ntokens != 2)
          {
              err = 201;
              break;
          }
          net->Nrules++;
          newrule(pr);
          rules->RuleState = r_RULE;
          rules->Errcode = 0;
          break;

        case r_IF:
          if (rules->RuleState != r_RULE)
          {
              err = 221;   // Mis-placed IF clause
              break;
          }
          rules->RuleState = r_IF;
          err = newpremise(pr, r_AND);
          break;

        case r_AND:
          if (rules->RuleState == r_IF) err = newpremise(pr, r_AND);
          else if (rules->RuleState == r_THEN || rules->RuleState == r_ELSE)
          {
              err = newaction(pr);
          }
          else err = 221;
          break;

        case r_OR:
          if (rules->RuleState == r_IF) err = newpremise(pr, r_OR);
          else err = 221;
          break;

        case r_THEN:
          if (rules->RuleState != r_IF)
          {
              err = 221;   // Mis-placed THEN clause
              break;
          }
          rules->RuleState = r_THEN;
          err = newaction(pr);
          break;

        case r_ELSE:
          if (rules->RuleState != r_THEN)
          {
            err = 221;   // Mis-placed ELSE clause
            break;
          }
          rules->RuleState = r_ELSE;
          err = newaction(pr);
          break;

        case r_PRIORITY:
          if (rules->RuleState != r_THEN && rules->RuleState != r_ELSE)
          {
              err = 221;
              break;
          }
          rules->RuleState = r_PRIORITY;
          err = newpriority(pr);
          break;

        default:
          err = 201;
    }

    // Set RuleState to r_ERROR if errors found
    if (err)
    {
        rules->RuleState = r_ERROR;
        rules->Errcode = err;
        err = 200;
    }
    return err;
}

void ruleerrmsg(Project *pr)
//-----------------------------------------------------------
//    Report a rule parsing error message
//-----------------------------------------------------------
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Rules   *rules = &pr->rules;

    int i;
    char label[MAXMSG + 1];
    char msg[MAXLINE + 1];
    char **Tok = parser->Tok;

    // Get text of error message
    switch (rules->Errcode)
    {
    case 201: strcpy(msg, R_ERR201); break;
    case 202: strcpy(msg, R_ERR202); break;
    case 203: strcpy(msg, R_ERR203); break;
    case 204: strcpy(msg, R_ERR204); break;
    case 207: strcpy(msg, R_ERR207); break;
    case 221: strcpy(msg, R_ERR221); break;
    default: return;
    }

    // Get label of rule being parsed
    if (net->Nrules > 0)
    {
        strncpy(label, t_RULE, MAXMSG);
        strncat(label, " ", MAXMSG);
        strncat(label, net->Rule[net->Nrules].label, MAXMSG);
    }
    else strncpy(label, t_RULES_SECT, MAXMSG);

    // Write rule label and error message to status report
    snprintf(pr->Msg, MAXMSG, "%s", msg);
    strncat(pr->Msg, label, MAXMSG);
    strncat(pr->Msg, ":", MAXMSG);
    writeline(pr, pr->Msg);

    // Write text of rule clause being parsed to status report
    strcpy(msg, Tok[0]);
    for (i = 1; i < parser->Ntokens; i++)
    {
        strncat(msg, " ", MAXLINE);
        strncat(msg, Tok[i], MAXLINE);
    }
    writeline(pr, msg);
}

void adjustrules(Project *pr, int objtype, int index)
//-----------------------------------------------------------
//    Adjusts rules when a specific node or link is deleted.
//-----------------------------------------------------------
{
    Network *net = &pr->network;

    int i, delete;
    Spremise *p;
    Saction *a;

    // Delete rules that refer to objtype and index
    for (i = net->Nrules; i >= 1; i--)
    {
        delete = FALSE;
        p = net->Rule[i].Premises;
        while (p != NULL && !delete)
        {
            if (objtype == p->object && p->index == index) delete = TRUE;
            p = p->next;
        }
        if (objtype == r_LINK)
        {
            a = net->Rule[i].ThenActions;
            while (a != NULL && !delete)
            {
                if (a->link == index) delete = TRUE;
                a = a->next;
            }
            a = net->Rule[i].ElseActions;
            while (a != NULL && !delete)
            {
                if (a->link == index) delete = TRUE;
                a = a->next;
            }
        }
        if (delete) deleterule(pr, i);
    }

    // Adjust all higher object indices to reflect deletion of object index
    for (i = 1; i <= net->Nrules; i++)
    {
        p = net->Rule[i].Premises;
        while (p != NULL)
        {
            if (objtype == p->object && p->index > index) p->index--;
            p = p->next;
        }
        if (objtype == r_LINK)
        {
            a = net->Rule[i].ThenActions;
            while (a != NULL)
            {
                if (a->link > index) a->link--;
                a = a->next;
            }
            a = net->Rule[i].ElseActions;
            while (a != NULL)
            {
                if (a->link > index) a->link--;
                a = a->next;
            }
        }
    }
}

void adjusttankrules(Project *pr)
//-----------------------------------------------------------
//    Adjusts tank indices in rule premises.
//-----------------------------------------------------------
{
    Network *net = &pr->network;

    int i, njuncs;
    Spremise *p;

    njuncs = net->Njuncs;
    for (i = 1; i <= net->Nrules; i++)
    {
        p = net->Rule[i].Premises;
        while (p != NULL)
        {
            if (p->object == r_NODE && p->index > njuncs) p->index++;
            p = p->next;
        }
    }
}

Spremise *getpremise(Spremise *premises, int i)
//----------------------------------------------------------
//    Return the i-th premise in a rule
//----------------------------------------------------------
{
    int count = 0;
    Spremise *p;

    p = premises;
    while (p != NULL)
    {
        count++;
        if (count == i) break;
        p = p->next;
    }
    return p;
}

Saction *getaction(Saction *actions, int i)
//----------------------------------------------------------
//    Return the i-th action from a rule's action list
//----------------------------------------------------------
{
    int count = 0;
    Saction *a;

    a = actions;
    while (a != NULL)
    {
        count++;
        if (count == i) break;
        a = a->next;
    }
    return a;
}

int writerule(Project *pr, FILE *f, int ruleIndex)
//-----------------------------------------------------------------------------
//  Write a rule to an INP file.
//-----------------------------------------------------------------------------
{
    Network  *net = &pr->network;

    Srule      *rule = &net->Rule[ruleIndex];
    Spremise   *p;
    Saction    *a;

    // Write each premise clause to the file
    p = rule->Premises;
    fprintf(f, "\nIF   ");
    while (p != NULL)
    {
        writepremise(p, f, net);
        p = p->next;
        if (p) fprintf(f, "\n%-5s", Ruleword[p->logop]);
    }

    // Write each THEN action clause to the file
    a = rule->ThenActions;
    if (a) fprintf(f, "\nTHEN ");
    while (a != NULL)
    {
        writeaction(a, f, net);
        a = a->next;
        if (a) fprintf(f, "\nAND  ");
    }

    // Write each ELSE action clause to the file
    a = rule->ElseActions;
    if (a) fprintf(f, "\nELSE ");
    while (a != NULL)
    {
        writeaction(a, f, net);
        a = a->next;
        if (a) fprintf(f, "\nAND  ");
    }

    // Write the rule's priority to the file
    if (rule->priority > 0) fprintf(f, "\nPRIORITY %f", rule->priority);
    return 0;
}

int checkrules(Project *pr, long dt)
//-----------------------------------------------------
//    Checks which rules should fire at current time.
//-----------------------------------------------------
{
    Network *net = &pr->network;
    Times   *time = &pr->times;
    Rules   *rules = &pr->rules;

    int i;
    int actionCount = 0;    // Number of actions actually taken

                            // Start of rule evaluation time interval
    rules->Time1 = time->Htime - dt + 1;

    // Iterate through each rule
    rules->ActionList = NULL;
    for (i = 1; i <= net->Nrules; i++)
    {
        // If premises true, add THEN clauses to action list
        if (evalpremises(pr, i) == TRUE)
        {
            updateactionlist(pr, i, net->Rule[i].ThenActions);
        }

        // If premises false, add ELSE actions to list
        else
        {
            if (net->Rule[i].ElseActions != NULL)
            {
                updateactionlist(pr, i, net->Rule[i].ElseActions);
            }
        }
    }

    // Execute actions then clear action list
    if (rules->ActionList != NULL) actionCount = takeactions(pr);
    clearactionlist(rules);
    return actionCount;
}

void newrule(Project *pr)
//----------------------------------------------------------
//    Adds a new rule to the project
//----------------------------------------------------------
{
    Network *net = &pr->network;

    char **Tok = pr->parser.Tok;
    Srule *rule = &net->Rule[net->Nrules];

    strncpy(rule->label, Tok[1], MAXID);
    rule->Premises = NULL;
    rule->ThenActions = NULL;
    rule->ElseActions = NULL;
    rule->priority = 0.0;
    pr->rules.LastPremise = NULL;
    pr->rules.LastThenAction = NULL;
    pr->rules.LastElseAction = NULL;
}

int newpremise(Project *pr, int logop)
//--------------------------------------------------------------------
//   Adds new premise to current rule.
//   Formats are:
//     IF/AND/OR <object> <id> <variable> <operator> <value>
//     IF/AND/OR  SYSTEM <variable> <operator> <value> (units)
//---------------------------------------------------------------------
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Rules   *rules = &pr->rules;

    int i, j, k, m, r, s, v;
    double x;
    char **Tok = parser->Tok;
    Spremise *p;

    // Check for correct number of tokens
    if (parser->Ntokens != 5 && parser->Ntokens != 6) return 201;

    // Find network object & id if present
    i = findmatch(Tok[1], Object);
    if (i == r_SYSTEM)
    {
        j = 0;
        v = findmatch(Tok[2], Varword);
        if (v != r_DEMAND && v != r_TIME && v != r_CLOCKTIME) return 201;
    }
    else
    {
        v = findmatch(Tok[3], Varword);
        if (v < 0) return (201);
        switch (i)
        {
          case r_NODE:
          case r_JUNC:
          case r_RESERV:
          case r_TANK:
            k = r_NODE;
            break;
          case r_LINK:
          case r_PIPE:
          case r_PUMP:
          case r_VALVE:
            k = r_LINK;
            break;
          default:
            return 201;
        }
        i = k;
        if (i == r_NODE)
        {
            j = findnode(net, Tok[2]);
            if (j == 0) return 203;
            switch (v)
            {
              case r_DEMAND:
              case r_HEAD:
              case r_GRADE:
              case r_LEVEL:
              case r_PRESSURE:
                break;
              case r_FILLTIME:
              case r_DRAINTIME:
                if (j <= net->Njuncs) return 201;
                 break;
              default:
                return 201;
            }
        }
        else
        {
            j = findlink(net, Tok[2]);
            if (j == 0) return 204;
            switch (v)
            {
              case r_FLOW:
              case r_STATUS:
              case r_SETTING:
                break;
              default:
                return 201;
            }
        }
    }

    // Parse relational operator (r) and check for synonyms
    if (i == r_SYSTEM) m = 3;
    else m = 4;
    k = findmatch(Tok[m], Operator);
    if (k < 0) return 201;
    switch (k)
    {
      case IS:
        r = EQ;
        break;
      case NOT:
        r = NE;
        break;
      case BELOW:
        r = LT;
        break;
      case ABOVE:
        r = GT;
        break;
      default:
        r = k;
    }

    // Parse for status (s) or numerical value (x)
    s = 0;
    x = MISSING;
    if (v == r_TIME || v == r_CLOCKTIME)
    {
        if (parser->Ntokens == 6) x = hour(Tok[4], Tok[5]) * 3600.;
        else                      x = hour(Tok[4], "") * 3600.;
        if (x < 0.0) return 202;
    }
    else if ((k = findmatch(Tok[parser->Ntokens - 1], Value)) > IS_NUMBER) s = k;
    else
    {
        if (!getfloat(Tok[parser->Ntokens - 1], &x))
        return (202);
        if (v == r_FILLTIME || v == r_DRAINTIME) x = x * 3600.0;
    }

    // Create new premise structure
    p = (Spremise *)malloc(sizeof(Spremise));
    if (p == NULL) return 101;
    p->object = i;
    p->index = j;
    p->variable = v;
    p->relop = r;
    p->logop = logop;
    p->status = s;
    p->value = x;

    // Add premise to current rule's premise list
    p->next = NULL;
    if (rules->LastPremise == NULL) net->Rule[net->Nrules].Premises = p;
    else rules->LastPremise->next = p;
    rules->LastPremise = p;
    return 0;
}

int newaction(Project *pr)
//----------------------------------------------------------
//   Adds new action to current rule.
//   Format is:
//      THEN/ELSE/AND LINK <id> <variable> IS <value>
//----------------------------------------------------------
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Rules   *rules = &pr->rules;

    int j, k, s;
    double x;
    Saction *a;
    char **Tok = parser->Tok;

    // Check for correct number of tokens
    if (parser->Ntokens != 6) return 201;

    // Check that link exists
    j = findlink(net, Tok[2]);
    if (j == 0) return 204;

    // Cannot control a CV
    if (net->Link[j].Type == CVPIPE) return 207;

    // Find value for status or setting
    s = -1;
    x = MISSING;
    if ((k = findmatch(Tok[5], Value)) > IS_NUMBER) s = k;
    else
    {
        if (!getfloat(Tok[5], &x)) return 202;
        if (x < 0.0) return 202;
    }

    // Cannot change setting for a GPV
    if (x != MISSING && net->Link[j].Type == GPV) return 202;

    // Set status for pipe in case setting was specified
    if (x != MISSING && net->Link[j].Type == PIPE)
    {
        if (x == 0.0) s = IS_CLOSED;
        else          s = IS_OPEN;
        x = MISSING;
    }

    // Create a new action structure
    a = (Saction *)malloc(sizeof(Saction));
    if (a == NULL) return 101;
    a->link = j;
    a->status = s;
    a->setting = x;

    // Add action to current rule's action list
    if (rules->RuleState == r_THEN)
    {
        a->next = NULL;
        if (rules->LastThenAction == NULL)
        {
            net->Rule[net->Nrules].ThenActions = a;
        }
        else rules->LastThenAction->next = a;
        rules->LastThenAction = a;
    }
    else
    {
        a->next = NULL;
        if (rules->LastElseAction == NULL)
        {
            net->Rule[net->Nrules].ElseActions = a;
        }
        else rules->LastElseAction->next = a;
        rules->LastElseAction = a;
    }
    return 0;
}

int newpriority(Project *pr)
//---------------------------------------------------
//    Adds priority rating to current rule
//---------------------------------------------------
{
    Network *net = &pr->network;

    double x;
    char **Tok = pr->parser.Tok;

    if (!getfloat(Tok[1], &x)) return 202;
    net->Rule[net->Nrules].priority = x;
    return 0;
}

int evalpremises(Project *pr, int i)
//----------------------------------------------------------
//    Checks if premises to rule i are true
//----------------------------------------------------------
{
    Network *net = &pr->network;

    int result;
    Spremise *p;

    result = TRUE;
    p = net->Rule[i].Premises;
    while (p != NULL)
    {
        if (p->logop == r_OR)
        {
            if (result == FALSE) result = checkpremise(pr, p);
        }
        else
        {
            if (result == FALSE) return (FALSE);
            result = checkpremise(pr, p);
        }
        p = p->next;
    }
    return result;
}

int checkpremise(Project *pr, Spremise *p)
//----------------------------------------------------------
//    Checks if a particular premise is true
//----------------------------------------------------------
{
    if (p->variable == r_TIME ||
        p->variable == r_CLOCKTIME) return (checktime(pr,p));
    else if (p->status > IS_NUMBER) return (checkstatus(pr,p));
    else return (checkvalue(pr,p));
}

int checktime(Project *pr, Spremise *p)
//------------------------------------------------------------
//    Checks if condition on system time holds
//------------------------------------------------------------
{
    Times *time = &pr->times;
    Rules *rules = &pr->rules;

    char flag;
    long t1, t2, x;

    // Get start and end of rule evaluation time interval
    if (p->variable == r_TIME)
    {
        t1 = rules->Time1;
        t2 = time->Htime;
    }
    else if (p->variable == r_CLOCKTIME)
    {
        t1 = (rules->Time1 + time->Tstart) % SECperDAY;
        t2 = (time->Htime + time->Tstart) % SECperDAY;
    }
    else return (0);

    // Test premise's time
    x = (long)(p->value);
    switch (p->relop)
    {
      // For inequality, test against current time
      case LT:
        if (t2 >= x) return (0);
        break;
      case LE:
        if (t2 > x) return (0);
        break;
      case GT:
        if (t2 <= x) return (0);
        break;
      case GE:
        if (t2 < x) return (0);
      break;

      // For equality, test if within interval
      case EQ:
      case NE:
        flag = FALSE;
        if (t2 < t1) // E.g., 11:00 am to 1:00 am
        {
            if (x >= t1 || x <= t2)
            flag = TRUE;
        }
        else
        {
            if (x >= t1 && x <= t2)
            flag = TRUE;
        }
        if (p->relop == EQ && flag == FALSE) return (0);
        if (p->relop == NE && flag == TRUE)  return (0);
        break;
    }

    // If we get to here then premise was satisfied
    return 1;
}

int checkstatus(Project *pr, Spremise *p)
//------------------------------------------------------------
//    Checks if condition on link status holds
//------------------------------------------------------------
{
    Hydraul *hyd = &pr->hydraul;

    char i;
    int j;

    switch (p->status)
    {
      case IS_OPEN:
      case IS_CLOSED:
      case IS_ACTIVE:
        i = hyd->LinkStatus[p->index];
        if      (i <= CLOSED) j = IS_CLOSED;
        else if (i == ACTIVE) j = IS_ACTIVE;
        else                  j = IS_OPEN;
        if (j == p->status && p->relop == EQ) return 1;
        if (j != p->status && p->relop == NE) return 1;
    }
    return 0;
}

int checkvalue(Project *pr, Spremise *p)
//----------------------------------------------------------
//    Checks if numerical condition on a variable is true.
//    Uses tolerance of 0.001 when testing conditions.
//----------------------------------------------------------
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;

    int     i, j, v;
    double  x,            // A variable's value
            tol = 1.e-3;  // Equality tolerance
    int    Njuncs = net->Njuncs;
    double *Ucf = pr->Ucf;
    double *NodeDemand = hyd->NodeDemand;
    double *LinkFlow = hyd->LinkFlow;
    double *LinkSetting = hyd->LinkSetting;
    Snode  *Node = net->Node;
    Slink  *Link = net->Link;
    Stank  *Tank = net->Tank;

    // Find the value being checked
    i = p->index;
    v = p->variable;
    switch (v)
    {
      case r_DEMAND:
        if (p->object == r_SYSTEM) x = hyd->Dsystem * Ucf[DEMAND];
        else                       x = NodeDemand[i] * Ucf[DEMAND];
        break;

      case r_HEAD:
      case r_GRADE:
        x = hyd->NodeHead[i] * Ucf[HEAD];
        break;

      case r_PRESSURE:
        x = (hyd->NodeHead[i] - Node[i].El) * Ucf[PRESSURE];
        break;

      case r_LEVEL:
        x = (hyd->NodeHead[i] - Node[i].El) * Ucf[HEAD];
        break;

      case r_FLOW:
        x = ABS(LinkFlow[i]) * Ucf[FLOW];
        break;

      case r_SETTING:
        if (LinkSetting[i] == MISSING) return 0;
        x = LinkSetting[i];
        switch (Link[i].Type)
        {
          case PRV:
          case PSV:
          case PBV:
            x = x * Ucf[PRESSURE];
            break;
          case FCV:
            x = x * Ucf[FLOW];
            break;
          default:
            break;
        }
        break;

      case r_FILLTIME:
        if (i <= Njuncs) return 0;
        j = i - Njuncs;
        if (Tank[j].A == 0.0) return 0;
        if (NodeDemand[i] <= TINY) return 0;
        x = (Tank[j].Vmax - Tank[j].V) / NodeDemand[i];
        break;

      case r_DRAINTIME:
        if (i <= Njuncs) return 0;
        j = i - Njuncs;
        if (Tank[j].A == 0.0) return 0;
        if (NodeDemand[i] >= -TINY) return 0;
        x = (Tank[j].Vmin - Tank[j].V) / NodeDemand[i];
        break;

      default:
        return 0;
    }

    // Compare value x against the premise
    switch (p->relop)
    {
      case EQ: if (ABS(x - p->value) > tol) return 0; break;
      case NE: if (ABS(x - p->value) < tol) return 0; break;
      case LT: if (x > p->value + tol) return 0;      break;
      case LE: if (x > p->value - tol) return 0;      break;
      case GT: if (x < p->value - tol) return 0;      break;
      case GE: if (x < p->value + tol) return 0;      break;
    }
    return 1;
}

void updateactionlist(Project *pr, int i, Saction *actions)
//---------------------------------------------------
//    Adds rule's actions to action list
//--------------------------------------------------
{
    Rules *rules = &pr->rules;

    SactionList *actionItem;
    Saction *a;

    // Iterate through each action of Rule i
    a = actions;
    while (a != NULL)
    {
        // Add action to list if its link not already on it
        if (!onactionlist(pr, i, a))
        {
            actionItem = (SactionList *)malloc(sizeof(SactionList));
            if (actionItem != NULL)
            {
                actionItem->action = a;
                actionItem->ruleIndex = i;
                actionItem->next = rules->ActionList;
                rules->ActionList = actionItem;
            }
        }
        a = a->next;
    }
}

int onactionlist(Project *pr, int i, Saction *a)
//-----------------------------------------------------------------------------
//  Checks if action a from rule i can be added to the action list
//-----------------------------------------------------------------------------
{
    Network *net = &pr->network;

    int link, i1;
    SactionList *actionItem;
    Saction *a1;

    // Search action list for link included in action a
    link = a->link;
    actionItem = pr->rules.ActionList;
    while (actionItem != NULL)
    {
        a1 = actionItem->action;
        i1 = actionItem->ruleIndex;

        // Link appears in list
        if (link == a1->link)
        {
            // Replace its action with 'a' if rule i has higher priority
            if (net->Rule[i].priority > net->Rule[i1].priority)
            {
                actionItem->action = a;
                actionItem->ruleIndex = i;
            }

            // Return indicating that 'a' should not be added to action list
            return 1;
        }
        actionItem = actionItem->next;
    }

    // Return indicating that it's ok to add 'a' to the action list
    return 0;
}

int takeactions(Project *pr)
//-----------------------------------------------------------
//    Implements actions on action list
//-----------------------------------------------------------
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Report  *rpt = &pr->report;
    Rules   *rules = &pr->rules;

    char flag;
    int k, s, n;
    double tol = 1.e-3, v, x;
    Saction *a;
    SactionList *actionItem;

    n = 0;
    actionItem = rules->ActionList;
    while (actionItem != NULL)
    {
        flag = FALSE;
        a = actionItem->action;
        k = a->link;
        s = hyd->LinkStatus[k];
        v = hyd->LinkSetting[k];
        x = a->setting;

        // Switch link from closed to open
        if (a->status == IS_OPEN && s <= CLOSED)
        {
            setlinkstatus(pr, k, 1, &hyd->LinkStatus[k], &hyd->LinkSetting[k]);
            flag = TRUE;
        }

        // Switch link from not closed to closed
        else if (a->status == IS_CLOSED && s > CLOSED)
        {
            setlinkstatus(pr, k, 0, &hyd->LinkStatus[k], &hyd->LinkSetting[k]);
            flag = TRUE;
        }

        // Change link's setting
        else if (x != MISSING)
        {
            switch (net->Link[k].Type)
            {
              case PRV:
              case PSV:
              case PBV:
                x = x / pr->Ucf[PRESSURE];
                break;
              case FCV:
                x = x / pr->Ucf[FLOW];
                break;
              default:
                break;
            }
            if (ABS(x - v) > tol)
            {
                setlinksetting(pr, k, x, &hyd->LinkStatus[k],
                               &hyd->LinkSetting[k]);
                flag = TRUE;
            }
        }

        // Report rule action
        if (flag == TRUE)
        {
            n++;
            if (rpt->Statflag)
            {
                writeruleaction(pr, k, net->Rule[actionItem->ruleIndex].label);
            }
        }

        // Move to next action on list
        actionItem = actionItem->next;
    }
    return n;
}

void clearactionlist(Rules *rules)
//----------------------------------------------------------
//    Clears memory used for action list
//----------------------------------------------------------
{
    SactionList *nextItem;
    SactionList *actionItem;
    actionItem = rules->ActionList;
    while (actionItem != NULL)
    {
        nextItem = actionItem->next;
        free(actionItem);
        actionItem = nextItem;
    }
}

void clearrule(Project *pr, int i)
//-----------------------------------------------------------
//  Clears memory used by a rule for premises & actions
//-----------------------------------------------------------
{
    Network *net = &pr->network;

    Spremise *p;
    Spremise *pnext;
    Saction *a;
    Saction *anext;

    p = net->Rule[i].Premises;
    while (p != NULL)
    {
        pnext = p->next;
        free(p);
        p = pnext;
    }
    a = net->Rule[i].ThenActions;
    while (a != NULL)
    {
        anext = a->next;
        free(a);
        a = anext;
    }
    a = net->Rule[i].ElseActions;
    while (a != NULL)
    {
        anext = a->next;
        free(a);
        a = anext;
    }
}


void writepremise(Spremise *p, FILE *f, Network *net)
//-----------------------------------------------------------------------------
//  Write a rule's premise clause to an INP file.
//-----------------------------------------------------------------------------
{
    char s_obj[20];
    char s_id[MAXID + 1];
    char s_value[20];
    int  subtype;

    // Get the type name & ID of object referred to in the premise
    if (p->object == r_NODE)
    {
        subtype = net->Node[p->index].Type;
        getobjtxt(r_NODE, subtype, s_obj);
        strcpy(s_id, net->Node[p->index].ID);
    }
    else if (p->object == r_LINK)
    {
        subtype = net->Link[p->index].Type;
        getobjtxt(r_LINK, subtype, s_obj);
        strcpy(s_id, net->Link[p->index].ID);
    }
    else
    {
        strcpy(s_obj, "SYSTEM");
        strcpy(s_id, "");
    }

    // If premise has no value field, use its status field as a value
    if (p->value == MISSING) strcpy(s_value, Value[p->status]);

    // Otherwise get text of premise's value field
    else
    {
        // For time values convert from seconds to hr:min:sec
        switch (p->variable)
        {
        case r_CLOCKTIME:
        case r_DRAINTIME:
        case r_FILLTIME:
        case r_TIME:
            gettimetxt(p->value, s_value);
            break;
        default: sprintf(s_value, "%.4f", p->value);
        }
    }

    // Write the premise clause to the file
    fprintf(f, "%s %s %s %s %s", s_obj, s_id, Varword[p->variable],
            Operator[p->relop], s_value);
}

void writeaction(Saction *a, FILE *f, Network *net)
//-----------------------------------------------------------------------------
//  Write a rule's action clause to an INP file.
//-----------------------------------------------------------------------------
{
    char s_id[MAXID + 1];
    char s_obj[20];
    char s_var[20];
    char s_value[20];
    int subtype;

    subtype = net->Link[a->link].Type;
    getobjtxt(r_LINK, subtype, s_obj);
    strcpy(s_id, net->Link[a->link].ID);
    if (a->setting == MISSING)
    {
        strcpy(s_var, "STATUS");
        strcpy(s_value, Value[a->status]);
    }
    else
    {
        strcpy(s_var, "SETTING");
        sprintf(s_value, "%.4f", a->setting);
    }
    fprintf(f, "%s %s %s = %s", s_obj, s_id, s_var, s_value);
}

void getobjtxt(int objtype, int subtype, char *objtxt)
//-----------------------------------------------------------------------------
//  Retrieve the text label for a specific type of object.
//-----------------------------------------------------------------------------
{
    if (objtype == r_NODE)
    {
        switch (subtype)
        {
            case JUNCTION:  strcpy(objtxt, "JUNCTION"); break;
            case RESERVOIR: strcpy(objtxt, "RESERVOIR"); break;
            case TANK:      strcpy(objtxt, "TANK"); break;
            default:        strcpy(objtxt, "NODE");
        }
    }
    else if (objtype == r_LINK)
    {
        switch (subtype)
        {
        case CVPIPE:
        case PIPE:  strcpy(objtxt, "PIPE"); break;
        case PUMP:  strcpy(objtxt, "PUMP"); break;
        default:    strcpy(objtxt, "VALVE");
        }
    }
    else strcpy(objtxt, "SYSTEM");
}

void gettimetxt(double secs, char *timetxt)
//-----------------------------------------------------------------------------
//  Convert number of seconds to a text string in hrs:min:sec format.
//-----------------------------------------------------------------------------
{
    int hours = 0, minutes = 0, seconds = 0;
    hours = (int)secs / 3600;
    if (hours > 24 * 7) sprintf(timetxt, "%.4f", secs / 3600.0);
    else
    {
        minutes = (int)((secs - 3600 * hours) / 60);
        seconds = (int)(secs - 3600 * hours - minutes * 60);
        sprintf(timetxt, "%d:%02d:%02d", hours, minutes, seconds);
    }
}
