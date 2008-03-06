/*
**********************************************************************
                                                                    
RULES.C -- Rule processor module for EPANET                  
                                                                
VERSION:    2.00                          
DATE:       5/8/00
            9/7/00
            10/25/00
            3/1/01
            8/15/07    (2.00.11)
AUTHOR:     L. Rossman
            US EPA - NRMRL
                                                                    
  The entry points for this module are:
     initrules()  -- called from ENopen() in EPANET.C
     addrule()    -- called from netsize() in INPUT2.C
     allocrules() -- called from allocdata() in EPANET.C
     ruledata()   -- called from newline() in INPUT2.C
     freerules()  -- called from freedata() in EPANET.C
     checkrules() -- called from ruletimestep() in HYDRAUL.C

**********************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "hash.h"
#include "text.h"
#include "types.h"
#include "funcs.h"
#define  EXTERN  extern
#include "vars.h"

struct      Premise         /* Rule Premise Clause */
{
   int      logop;          /* Logical operator */
   int      object;         /* Node or link */
   int      index;          /* Object's index */
   int      variable;       /* Pressure, flow, etc. */
   int      relop;          /* Relational operator */
   int      status;         /* Variable's status */
   double    value;          /* Variable's value */
   struct   Premise *next;
};

struct     Action           /* Rule Action Clause */
{
   int     link;            /* Link index */
   int     status;          /* Link's status */
   double   setting;         /* Link's setting */
   struct  Action *next;
};

struct      aRule           /* Control Rule Structure */
{
   char     label[MAXID+1];    /* Rule character label */
   double    priority;          /* Priority level */
   struct   Premise  *Pchain;  /* Linked list of premises */
   struct   Action   *Tchain;  /* Linked list of actions if true */
   struct   Action   *Fchain;  /* Linked list of actions if false */
   struct   aRule    *next;
};

struct      ActItem         /* Action list item */
{
   int      ruleindex;        /* Index of rule action belongs to */
   struct   Action   *action; /* An action structure */
   struct   ActItem  *next;     
};

struct  aRule *Rule;        /* Array of rules */
struct  ActItem *ActList;   /* Linked list of action items */
int     RuleState;          /* State of rule interpreter */
long    Time1;              /* Start of rule evaluation time interval (sec) */
struct  Premise *Plast;     /* Previous premise clause */

enum    Rulewords      {r_RULE,r_IF,r_AND,r_OR,r_THEN,r_ELSE,r_PRIORITY,r_ERROR};
char    *Ruleword[]  = {w_RULE,w_IF,w_AND,w_OR,w_THEN,w_ELSE,w_PRIORITY,NULL};

enum    Varwords       {r_DEMAND, r_HEAD, r_GRADE, r_LEVEL, r_PRESSURE,
                        r_FLOW, r_STATUS, r_SETTING, r_POWER, r_TIME,
                        r_CLOCKTIME, r_FILLTIME, r_DRAINTIME};
char    *Varword[]   = {w_DEMAND, w_HEAD, w_GRADE, w_LEVEL, w_PRESSURE,
                        w_FLOW, w_STATUS, w_SETTING, w_POWER,w_TIME,
                        w_CLOCKTIME,w_FILLTIME,w_DRAINTIME, NULL};

enum    Objects        {r_JUNC,r_RESERV,r_TANK,r_PIPE,r_PUMP,r_VALVE,
                        r_NODE,r_LINK,r_SYSTEM};
char    *Object[]    = {w_JUNC,w_RESERV,w_TANK,w_PIPE,w_PUMP,w_VALVE,
                        w_NODE,w_LINK,w_SYSTEM,NULL};

/* NOTE: place "<=" & ">=" before "<" & ">" so that findmatch() works correctly. */
enum    Operators      { EQ, NE,  LE,  GE,  LT, GT, IS,  NOT,  BELOW,  ABOVE};
char    *Operator[]  = {"=","<>","<=",">=","<",">",w_IS,w_NOT,w_BELOW,w_ABOVE,NULL};

enum    Values         {IS_NUMBER,IS_OPEN,IS_CLOSED,IS_ACTIVE};
char    *Value[]     = {"XXXX",   w_OPEN, w_CLOSED, w_ACTIVE,NULL};

/* External variables declared in INPUT2.C */
extern char      *Tok[MAXTOKS];
extern int       Ntokens;

/*
**   Local function prototypes are defined here and not in FUNCS.H 
**   because some of them utilize the Premise and Action structures
**   defined locally in this module.
*/
void    newrule(void);
int     newpremise(int);
int     newaction(void);
int     newpriority(void);
int     evalpremises(int);
void    updateactlist(int, struct Action *);
int     checkaction(int, struct Action *);
int     checkpremise(struct Premise *);
int     checktime(struct Premise *);
int     checkstatus(struct Premise *);
int     checkvalue(struct Premise *);
int     takeactions(void);
void    clearactlist(void);
void    clearrules(void);
void    ruleerrmsg(int);


void initrules()
/*
**--------------------------------------------------------------
**    Initializes rule base.
**    Called by ENopen() in EPANET.C module
**--------------------------------------------------------------
*/
{
   RuleState = r_PRIORITY;
   Rule = NULL;
}


void addrule(char *tok)
/*
**--------------------------------------------------------------
**    Updates rule count if RULE keyword found in line of input.
**    Called by netsize() in INPUT2.C module.
**--------------------------------------------------------------
*/
{
   if (match(tok,w_RULE)) MaxRules++;
}


int  allocrules()
/*
**--------------------------------------------------------------
**    Allocates memory for rule-based controls.
**    Called by allocdata() in EPANET.C module.
**--------------------------------------------------------------
*/
{
   Rule = (struct aRule *) calloc(MaxRules+1,sizeof(struct aRule));
   if (Rule == NULL) return(101);
   else return(0);
}


void freerules()
/*
**--------------------------------------------------------------
**    Frees memory used for rule-based controls.
**    Called by freedata() in EPANET.C module.
**--------------------------------------------------------------
*/
{
   clearrules();
   free(Rule);
}


int checkrules(long dt)
/*
**-----------------------------------------------------
**    Checks which rules should fire at current time.
**    Called by ruletimestep() in HYDRAUL.C.
**-----------------------------------------------------
*/
{
   int i,
       r;    /* Number of actions actually taken */

   /* Start of rule evaluation time interval */
   Time1 = Htime - dt + 1;

   /* Iterate through each rule */
   ActList = NULL;
   r = 0;
   for (i=1; i<=Nrules; i++)
   {
      /* If premises true, add THEN clauses to action list. */
      if (evalpremises(i) == TRUE) updateactlist(i,Rule[i].Tchain);

      /* If premises false, add ELSE actions to list. */
      else
      {
          if (Rule[i].Fchain != NULL) updateactlist(i,Rule[i].Fchain);
      }
   }

   /* Execute actions then clear list. */
   if (ActList != NULL) r = takeactions();
   clearactlist();
   return(r);
}


int  ruledata()
/*
**--------------------------------------------------------------
**    Parses a line from [RULES] section of input.
**    Called by newline() in INPUT2.C module.
**    Tok[] is global array of tokens parsed from input line.
**--------------------------------------------------------------
*/
{
   int    key,                      /* Keyword code */
          err;

   /* Exit if current rule has an error */
   if (RuleState == r_ERROR) return(0);

   /* Find the key word that begins the rule statement */
   err = 0;
   key = findmatch(Tok[0],Ruleword);
   switch (key)
   {
      case -1:     err = 201;      /* Unrecognized keyword */
                   break;
      case r_RULE: Nrules++;
                   newrule();
                   RuleState = r_RULE;
                   break;
      case r_IF:   if (RuleState != r_RULE)
                   {
                      err = 221;   /* Mis-placed IF clause */
                      break;
                   }
                   RuleState = r_IF;
                   err = newpremise(r_AND);
                   break;
      case r_AND:  if (RuleState == r_IF) err = newpremise(r_AND);
                   else if (RuleState == r_THEN || RuleState == r_ELSE)
                      err = newaction();
                   else err = 221;
                   break;
      case r_OR:   if (RuleState == r_IF) err = newpremise(r_OR);
                   else err = 221;
                   break;
      case r_THEN: if (RuleState != r_IF)
                   {
                      err = 221;   /* Mis-placed THEN clause */
                      break;
                   }
                   RuleState = r_THEN;
                   err = newaction();
                   break;
      case r_ELSE: if (RuleState != r_THEN)
                   {
                      err = 221;   /* Mis-placed ELSE clause */
                      break;
                   }
                   RuleState = r_ELSE;
                   err = newaction();
                   break;
      case r_PRIORITY: if (RuleState != r_THEN && RuleState != r_ELSE)
                       {
                          err = 221;
                          break;
                       }
                       RuleState = r_PRIORITY;
                       err = newpriority();
                       break;
      default:         err = 201;
   }

   /* Set RuleState to r_ERROR if errors found */
   if (err)
   {
      RuleState = r_ERROR;
      ruleerrmsg(err);
      err = 200;
   }
   return(err);
}


void  clearactlist()
/*
**----------------------------------------------------------
**    Clears memory used for action list
**----------------------------------------------------------
*/
{
   struct ActItem *a;
   struct ActItem *anext;
   a = ActList;
   while (a != NULL)
   {
      anext = a->next;
      free(a);
      a = anext;
   }
}


void  clearrules()
/*
**-----------------------------------------------------------
**    Clears memory used for premises & actions for all rules
**-----------------------------------------------------------
*/
{
   struct Premise *p;
   struct Premise *pnext;
   struct Action  *a;
   struct Action  *anext;
   int i;
   for (i=1; i<=Nrules; i++)
   {
      p = Rule[i].Pchain;
      while (p != NULL)
      {
         pnext = p->next;
         free(p);
         p = pnext;
      }
      a = Rule[i].Tchain;
      while (a != NULL)
      {
         anext = a->next;
         free(a);
         a = anext;
      }
      a = Rule[i].Fchain;
      while (a != NULL)
      {
         anext = a->next;
         free(a);
         a = anext;
      }
   }
}


void  newrule()
/*
**----------------------------------------------------------
**    Adds new rule to rule base
**----------------------------------------------------------
*/
{
   strncpy(Rule[Nrules].label, Tok[1], MAXID);
   Rule[Nrules].Pchain = NULL;
   Rule[Nrules].Tchain = NULL;
   Rule[Nrules].Fchain = NULL;
   Rule[Nrules].priority = 0.0;
   Plast = NULL;
}


int  newpremise(int logop)
/*
**--------------------------------------------------------------------
**   Adds new premise to current rule.
**   Formats are:
**     IF/AND/OR <object> <id> <variable> <operator> <value>
**     IF/AND/OR  SYSTEM <variable> <operator> <value> (units)
**
**   Calls findmatch() and hour() in INPUT2.C.
**   Calls findnode() and findlink() in EPANET.C.
**---------------------------------------------------------------------
*/
{
   int   i,j,k,m,r,s,v;
   double x;
   struct Premise *p;

   /* Check for correct number of tokens */
   if (Ntokens != 5 && Ntokens != 6) return(201);

   /* Find network object & id if present */
   i = findmatch(Tok[1],Object);
   if (i == r_SYSTEM)
   { 
      j = 0;
      v = findmatch(Tok[2],Varword);
      if (v != r_DEMAND && v != r_TIME && v != r_CLOCKTIME) return(201);
   }
   else
   {
      v = findmatch(Tok[3],Varword);
      if (v < 0) return(201);
      switch (i) 
      {
         case r_NODE:
         case r_JUNC:
         case r_RESERV:
         case r_TANK:   k = r_NODE; break;
         case r_LINK:
         case r_PIPE:
         case r_PUMP:
         case r_VALVE:  k = r_LINK; break;
         default: return(201);
      }
      i = k;
      if (i == r_NODE)
      {
         j = findnode(Tok[2]);
         if (j == 0) return(203);
         switch (v)
         {
            case r_DEMAND:
            case r_HEAD:
            case r_GRADE:
            case r_LEVEL:
            case r_PRESSURE: break;

/*** Updated 9/7/00 ***/
            case r_FILLTIME:
            case r_DRAINTIME: if (j <= Njuncs) return(201); break;

            default: return(201);
         }
      }
      else
      {
         j = findlink(Tok[2]);
         if (j == 0) return(204);
         switch (v)
         {
            case r_FLOW:
            case r_STATUS:
            case r_SETTING: break;
            default: return(201);
         }
      }
   }

   /* Parse relational operator (r) and check for synonyms */
   if (i == r_SYSTEM) m = 3;
   else m = 4;
   k = findmatch(Tok[m],Operator);
   if (k < 0) return(201);
   switch(k)
   {
      case IS:    r = EQ; break;
      case NOT:   r = NE; break;
      case BELOW: r = LT; break;
      case ABOVE: r = GT; break;
      default:    r = k;
   }

   /* Parse for status (s) or numerical value (x) */
   s = 0;
   x = MISSING;
   if (v == r_TIME || v == r_CLOCKTIME)
   {
      if (Ntokens == 6)
         x = hour(Tok[4],Tok[5])*3600.;
      else
         x = hour(Tok[4],"")*3600.;
      if (x < 0.0) return(202);
   }
   else if ((k = findmatch(Tok[Ntokens-1],Value)) > IS_NUMBER) s = k;
   else
   {
      if (!getfloat(Tok[Ntokens-1],&x)) return(202);
      if (v == r_FILLTIME || v == r_DRAINTIME) x = x*3600.0;                   //(2.00.11 - LR)
   }

   
         
   /* Create new premise structure */
   p = (struct Premise *) malloc(sizeof(struct Premise));
   if (p == NULL) return(101);
   p->object = i;
   p->index =  j;
   p->variable = v;
   p->relop = r;
   p->logop = logop;
   p->status   = s;
   p->value    = x;

   /* Add premise to current rule's premise list */
   p->next = NULL;
   if (Plast == NULL) Rule[Nrules].Pchain = p;
   else Plast->next = p;
   Plast = p;
   return(0);
}


int  newaction()
/*
**----------------------------------------------------------
**   Adds new action to current rule.
**   Format is:
**      THEN/ELSE/AND LINK <id> <variable> IS <value>
**
**   Calls findlink() from EPANET.C.
**   Calls getfloat() and findmatch() from INPUT2.C.
**----------------------------------------------------------
*/
{
   int   j,k,s;
   double x;
   struct Action *a;

   /* Check for correct number of tokens */
   if (Ntokens != 6) return(201);

   /* Check that link exists */
   j = findlink(Tok[2]);
   if (j == 0) return(204);

/***  Updated 9/7/00  ***/
   /* Cannot control a CV */
   if (Link[j].Type == CV) return(207);

   /* Find value for status or setting */
   s = -1;
   x = MISSING;
   if ((k = findmatch(Tok[5],Value)) > IS_NUMBER) s = k;
   else
   {
      if (!getfloat(Tok[5],&x)) return(202);
      if (x < 0.0) return(202);
   }

/*** Updated 9/7/00 ***/
   /* Cannot change setting for a GPV ***/
   if (x != MISSING && Link[j].Type == GPV) return(202);

/*** Updated 3/1/01 ***/
   /* Set status for pipe in case setting was specified */
   if (x != MISSING && Link[j].Type == PIPE)
   {
      if (x == 0.0) s = IS_CLOSED;
      else          s = IS_OPEN;
      x = MISSING;
   }

   /* Create a new action structure */
   a = (struct Action *) malloc(sizeof(struct Action));
   if (a == NULL) return(101);
   a->link = j;
   a->status = s;
   a->setting = x;

   /* Add action to current rule's action list */
   if (RuleState == r_THEN)
   {
     a->next = Rule[Nrules].Tchain;
     Rule[Nrules].Tchain = a;
   }
   else
   {
      a->next = Rule[Nrules].Fchain;
      Rule[Nrules].Fchain = a;
   }
   return(0);
}


int  newpriority()
/*
**---------------------------------------------------
**    Adds priority rating to current rule
**---------------------------------------------------
*/
{
    double x;
    if (!getfloat(Tok[1],&x)) return(202);
    Rule[Nrules].priority = x;
    return(0);
}


int  evalpremises(int i)
/*
**----------------------------------------------------------
**    Checks if premises to rule i are true
**----------------------------------------------------------
*/
{
    int result;
    struct Premise *p;

    result = TRUE;
    p = Rule[i].Pchain;
    while (p != NULL)
    {
        if (p->logop == r_OR)
        {
            if (result == FALSE)
            {
                result = checkpremise(p);
            }
        }
        else
        {
            if (result == FALSE) return(FALSE);
            result = checkpremise(p);
        }
        p = p->next;
    }
    return(result);
}

 
int  checkpremise(struct Premise *p)
/*
**----------------------------------------------------------
**    Checks if a particular premise is true
**----------------------------------------------------------
*/
{
    if (p->variable == r_TIME || p->variable == r_CLOCKTIME)
       return(checktime(p));
    else if (p->status > IS_NUMBER)
       return(checkstatus(p));
    else
       return(checkvalue(p));
}


int  checktime(struct Premise *p)
/*
**------------------------------------------------------------
**    Checks if condition on system time holds
**------------------------------------------------------------
*/
{
   char  flag;
   long  t1,t2,x;

   /* Get start and end of rule evaluation time interval */ 
   if (p->variable == r_TIME)
   {
        t1 = Time1;
        t2 = Htime;
   }
   else if (p->variable == r_CLOCKTIME)
   {
        t1 = (Time1 + Tstart) % SECperDAY;
        t2 = (Htime + Tstart) % SECperDAY;
   }
   else return(0);

   /* Test premise's time */
   x = (long)(p->value);
   switch (p->relop)
   {
      /* For inequality, test against current time */
        case LT: if (t2 >= x) return(0); break;
        case LE: if (t2 >  x) return(0); break;
        case GT: if (t2 <= x) return(0); break;
        case GE: if (t2 <  x) return(0); break;

      /* For equality, test if within interval */
        case EQ:
        case NE:
           flag = FALSE;
           if (t2 < t1)     /* E.g., 11:00 am to 1:00 am */
           {
              if (x >= t1 || x <= t2) flag = TRUE;
           }
           else
           {
              if (x >= t1 && x <= t2) flag = TRUE;
           }
           if (p->relop == EQ && flag == FALSE) return(0);
           if (p->relop == NE && flag == TRUE)  return(0);
           break;
   }

   /* If we get to here then premise was satisfied */
   return(1);
}


int  checkstatus(struct Premise *p)
/*
**------------------------------------------------------------
**    Checks if condition on link status holds
**------------------------------------------------------------
*/
{
    char i;
    int  j;
    switch (p->status)
    {
        case IS_OPEN:
        case IS_CLOSED:
        case IS_ACTIVE:
                i = S[p->index];
                if      (i <= CLOSED) j = IS_CLOSED;
                else if (i == ACTIVE) j = IS_ACTIVE;
                else                  j = IS_OPEN;
                if (j == p->status &&
                p->relop == EQ)     return(1);
                if (j != p->status &&
                p->relop == NE) return(1);
    }
    return(0);
}


int  checkvalue(struct Premise *p)
/*
**----------------------------------------------------------
**    Checks if numerical condition on a variable is true.
**    Uses tolerance of 0.001 when testing conditions.
**----------------------------------------------------------
*/
{
    int   i,j,v;
    double x,
          tol = 1.e-3;

    i = p->index;
    v = p->variable;
    switch (v)
    {

/*** Updated 10/25/00 ***/
        case r_DEMAND:    if (p->object == r_SYSTEM) x = Dsystem*Ucf[DEMAND];
                          else x = D[i]*Ucf[DEMAND];
                          break;

        case r_HEAD:
        case r_GRADE:     x = H[i]*Ucf[HEAD];
                          break;
        case r_PRESSURE:  x = (H[i]-Node[i].El)*Ucf[PRESSURE];
                          break;
        case r_LEVEL:     x = (H[i]-Node[i].El)*Ucf[HEAD];
                          break;
        case r_FLOW:      x = ABS(Q[i])*Ucf[FLOW];
                          break;
        case r_SETTING:   if (K[i] == MISSING) return(0);
                          x = K[i];
                          switch (Link[i].Type)
                          {
                             case PRV:
                             case PSV:
                             case PBV:  x = x*Ucf[PRESSURE]; break;
                             case FCV:  x = x*Ucf[FLOW];     break;
                          }
                          break;
        case r_FILLTIME:  if (i <= Njuncs) return(0);
                          j = i-Njuncs;
                          if (Tank[j].A == 0.0) return(0);
                          if (D[i] <= TINY) return(0);
                          x = (Tank[j].Vmax - Tank[j].V)/D[i];
                          break;
        case r_DRAINTIME: if (i <= Njuncs) return(0);
                          j = i-Njuncs;
                          if (Tank[j].A == 0.0) return(0);
                          if (D[i] >= -TINY) return(0);
                          x = (Tank[j].Vmin - Tank[j].V)/D[i];
                          break;
        default:          return(0);
    }
    switch (p->relop)
    {
        case EQ:        if (ABS(x - p->value) > tol) return(0);
                        break;
        case NE:        if (ABS(x - p->value) < tol) return(0);
                        break;
        case LT:        if (x > p->value + tol) return(0); break;
        case LE:        if (x > p->value - tol) return(0); break;
        case GT:        if (x < p->value - tol) return(0); break;
        case GE:        if (x < p->value + tol) return(0); break;
    }
    return(1);
}


void  updateactlist(int i, struct Action *actions)
/*
**---------------------------------------------------
**    Adds rule's actions to action list
**---------------------------------------------------
*/
{
   struct ActItem *item;
   struct Action *a;

   /* Iterate through each action of Rule i */
   a = actions;
   while (a != NULL)
   {
      /* Add action to list if not already on it */
      if (!checkaction(i,a))
      {
         item = (struct ActItem *) malloc(sizeof(struct ActItem));
         if (item != NULL)
         {
            item->action = a;
            item->ruleindex = i;
            item->next = ActList;
            ActList = item;
         }
      }
      a = a->next;
   }
}


int  checkaction(int i, struct Action *a)
/*
**-----------------------------------------------------------
**    Checks if an action is already on the Action List
**-----------------------------------------------------------
*/
{
   int i1,k,k1;
   struct ActItem *item;
   struct Action *a1;

   /* Search action list for link named in action */
   k = a->link;                 /* Action applies to link k */
   item = ActList;
   while (item != NULL)
   {
      a1 = item->action;
      i1 = item->ruleindex;
      k1 = a1->link;

      /* If link on list then replace action if rule has higher priority. */
      if (k1 == k)
      {
         if (Rule[i].priority > Rule[i1].priority)
         {
            item->action = a;
            item->ruleindex = i;
         }
         return(1);
      }
      item = item->next;
   }
   return(0);
}


int  takeactions()
/*
**-----------------------------------------------------------
**    Implements actions on action list
**-----------------------------------------------------------
*/
{
    struct Action *a;
    struct ActItem *item;
    char   flag;
    int    k, s, n;
    double  tol = 1.e-3,
           v, x;

    n = 0;
    item = ActList;
    while (item != NULL)
    {
        flag = FALSE;
        a = item->action;
        k = a->link;
        s = S[k];
        v = K[k];
        x = a->setting;

        /* Switch link from closed to open */
        if (a->status == IS_OPEN && s <= CLOSED)
        {
            setlinkstatus(k, 1, &S[k], &K[k]);
            flag = TRUE;
        }

        /* Switch link from not closed to closed */ 
        else if (a->status == IS_CLOSED && s > CLOSED)
        {
            setlinkstatus(k, 0, &S[k], &K[k]);
            flag = TRUE;
        }

        /* Change link's setting */
        else if (x != MISSING)
        {
            switch(Link[k].Type)
            {
                case PRV:
                case PSV:
                case PBV:    x = x/Ucf[PRESSURE];  break;
                case FCV:    x = x/Ucf[FLOW];      break;
            }
            if (ABS(x-v) > tol)
            {
                setlinksetting(k, x, &S[k], &K[k]);
                flag = TRUE;
            }
        }

        /* Report rule action */
        if (flag == TRUE)
        {
           n++;
           if (Statflag) writeruleaction(k,Rule[item->ruleindex].label);
        }

        /* Move to next action on list */
        item = item->next;
    }
    return(n);
}


void  ruleerrmsg(int err)
/*
**-----------------------------------------------------------
**    Reports error message
**-----------------------------------------------------------
*/
{
   int    i;
   char   label[81];
   char   fmt[256];
   switch (err)
   {
      case 201:   strcpy(fmt,R_ERR201);  break;
      case 202:   strcpy(fmt,R_ERR202);  break;
      case 203:   strcpy(fmt,R_ERR203);  break;
      case 204:   strcpy(fmt,R_ERR204);  break;

/***  Updated on 9/7/00  ***/
      case 207:   strcpy(fmt,R_ERR207);  break;

      case 221:   strcpy(fmt,R_ERR221);  break;
      default:    return;
   }
   if (Nrules > 0)
   {
      strcpy(label,t_RULE);
      strcat(label," ");
      strcat(label,Rule[Nrules].label);
   }
   else strcpy(label,t_RULES_SECT);
   sprintf(Msg,fmt);
   strcat(Msg,label);
   strcat(Msg,":");
   writeline(Msg);
   strcpy(fmt,Tok[0]);
   for (i=1; i<Ntokens; i++)
   {
      strcat(fmt," ");
      strcat(fmt,Tok[i]);
   }
   writeline(fmt);
}
   
/***************** END OF RULES.C ******************/   

