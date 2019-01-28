/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       input2.c
Description:  reads and interprets network data from an EPANET input file
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 01/01/2019
******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <math.h>

#include "types.h"
#include "funcs.h"
#include "hash.h"
#include "text.h"

#define MAXERRS 10      // Max. input errors reported

extern char *SectTxt[]; // Input section keywords (see ENUMSTXT.H)

// Exported functions
int addnodeID(Network *n, int, char *);
int addlinkID(Network *n, int, char *);
STmplist *getlistitem(char *, STmplist *);

// Imported functions
extern int powercurve(double, double, double, double, double, double *,
                      double *, double *);

// Local functions
static int  newline(Project *, int, char *);
static int  addpattern(Parser *, char *);
static int  addcurve(Parser *, char *);
static int  unlinked(Project *);
static int  getpumpparams(Project *);
static void inperrmsg(Project *, int, int, char *);


int netsize(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: determines number of network objects
**--------------------------------------------------------------
*/
{
    Parser *parser = &pr->parser;
  
    char line[MAXLINE + 1]; // Line from input data file
    char *tok;              // First token of line
    int sect, newsect;      // Input data sections
    int errcode = 0;        // Error code

    // Initialize object counts
    parser->MaxJuncs = 0;
    parser->MaxTanks = 0;
    parser->MaxPipes = 0;
    parser->MaxPumps = 0;
    parser->MaxValves = 0;
    parser->MaxControls = 0;
    parser->MaxRules = 0;
    parser->MaxCurves = 0;
    sect = -1;

    // Add a default demand pattern
    parser->MaxPats = -1;
    addpattern(parser,"");

    if (parser->InFile == NULL) return 0;

    // Make a pass through input file counting number of each object
    while (fgets(line, MAXLINE, parser->InFile) != NULL)
    {
        // Skip blank lines & those beginning with a comment
        tok = strtok(line, SEPSTR);
        if (tok == NULL) continue;
        if (*tok == ';') continue;

        // Check if line begins with a new section heading
        if (tok[0] == '[')
        {
            newsect = findmatch(tok, SectTxt);
            if (newsect >= 0)
            {
                sect = newsect;
                if (sect == _END) break;
                continue;
            }
            else continue;
        }

        // Add to count of current object
        switch (sect)
        {
            case _JUNCTIONS: parser->MaxJuncs++;    break;
            case _RESERVOIRS:
            case _TANKS:     parser->MaxTanks++;    break;
            case _PIPES:     parser->MaxPipes++;    break;
            case _PUMPS:     parser->MaxPumps++;    break;
            case _VALVES:    parser->MaxValves++;   break;
            case _CONTROLS:  parser->MaxControls++; break;
            case _RULES:     addrule(parser,tok);   break;
            case _PATTERNS:  errcode = addpattern(parser, tok); break;
            case _CURVES:    errcode = addcurve(parser, tok);   break;
        }
        if (errcode) break;
    }

    parser->MaxNodes = parser->MaxJuncs + parser->MaxTanks;
    parser->MaxLinks = parser->MaxPipes + parser->MaxPumps + parser->MaxValves;
    if (parser->MaxPats < 1) parser->MaxPats = 1;
    if (!errcode)
    {
        if (parser->MaxJuncs < 1)       errcode = 223; // Not enough nodes
        else if (parser->MaxTanks == 0) errcode = 224; // No tanks
    }
    return errcode;
}

int readdata(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: reads contents of input data file
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    char line[MAXLINE + 1],  // Line from input data file
         wline[MAXLINE + 1]; // Working copy of input line
    int  sect, newsect,      // Data sections
         errcode = 0,        // Error code
         inperr, errsum;     // Error code & total error count

    // Allocate input buffer
    parser->X = (double *)calloc(MAXTOKS, sizeof(double));
    ERRCODE(MEMCHECK(parser->X));
    if (errcode) return errcode;

    // Initialize actual number of network components
    parser->Ntitle = 0;
    net->Nnodes = 0;
    net->Njuncs = 0;
    net->Ntanks = 0;
    net->Nlinks = 0;
    net->Npipes = 0;
    net->Npumps = 0;
    net->Nvalves = 0;
    net->Ncontrols = 0;
    net->Nrules = 0;
    net->Ncurves = parser->MaxCurves;
    net->Npats = parser->MaxPats;
    parser->PrevPat = NULL;
    parser->PrevCurve = NULL;

    sect = -1;
    errsum = 0;

    // Read each line from input file
    while (fgets(line, MAXLINE, parser->InFile) != NULL)
    {
        // Make copy of line and scan for tokens
        strcpy(wline, line);
        parser->Ntokens = gettokens(wline, parser->Tok, MAXTOKS,
                                    parser->Comment);

        // Skip blank lines and comments
        parser->ErrTok = -1;
        if (parser->Ntokens == 0) continue;
        if (*parser->Tok[0] == ';') continue;

        // Check if max. line length exceeded
        if (strlen(line) >= MAXLINE)
        {
            sprintf(pr->Msg, "%s section: %s", geterrmsg(214, pr->Msg),
                    SectTxt[sect]);
            writeline(pr, pr->Msg);
            writeline(pr, line);
            errsum++;
        }

        // Check if at start of a new input section
        if (parser->Tok[0][0] == '[')
        {
            newsect = findmatch(parser->Tok[0], SectTxt);
            if (newsect >= 0)
            {
                sect = newsect;
                if (sect == _END) break;
                continue;
            }
            else
            {
                inperrmsg(pr, 201, sect, line);
                errsum++;
                break;
            }
        }

        // Otherwise process next line of input in current section
        else
        {
            if (sect >= 0)
            {
                inperr = newline(pr, sect, line);
                if (inperr > 0)
                {
                    inperrmsg(pr, inperr, sect, line);
                    errsum++;
                }
            }
            else
            {
                errcode = 200;
                break;
            }
        }

        // Stop if reach end of file or max. error count
        if (errsum == MAXERRS)  break;
    }
    
    // Check for errors
    if (errsum > 0) errcode = 200;
    
    // Check for unlinked nodes
    if (!errcode) errcode = unlinked(pr);

    // Get pattern & curve data from temporary lists
    if (!errcode) errcode = getpatterns(pr);
    if (!errcode) errcode = getcurves(pr);
    if (!errcode) errcode = getpumpparams(pr);

    // Free input buffer
    free(parser->X);
    return errcode;
}

int newline(Project *pr, int sect, char *line)
/*
**--------------------------------------------------------------
**  Input:   sect  = current section of input file
**           *line = line read from input file
**  Output:  returns error code or 0 if no error found
**  Purpose: processes a new line of data from input file
**
**  Note: The xxxdata() functions appear in INPUT3.c.
**--------------------------------------------------------------
*/
{
    Parser *parser = &pr->parser;
    int n;

    switch (sect)
    {
        case _TITLE:
          if (parser->Ntitle < 3)
          {
              n = (int)strlen(line);
              if (line[n - 1] == 10)
              line[n - 1] = ' ';
              strncpy(pr->Title[parser->Ntitle], line, TITLELEN);
              parser->Ntitle++;
          }
          return 0;
        case _JUNCTIONS:   return (juncdata(pr));
        case _RESERVOIRS:
        case _TANKS:       return (tankdata(pr));
        case _PIPES:       return (pipedata(pr));
        case _PUMPS:       return (pumpdata(pr));
        case _VALVES:      return (valvedata(pr));
        case _PATTERNS:    return (patterndata(pr));
        case _CURVES:      return (curvedata(pr));
        case _DEMANDS:     return (demanddata(pr));
        case _CONTROLS:    return (controldata(pr));
        case _RULES:
          if (ruledata(pr) > 0)
          {
              ruleerrmsg(pr);
              return 200;
          }
          else return 0;
        case _SOURCES:     return (sourcedata(pr));
        case _EMITTERS:    return (emitterdata(pr));
        case _QUALITY:     return (qualdata(pr));
        case _STATUS:      return (statusdata(pr));
        case _ROUGHNESS:   return (0);
        case _ENERGY:      return (energydata(pr));
        case _REACTIONS:   return (reactdata(pr));
        case _MIXING:      return (mixingdata(pr));
        case _REPORT:      return (reportdata(pr));
        case _TIMES:       return (timedata(pr));
        case _OPTIONS:     return (optiondata(pr));
        case _COORDS:      return (coordata(pr));

        // Data in these sections are not used for any computations
        case _LABELS:
        case _TAGS:
        case _VERTICES:
        case _BACKDROP:
          return (0);
    }
    return 201;
}

int getpumpparams(Project *pr)
/*
**-------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: computes pump curve coefficients for all pumps
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    int i, k, errcode = 0;
    char errmsg[MAXMSG+1];

    for (i = 1; i <= net->Npumps; i++)
    {
        errcode = updatepumpparams(pr, i);
        if (errcode)
        {
            k = net->Pump[i].Link;
            sprintf(pr->Msg, "Error %d: %s %s",
                    errcode, geterrmsg(errcode, errmsg), net->Link[k].ID);
            writeline(pr, pr->Msg);
            return 200;
        }
    }
    return 0;
}

int updatepumpparams(Project *pr, int pumpindex)
/*
**-------------------------------------------------------------
**  Input:   pumpindex = index of a pump
**  Output:  returns error code
**  Purpose: computes & checks a pump's head curve coefficients
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Spump  *pump;
    Scurve *curve;

    int m;
    int curveindex;
    int npts = 0;
    int errcode = 0;
    double a, b, c, h0 = 0.0, h1 = 0.0, h2 = 0.0, q1 = 0.0, q2 = 0.0;

    pump = &net->Pump[pumpindex];
    if (pump->Ptype == CONST_HP)  // Constant Hp pump
    {
        pump->H0 = 0.0;
        pump->R = -8.814 * net->Link[pump->Link].Km;
        pump->N = -1.0;
        pump->Hmax = BIG; // No head limit
        pump->Qmax = BIG; // No flow limit
        pump->Q0 = 1.0;   // Init. flow = 1 cfs
        return errcode;
    }

    else if (pump->Ptype == NOCURVE) // Pump curve specified
    {
        curveindex = pump->Hcurve;
        if (curveindex == 0) return 226;
        curve = &net->Curve[curveindex];
        curve->Type = PUMP_CURVE;
        npts = curve->Npts;

        // Generic power function curve 
        if (npts == 1)
        {
            pump->Ptype = POWER_FUNC;
            q1 = curve->X[0];
            h1 = curve->Y[0];
            h0 = 1.33334 * h1;
            q2 = 2.0 * q1;
            h2 = 0.0;
        }
      
        // 3 point curve with shutoff head
        else if (npts == 3 && curve->X[0] == 0.0)
        {
            pump->Ptype = POWER_FUNC;
            h0 = curve->Y[0];
            q1 = curve->X[1];
            h1 = curve->Y[1];
            q2 = curve->X[2];
            h2 = curve->Y[2];
        }
      
        // Custom pump curve
        else
        {
            pump->Ptype = CUSTOM;
            for (m = 1; m < npts; m++)
            {
                if (curve->Y[m] >= curve->Y[m - 1]) return 227;
            }
            pump->Qmax = curve->X[npts - 1];
            pump->Q0 = (curve->X[0] + pump->Qmax) / 2.0;
            pump->Hmax = curve->Y[0];
        }
      
        // Compute shape factors & limits of power function curves
        if (pump->Ptype == POWER_FUNC)
        {
            if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c)) return 227;
            else
            {
                pump->H0 = -a;
                pump->R = -b;
                pump->N = c;
                pump->Q0 = q1;
                pump->Qmax = pow((-a / b), (1.0 / c));
                pump->Hmax = h0;
            }
        }
    }
    return 0;
}


int addnodeID(Network *net, int n, char *id)
/*
**-------------------------------------------------------------
**  Input:   n = node index
**           id = ID label
**  Output:  returns 0 if ID already in use, 1 if not
**  Purpose: adds a node ID to the Node Hash Table
**--------------------------------------------------------------
*/
{
    if (findnode(net,id)) return 0; 
    strncpy(net->Node[n].ID, id, MAXID);
    hashtable_insert(net->NodeHashTable, net->Node[n].ID, n);
    return 1;
}

int addlinkID(Network *net, int n, char *id)
/*
**-------------------------------------------------------------
**  Input:   n = link index
**           id = ID label
**  Output:  returns 0 if ID already in use, 1 if not
**  Purpose: adds a link ID to the Link Hash Table
**--------------------------------------------------------------
*/
{
    if (findlink(net,id)) return 0;
    strncpy(net->Link[n].ID, id, MAXID);
    hashtable_insert(net->LinkHashTable, net->Link[n].ID, n);
    return 1;
}

int addpattern(Parser *parser, char *id)
/*
**-------------------------------------------------------------
**  Input:   id = pattern ID label
**  Output:  returns error code
**  Purpose: adds a new pattern to the database
**--------------------------------------------------------------
*/
{
    STmplist *patlist;

    // Check if ID is same as last one processed
    if (parser->Patlist != NULL && strcmp(id, parser->Patlist->ID) == 0) return 0;

    // Check that pattern was not already created
    if (getlistitem(id, parser->Patlist) == NULL)
    {
        // Update pattern count & create new list element
        (parser->MaxPats)++;
        patlist = (STmplist *)malloc(sizeof(STmplist));
        if (patlist == NULL) return 101;

        // Initialize list element properties
        else
        {
            patlist->i = parser->MaxPats;
            strncpy(patlist->ID, id, MAXID);
            patlist->x = NULL;
            patlist->y = NULL;
            patlist->next = parser->Patlist;
            parser->Patlist = patlist;
        }
    }
    return 0;
}

int addcurve(Parser *parser, char *id)
/*
**-------------------------------------------------------------
**  Input:   id = curve ID label
**  Output:  returns error code
**  Purpose: adds a new curve to the database
**--------------------------------------------------------------
*/
{
    STmplist *curvelist;

    // Check if ID is same as last one processed
    if (parser->Curvelist != NULL && strcmp(id, parser->Curvelist->ID) == 0) return 0;

    // Check that curve was not already created
    if (getlistitem(id, parser->Curvelist) == NULL)
    {
        // Update curve count & create new list element
        (parser->MaxCurves)++;
        curvelist = (STmplist *)malloc(sizeof(STmplist));
        if (curvelist == NULL) return 101;

        // Initialize list element properties
        else
        {
            curvelist->i = parser->MaxCurves;
            strncpy(curvelist->ID, id, MAXID);
            curvelist->x = NULL;
            curvelist->y = NULL;
            curvelist->next = parser->Curvelist;
            parser->Curvelist = curvelist;
        }
    }
    return 0;
}

STmplist *getlistitem(char *id, STmplist *list)
/*
**-------------------------------------------------------------
**  Input:   id = ID label
**           list = pointer to head of a temporary list
**  Output:  returns list item with requested ID label
**  Purpose: searches for item in temporary list
**-------------------------------------------------------------
*/
{
    STmplist *item;
    for (item = list; item != NULL; item = item->next)
    {
        if (strcmp(item->ID, id) == 0) return item;
    }
    return NULL;
}

int unlinked(Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code if any unlinked junctions found
** Purpose: checks for unlinked junctions in network
**
** NOTE: unlinked tanks have no effect on computations.
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    int *marked;
    int i, err, errcode;
  
    errcode = 0;
    err = 0;

    // Create an array to record number of links incident on each node
    marked = (int *)calloc(net->Nnodes + 1, sizeof(int));
    ERRCODE(MEMCHECK(marked));
    if (errcode) return errcode;
    memset(marked, 0, (net->Nnodes + 1) * sizeof(int));
 
    // Mark end nodes of each link
    for (i = 1; i <= net->Nlinks; i++)
    {
        marked[net->Link[i].N1]++;
        marked[net->Link[i].N2]++;
    }
 
    // Check each junction
    for (i = 1; i <= net->Njuncs; i++)
    {
        // If not marked then error
        if (marked[i] == 0) 
        {
            err++;
            sprintf(pr->Msg, "Error 233: %s %s", geterrmsg(233, pr->Msg), net->Node[i].ID);
            writeline(pr, pr->Msg);
        }
        if (err >= MAXERRS) break;
    }
    if (err > 0) errcode = 200;
    free(marked);
    return errcode;
}

int getpatterns(Project *pr)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: retrieves pattern data from temporary linked list
**-------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Parser  *parser = &pr->parser;

    int i, j;
    SFloatlist *f;
    STmplist *tmppattern;
    Spattern *pattern;
  
    // Start at head of the list of patterns
    tmppattern = parser->Patlist;

    // Traverse list of temporary patterns
    while (tmppattern != NULL)
    {
        // Get index of temporary pattern in network's Pattern array
        i = tmppattern->i;

        // Check if this is the default pattern
        if (strcmp(tmppattern->ID, parser->DefPatID) == 0) hyd->DefPat = i;

        // Copy temporary patttern to network's pattern
        if (i >= 0 && i <= parser->MaxPats)
        {
            pattern = &net->Pattern[i];
            strcpy(pattern->ID, tmppattern->ID);

            /* Give pattern a length of at least 1 */
            if (pattern->Length == 0) pattern->Length = 1;

            // Allocate array of pattern factors
            pattern->F = (double *)calloc(pattern->Length, sizeof(double));
            if (pattern->F == NULL) return 101;

            // Start at head of temporary pattern multiplier list
            // (which holds multipliers in reverse order)
            f = tmppattern->x;
            j = pattern->Length - 1;

            // Use at least one multiplier equal to 1.0
            if (f == NULL) pattern->F[0] = 1.0;

            // Traverse temporary multiplier list, copying Pattern array */
            else while (f != NULL && j >= 0)
            {
                pattern->F[j] = f->value;
                f = f->next;
                j--;
            }
        }
        tmppattern = tmppattern->next;
    }
    return 0;
}

int getcurves(Project *pr)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: retrieves curve data from temporary linked list
**-----------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
  
    int i, j;
    double x;
    char errmsg[MAXMSG+1];
    SFloatlist *fx, *fy;
    STmplist *tmpcurve;
    Scurve *curve;

    // Traverse list of temporary curves
    tmpcurve = parser->Curvelist;
    while (tmpcurve != NULL)
    {
        // Get index of temporary curve in network's Curve array
        i = tmpcurve->i;

        // Copy temporary curve to network's curve
        if (i >= 1 && i <= parser->MaxCurves)
        {
            curve = &net->Curve[i];
            strcpy(curve->ID, tmpcurve->ID);

            // Check that network curve has data points
            if (curve->Npts <= 0)
            {
                sprintf(pr->Msg, "Error 230: %s %s", geterrmsg(230, errmsg), curve->ID);
                writeline(pr, pr->Msg);
                return 200;
            }

            // Allocate memory for network's curve data
            curve->X = (double *)calloc(curve->Npts, sizeof(double));
            curve->Y = (double *)calloc(curve->Npts, sizeof(double));
            if (curve->X == NULL || curve->Y == NULL) return 101;

            // Traverse list of x,y data
            x = BIG;
            fx = tmpcurve->x;
            fy = tmpcurve->y;
            j = curve->Npts - 1;
            while (fx != NULL && fy != NULL && j >= 0)
            {
                // Check that x data is in ascending order
                if (fx->value >= x)
                {
                    sprintf(pr->Msg, "Error 230: %s %s", geterrmsg(230, errmsg), curve->ID);
                    writeline(pr, pr->Msg);
                    return 200;
                }
                x = fx->value;

                // Copy x,y data to network's curve
                curve->X[j] = fx->value;
                fx = fx->next;
                curve->Y[j] = fy->value;
                fy = fy->next;
                j--;
            }
        }
        tmpcurve = tmpcurve->next;
    }
    return 0;
}

int findmatch(char *line, char *keyword[])
/*
**--------------------------------------------------------------
**  Input:   *line      = line from input file
**           *keyword[] = list of NULL terminated keywords
**  Output:  returns index of matching keyword or
**           -1 if no match found
**  Purpose: determines which keyword appears on input line
**--------------------------------------------------------------
*/
{
    int i = 0;
    while (keyword[i] != NULL)
    {
        if (match(line, keyword[i])) return i;
        i++;
    }
    return -1;
}

int match(const char *str, const char *substr)
/*
**--------------------------------------------------------------
**  Input:   *str    = string being searched
**           *substr = substring being searched for
**  Output:  returns 1 if substr found in str, 0 if not
**  Purpose: sees if substr matches any part of str
**
**      (Not case sensitive)
**--------------------------------------------------------------
*/
{
    int i, j;

    // Fail if substring is empty
    if (!substr[0]) return 0;

    // Skip leading blanks of str
    for (i = 0; str[i]; i++)
    {
        if (str[i] != ' ') break;
    }

    // Check if substr matches remainder of str
    for (j = 0; substr[j]; i++, j++)
    {
        if (!str[i] || UCHAR(str[i]) != UCHAR(substr[j])) return 0;
    }
    return 1;
}

int  gettokens(char *s, char** Tok, int maxToks, char *comment)
/*
 **--------------------------------------------------------------
 **  Input:   *s = string to be tokenized
 **  Output:  returns number of tokens in s
 **  Purpose: scans string for tokens, saving pointers to them
 **           in module global variable Tok[]
 **
 ** Tokens can be separated by the characters listed in SEPSTR
 ** (spaces, tabs, newline, carriage return) which is defined
 ** in TYPES.H. Text between quotes is treated as a single token.
 **--------------------------------------------------------------
 */
{
    int  m, n;
    size_t len;
    char *c, *c2;
  
    // clear comment
    comment[0] = '\0';
  
    // Begin with no tokens
    for (n=0; n<maxToks; n++) Tok[n] = NULL;
    n = 0;
  
    // Truncate s at start of comment
    c = strchr(s,';');
    if (c)
    {
        c2 = c+1;
        if (c2)
        {
            // there is a comment here, after the semi-colon.
            len = strlen(c2);
            if (len > 0)
            {
                len = strcspn(c2, "\n\r");
                len = MIN(len, MAXMSG);
                strncpy(comment, c2, len);
                comment[MIN(len,MAXMSG)] = '\0';
            }
        }
        *c = '\0';
    }
    len = (int)strlen(s);
  
    // Scan s for tokens until nothing left
    while (len > 0 && n < MAXTOKS)
    {
        m = (int)strcspn(s,SEPSTR);     // Find token length
        len -= m+1;                     // Update length of s
        if (m == 0) s++;                // No token found
        else
        {
            if (*s == '"')              // Token begins with quote
            {
                s++;                           // Start token after quote
                m = (int)strcspn(s,"\"\n\r");  // Find end quote (or EOL)
            }                            
            s[m] = '\0';                 // Null-terminate the token
            Tok[n] = s;                  // Save pointer to token
            n++;                         // Update token count
            s += m+1;                    // Begin next token
        }
    }
    return n;
}  

double hour(char *time, char *units)
/*
**---------------------------------------------------------
**  Input:   *time  = string containing a time value
**           *units = string containing time units
**  Output:  returns numerical value of time in hours,
**           or -1 if an error occurs
**  Purpose: converts time from units to hours
**---------------------------------------------------------
*/
{
    int n;
    double y[3];
    char *s;

    // Separate clock time into hrs, min, sec
    for (n = 0; n < 3; n++) y[n] = 0.0;
    n = 0;
    s = strtok(time, ":");
    while (s != NULL && n <= 3)
    {
        if (!getfloat(s, &y[n])) return -1.0;
        s = strtok(NULL, ":");
        n++;
    }

    // If decimal time with units attached then convert to hours
    if (n == 1)
    {
        if (strlen(units) == 0)      return (y[0]);
        if (match(units, w_SECONDS)) return (y[0] / 3600.0);
        if (match(units, w_MINUTES)) return (y[0] / 60.0);
        if (match(units, w_HOURS))   return (y[0]);
        if (match(units, w_DAYS))    return (y[0] * 24.0);
    }

    // Convert hh:mm:ss format to decimal hours 
    if (n > 1) y[0] = y[0] + y[1] / 60.0 + y[2] / 3600.0;

    // If am/pm attached then adjust hour accordingly
    // (12 am is midnight, 12 pm is noon)
    if (units[0] == '\0') return y[0];
    if (match(units, w_AM))
    {
        if (y[0] >= 13.0) return -1.0;
        if (y[0] >= 12.0) return (y[0] - 12.0);
        else              return (y[0]);
    }
    if (match(units, w_PM))
    {
        if (y[0] >= 13.0) return -1.0;
        if (y[0] >= 12.0) return y[0];
        else              return (y[0] + 12.0);
    }
    return -1.0;
} 

int getfloat(char *s, double *y)
/*
**-----------------------------------------------------------
**  Input:   *s = character string
**  Output:  *y = floating point number
**           returns 1 if conversion successful, 0 if not
**  Purpose: converts string to floating point number
**-----------------------------------------------------------
*/
{
    char *endptr;
    *y = (double)strtod(s, &endptr);
    if (*endptr > 0) return 0;
    return 1;
}

int setreport(Project *pr, char *s)
/*
**-----------------------------------------------------------
**  Input:   *s = report format command
**  Output:  none
**  Returns: error code
**  Purpose: processes a report formatting command
**           issued by the ENsetreport function
**-----------------------------------------------------------
*/
{
    Parser *parser = &pr->parser;
    parser->Ntokens = gettokens(s, parser->Tok, MAXTOKS, parser->Comment);
    return reportdata(pr);
}

void inperrmsg(Project *pr, int err, int sect, char *line)
/*
**-------------------------------------------------------------
**  Input:   err   = error code
**           sect  = input data section
**           *line = line from input file
**  Output:  none
**  Purpose: displays input reader error message
**-------------------------------------------------------------
*/
{
    Parser *parser = &pr->parser;
  
    char errStr[MAXMSG + 1] = "";
    char tok[MAXMSG + 1];

    // Get token associated with input error
    if (parser->ErrTok >= 0) strcpy(tok, parser->Tok[parser->ErrTok]);
    else strcpy(tok, "");
  
    // write error message to report file
    sprintf(pr->Msg, "Error %d: %s %s in %s section:",
            err, geterrmsg(err, errStr), tok, SectTxt[sect]);
    writeline(pr, pr->Msg);

    // Echo input line
    writeline(pr, line);
}
