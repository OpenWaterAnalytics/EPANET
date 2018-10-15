/*
**********************************************************************

INPUT2.C -- Input data file interpreter for EPANET

VERSION:    2.00
DATE:       5/30/00
            9/7/00
            10/25/00
AUTHOR:     L. Rossman
            US EPA - NRMRL

This module reads and interprets the input data from file InFile.

The entry points for this module are:
   netsize()   -- called from ENopen() in EPANET.C
   readdata()  -- called from getdata() in INPUT1.C

The following utility functions are all called from INPUT3.C
   addnodeID()
   addlinkID()
   findID()
   getfloat()

**********************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "hash.h"
#include "text.h"
#include "types.h"
#include "epanet2.h"
#include "funcs.h"
#include <math.h>

#define MAXERRS 10 /* Max. input errors reported        */


/* Defined in enumstxt.h in EPANET.C */
extern char *SectTxt[]; /* Input section keywords            */

int netsize(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: determines number of system components
**--------------------------------------------------------------
*/
{
  parser_data_t *par = &pr->parser;
  
  char line[MAXLINE + 1]; /* Line from input data file    */
  char *tok;              /* First token of line          */
  int sect, newsect;      /* Input data sections          */
  int errcode = 0;        /* Error code                   */

  /* Initialize network component counts */
  par->MaxJuncs = 0;
  par->MaxTanks = 0;
  par->MaxPipes = 0;
  par->MaxPumps = 0;
  par->MaxValves = 0;
  par->MaxControls = 0;
  par->MaxRules = 0;
  par->MaxCurves = 0;
  sect = -1;

  /* Add a default pattern 0 */
  par->MaxPats = -1;
  addpattern(par,"");

  if (par->InFile == NULL) {
    return (0);
  }

  /* Make pass through data file counting number of each component */
  while (fgets(line, MAXLINE, par->InFile) != NULL) {
    /* Skip blank lines & those beginning with a comment */
    tok = strtok(line, SEPSTR);
    if (tok == NULL)
      continue;
    if (*tok == ';')
      continue;

    /* Check if line begins with a new section heading */
    if (tok[0] == '[') {
      newsect = findmatch(tok, SectTxt);
      if (newsect >= 0) {
        sect = newsect;
        if (sect == _END)
          break;
        continue;
      } else
        continue;
    }

    /* Add to count of current component */
    switch (sect) {
    case _JUNCTIONS:
      par->MaxJuncs++;
      break;
    case _RESERVOIRS:
    case _TANKS:
      par->MaxTanks++;
      break;
    case _PIPES:
      par->MaxPipes++;
      break;
    case _PUMPS:
      par->MaxPumps++;
      break;
    case _VALVES:
      par->MaxValves++;
      break;
    case _CONTROLS:
      par->MaxControls++;
      break;
    case _RULES:
      addrule(par,tok);
      break; /* See RULES.C */
    case _PATTERNS:
      errcode = addpattern(par, tok);
      break;
    case _CURVES:
      errcode = addcurve(par, tok);
      break;
    }
    if (errcode)
      break;
  }

  par->MaxNodes = par->MaxJuncs + par->MaxTanks;
  par->MaxLinks = par->MaxPipes + par->MaxPumps + par->MaxValves;
  if (par->MaxPats < 1)
    par->MaxPats = 1;
  if (!errcode) {
    if (par->MaxJuncs < 1)
      errcode = 223; /* Not enough nodes */
    else if (par->MaxTanks == 0)
      errcode = 224; /* No tanks */
  }
  return (errcode);
} /*  End of netsize  */

int readdata(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: reads contents of input data file
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  char line[MAXLINE + 1], /* Line from input data file       */
      wline[MAXLINE + 1]; /* Working copy of input line      */
  int sect, newsect,      /* Data sections                   */
      errcode = 0,        /* Error code                      */
      inperr, errsum;     /* Error code & total error count  */

  /* Allocate input buffer */
  par->X = (double *)calloc(MAXTOKS, sizeof(double));
  ERRCODE(MEMCHECK(par->X));

  if (!errcode) {

    /* Initialize number of network components */
    par->Ntitle = 0;
    net->Nnodes = 0;
    net->Njuncs = 0;
    net->Ntanks = 0;
    net->Nlinks = 0;
    net->Npipes = 0;
    net->Npumps = 0;
    net->Nvalves = 0;
    net->Ncontrols = 0;
    net->Nrules = 0;
    net->Ncurves = par->MaxCurves;
    net->Npats = par->MaxPats;
    par->PrevPat = NULL;
    par->PrevCurve = NULL;

    sect = -1;
    errsum = 0;

    /* Read each line from input file. */
    while (fgets(line, MAXLINE, par->InFile) != NULL) {

      /* Make copy of line and scan for tokens */
      strcpy(wline, line);
      par->Ntokens = gettokens(wline, par->Tok, MAXTOKS, par->Comment);

      /* Skip blank lines and comments */
      if (par->Ntokens == 0)
        continue;
      if (*par->Tok[0] == ';')
        continue;

      /* Check if max. length exceeded */
      if (strlen(line) >= MAXLINE) {
        char errMsg[MAXMSG+1];
        EN_geterror(214, errMsg, MAXMSG);
        sprintf(pr->Msg, "%s section: %s", errMsg, SectTxt[sect]);
        writeline(pr, pr->Msg);
        writeline(pr, line);
        errsum++;
      }

      /* Check if at start of a new input section */
      if (par->Tok[0][0] == '[') {
        newsect = findmatch(par->Tok[0], SectTxt);
        if (newsect >= 0) {
          sect = newsect;
          if (sect == _END)
            break;
          continue;
        } else {
          inperrmsg(pr, 201, sect, line);
          errsum++;
          break;
        }
      }

      /* Otherwise process next line of input in current section */
      else {
        if (sect >= 0) // for cases were no section is present on the top of the
                       // input file
        {
          inperr = newline(pr, sect, line);
          if (inperr > 0) {
            inperrmsg(pr,inperr, sect, line);
            errsum++;
          }
        } else {
          errcode = 200;
          break;
        }
      }

      /* Stop if reach end of file or max. error count */
      if (errsum == MAXERRS)
        break;
    } /* End of while */

    /* Check for errors */
    if (errsum > 0)
      errcode = 200;
  }

  /* Check for unlinked nodes */
  if (!errcode)
    errcode = unlinked(pr);

  /* Get pattern & curve data from temp. lists */
  if (!errcode)
    errcode = getpatterns(pr);
  if (!errcode)
    errcode = getcurves(pr);
  if (!errcode)
    errcode = getpumpparams(pr);

  /* Free input buffer */
  free(par->X);
  return (errcode);

} /*  End of readdata  */

int newline(EN_Project *pr, int sect, char *line)
/*
**--------------------------------------------------------------
**  Input:   sect  = current section of input file
**           *line = line read from input file
**  Output:  returns error code or 0 if no error found
**  Purpose: processes a new line of data from input file
**--------------------------------------------------------------
*/
{
  parser_data_t *par = &pr->parser;
  
  int n;
  switch (sect) {
  case _TITLE:
    if (par->Ntitle < 3) {
      n = (int)strlen(line);
      if (line[n - 1] == 10)
        line[n - 1] = ' ';
      strncpy(pr->Title[par->Ntitle], line, TITLELEN);
      par->Ntitle++;
    }
    return (0);
  case _JUNCTIONS:
    return (juncdata(pr));
  case _RESERVOIRS:
  case _TANKS:
    return (tankdata(pr));
  case _PIPES:
    return (pipedata(pr));
  case _PUMPS:
    return (pumpdata(pr));
  case _VALVES:
    return (valvedata(pr));
  case _PATTERNS:
    return (patterndata(pr));
  case _CURVES:
    return (curvedata(pr));
  case _DEMANDS:
    return (demanddata(pr));
  case _CONTROLS:
    return (controldata(pr));
  case _RULES:
    return (ruledata(pr)); /* See RULES.C */
  case _SOURCES:
    return (sourcedata(pr));
  case _EMITTERS:
    return (emitterdata(pr));
  case _QUALITY:
    return (qualdata(pr));
  case _STATUS:
    return (statusdata(pr));
  case _ROUGHNESS:
    return (0);
  case _ENERGY:
    return (energydata(pr));
  case _REACTIONS:
    return (reactdata(pr));
  case _MIXING:
    return (mixingdata(pr));
  case _REPORT:
    return (reportdata(pr));
  case _TIMES:
    return (timedata(pr));
  case _OPTIONS:
    return (optiondata(pr));

  /* Data in these sections are not used for any computations */
  case _COORDS:
    if (par->Coordflag == TRUE) {
      return (coordata(pr));
    } else
      return (0);
  case _LABELS:
    return (0);
  case _TAGS:
    return (0);
  case _VERTICES:
    return (0);
  case _BACKDROP:
    return (0);
  }
  return (201);
} /* end of newline */

int getpumpparams(EN_Project *pr)
/*
**-------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: computes & checks pump curve parameters
**--------------------------------------------------------------
*/
{
  int i, j = 0, k, m, n = 0;
  double a, b, c, h0 = 0.0, h1 = 0.0, h2 = 0.0, q1 = 0.0, q2 = 0.0;
  char errMsg[MAXMSG+1];
  Spump *pump;
  Slink *link;
  Scurve *curve;
  
  EN_Network *net = &pr->network;
  
  for (i = 1; i <= net->Npumps; i++) {
    pump = &net->Pump[i];
    k = pump->Link;
    link = &net->Link[k];
    if (pump->Ptype == CONST_HP) { /* Constant Hp pump */
      pump->H0 = 0.0;
      pump->R = -8.814 * link->Km;
      pump->N = -1.0;
      pump->Hmax = BIG; /* No head limit      */
      pump->Qmax = BIG; /* No flow limit      */
      pump->Q0 = 1.0;   /* Init. flow = 1 cfs */
      continue;
    }
    else if (pump->Ptype == NOCURVE) { /* Pump curve specified */
      j = pump->Hcurve; /* Get index of head curve */
      if (j == 0) {       /* Error: No head curve */
        EN_geterror(226, errMsg, MAXMSG);
        sprintf(pr->Msg, "%s link: %s", errMsg, link->ID);
        writeline(pr, pr->Msg);
        return (200);
      }
      curve = &net->Curve[j];
      curve->Type = P_CURVE;
      n = curve->Npts;
      if (n == 1) {  /* Only a single h-q point supplied so use generic */
        pump->Ptype = POWER_FUNC; /* power function curve.   */
        q1 = curve->X[0];
        h1 = curve->Y[0];
        h0 = 1.33334 * h1;
        q2 = 2.0 * q1;
        h2 = 0.0;
      } else if (n == 3 && curve->X[0] == 0.0) /* 3 h-q points supplied with */
      {                                /* shutoff head so use fitted */
        pump->Ptype = POWER_FUNC;    /* power function curve.      */
        h0 = curve->Y[0];
        q1 = curve->X[1];
        h1 = curve->Y[1];
        q2 = curve->X[2];
        h2 = curve->Y[2];
      } 
      else { // use a custom curve, referenced by ID
        pump->Ptype = CUSTOM; /* Else use custom pump curve.*/
        // at this point, j is set to that curve's index.
      }
      
      /* Compute shape factors & limits of power function pump curves */
      if (pump->Ptype == POWER_FUNC) {
        if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c)) { /* Error: Invalid curve */
          EN_geterror(227, errMsg, MAXMSG);
          sprintf(pr->Msg, "%s link: %s", errMsg, link->ID);
          writeline(pr, pr->Msg);
          return (200);
        } else {
          pump->H0 = -a;
          pump->R = -b;
          pump->N = c;
          pump->Q0 = q1;
          pump->Qmax = pow((-a / b), (1.0 / c));
          pump->Hmax = h0;
        }
      }
    }

    /* Assign limits to custom pump curves */
    if (pump->Ptype == CUSTOM) {
      curve = &net->Curve[j];
      for (m = 1; m < n; m++) {
        if (curve->Y[m] >= curve->Y[m - 1]) { /* Error: Invalid curve */
          EN_geterror(227, errMsg, MAXMSG);
          sprintf(pr->Msg, "%s link: %s", errMsg, link->ID);
          writeline(pr, pr->Msg);
          return (200);
        }
      }
      pump->Qmax = curve->X[n - 1];
      pump->Q0 = (curve->X[0] + pump->Qmax) / 2.0;
      pump->Hmax = curve->Y[0];
    }
  } /* Next pump */
  return (0);
}

int addnodeID(EN_Network *net, int n, char *id)
/*
**-------------------------------------------------------------
**  Input:   n = node index
**           id = ID label
**  Output:  returns 0 if ID already in use, 1 if not
**  Purpose: adds a node ID to the Node Hash Table
**--------------------------------------------------------------
*/
{
  if (findnode(net,id)) {
    return (0); /* see EPANET.C */
  }
  strncpy(net->Node[n].ID, id, MAXID);
  ENHashTableInsert(net->NodeHashTable, net->Node[n].ID, n); /* see HASH.C */
  return (1);
}

int addlinkID(EN_Network *net, int n, char *id)
/*
**-------------------------------------------------------------
**  Input:   n = link index
**           id = ID label
**  Output:  returns 0 if ID already in use, 1 if not
**  Purpose: adds a link ID to the Link Hash Table
**--------------------------------------------------------------
*/
{
  if (findlink(net,id)) {
    return (0); /* see EPANET.C */
  }
  strncpy(net->Link[n].ID, id, MAXID);
  ENHashTableInsert(net->LinkHashTable, net->Link[n].ID, n); /* see HASH.C */
  return (1);
}

int addpattern(parser_data_t *par, char *id)
/*
**-------------------------------------------------------------
**  Input:   id = pattern ID label
**  Output:  returns error code
**  Purpose: adds a new pattern to the database
**--------------------------------------------------------------
*/
{
  
  STmplist *p;

  /* Check if ID is same as last one processed */
  if (par->Patlist != NULL && strcmp(id, par->Patlist->ID) == 0) {
    return (0);
  }

  /* Check that pattern was not already created */
  if (findID(id, par->Patlist) == NULL) {

    /* Update pattern count & create new list element */
    (par->MaxPats)++;
    p = (STmplist *)malloc(sizeof(STmplist));
    if (p == NULL)
      return (101);

    /* Initialize list element properties */
    else {
      p->i = par->MaxPats;
      strncpy(p->ID, id, MAXID);
      p->x = NULL;
      p->y = NULL;
      p->next = par->Patlist;
      par->Patlist = p;
    }
  }
  return (0);
}

int addcurve(parser_data_t *par, char *id)
/*
**-------------------------------------------------------------
**  Input:   id = curve ID label
**  Output:  returns error code
**  Purpose: adds a new curve to the database
**--------------------------------------------------------------
*/
{
  STmplist *c;

  /* Check if ID is same as last one processed */
  if (par->Curvelist != NULL && strcmp(id, par->Curvelist->ID) == 0)
    return (0);

  /* Check that curve was not already created */
  if (findID(id, par->Curvelist) == NULL) {

    /* Update curve count & create new list element */
    (par->MaxCurves)++;
    c = (STmplist *)malloc(sizeof(STmplist));
    if (c == NULL)
      return (101);

    /* Initialize list element properties */
    else {
      c->i = par->MaxCurves;
      strncpy(c->ID, id, MAXID);
      c->x = NULL;
      c->y = NULL;
      c->next = par->Curvelist;
      par->Curvelist = c;
    }
  }
  return (0);
}

STmplist *findID(char *id, STmplist *list)
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
  for (item = list; item != NULL; item = item->next) {
    if (strcmp(item->ID, id) == 0) {
      return (item);
    }
  }
  return (NULL);
}

int unlinked(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  int *marked;
  int i, err, errcode;
  char errMsg[MAXMSG+1];
  
  errcode = 0;
  err = 0;
  marked = (int *)calloc(net->Nnodes + 1, sizeof(int));
  ERRCODE(MEMCHECK(marked));
  if (!errcode) {
    memset(marked, 0, (net->Nnodes + 1) * sizeof(int));
    for (i = 1; i <= net->Nlinks; i++) /* Mark end nodes of each link */
    {
      marked[net->Link[i].N1]++;
      marked[net->Link[i].N2]++;
    }
    for (i = 1; i <= net->Njuncs; i++) /* Check each junction  */
    {
      if (marked[i] == 0) /* If not marked then error */
      {
        err++;
        EN_geterror(233, errMsg, MAXMSG);
        sprintf(pr->Msg, "%s node: %s", errMsg, net->Node[i].ID);
        writeline(pr, pr->Msg);
      }
      if (err >= MAXERRS)
        break;
    }
    if (err > 0)
      errcode = 200;
  }
  free(marked);
  return (errcode);
} /* End of unlinked */

int getpatterns(EN_Project *pr)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: retrieves pattern data from temporary linked list
**-------------------------------------------------------------
*/
{
  int i, j;
  SFloatlist *f;
  STmplist *pat;
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  parser_data_t *par = &pr->parser;

  /* Start at head of list */
  pat = par->Patlist;

  /* Traverse list of patterns */
  while (pat != NULL) {

    /* Get index of current pattern in Pattern array */
    i = pat->i;

    /* Check if this is the default pattern */
    if (strcmp(pat->ID, par->DefPatID) == 0) {
      hyd->DefPat = i;
    }
    if (i >= 0 && i <= par->MaxPats) {
      /* Save pattern ID */
      
      Spattern *pattern = &net->Pattern[i];
      
      strcpy(pattern->ID, pat->ID);

      /* Give pattern a length of at least 1 */
      if (pattern->Length == 0)
        pattern->Length = 1;
      pattern->F = (double *)calloc(pattern->Length, sizeof(double));
      if (pattern->F == NULL)
        return (101);

      /* Start at head of pattern multiplier list */
      /* (which holds multipliers in reverse order)*/
      f = pat->x;
      j = pattern->Length - 1;

      /* Use at least one multiplier equal to 1.0 */
      if (f == NULL)
        pattern->F[0] = 1.0;

      /* Traverse list, storing multipliers in Pattern array */
      else
        while (f != NULL && j >= 0) {
          pattern->F[j] = f->value;
          f = f->next;
          j--;
        }
    }
    pat = pat->next;
  }
  return (0);
}

int getcurves(EN_Project *pr)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: retrieves curve data from temporary linked list
**-----------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  
  int i, j;
  double x;
  SFloatlist *fx, *fy;
  STmplist *c;

  /* Start at head of curve list */
  c = par->Curvelist;

  /* Traverse list of curves */
  while (c != NULL) {
    i = c->i;
    if (i >= 1 && i <= par->MaxCurves) {
      Scurve *curve = &net->Curve[i];
      
      /* Save curve ID */
      strcpy(curve->ID, c->ID);

      /* Check that curve has data points */
      if (curve->Npts <= 0) {
        char errMsg[MAXMSG+1];
        EN_geterror(230, errMsg, MAXMSG);
        sprintf(pr->Msg, "%s curve: %s", errMsg, curve->ID);
        writeline(pr, pr->Msg);
        return (200);
      }

      /* Allocate memory for curve data */
      curve->X = (double *)calloc(curve->Npts, sizeof(double));
      curve->Y = (double *)calloc(curve->Npts, sizeof(double));
      if (curve->X == NULL || curve->Y == NULL)
        return (101);

      /* Traverse list of x,y data */
      x = BIG;
      fx = c->x;
      fy = c->y;
      j = curve->Npts - 1;
      while (fx != NULL && fy != NULL && j >= 0) {

        /* Check that x data is in ascending order */
        if (fx->value >= x) {
          char errMsg[MAXMSG+1];
          EN_geterror(230, errMsg, MAXMSG);
          sprintf(pr->Msg, "%s node: %s", errMsg, curve->ID);
          writeline(pr, pr->Msg);
          return (200);
        }
        x = fx->value;

        /* Save x,y data in Curve structure */
        curve->X[j] = fx->value;
        fx = fx->next;
        curve->Y[j] = fy->value;
        fy = fy->next;
        j--;
      }
    }
    c = c->next;
  }
  return (0);
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
  while (keyword[i] != NULL) {
    if (match(line, keyword[i]))
      return (i);
    i++;
  }
  return (-1);
} /* end of findmatch */

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

  /*** Updated 9/7/00 ***/
  /* Fail if substring is empty */
  if (!substr[0])
    return (0);

  /* Skip leading blanks of str. */
  for (i = 0; str[i]; i++)
    if (str[i] != ' ')
      break;

  /* Check if substr matches remainder of str. */
  for (j = 0; substr[j]; i++, j++)
    if (!str[i] || UCHAR(str[i]) != UCHAR(substr[j]))
      return (0);

  return (1);
} /* end of match */

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
  
  /* Begin with no tokens */
  for (n=0; n<maxToks; n++) { 
    Tok[n] = NULL;
  }
  n = 0;
  
  /* Truncate s at start of comment */
  c = strchr(s,';');
  if (c) {
    c2 = c+1;
    if (c2) {
      // there is a comment here, after the semi-colon.
      len = strlen(c2);
      if (len > 0) {
        len = strcspn(c2, "\n\r");
        len = MIN(len, MAXMSG);
        strncpy(comment, c2, len);
        comment[MIN(len,MAXMSG)] = '\0';
      }
    }
    *c = '\0';
  }
  len = (int)strlen(s);
  
  /* Scan s for tokens until nothing left */
  while (len > 0 && n < MAXTOKS)
  {
    m = (int)strcspn(s,SEPSTR);          /* Find token length */
    len -= m+1;                     /* Update length of s */
    if (m == 0) s++;                /* No token found */
    else
    {
      if (*s == '"')               /* Token begins with quote */
      {
        s++;                      /* Start token after quote */
        m = (int)strcspn(s,"\"\n\r");  /* Find end quote (or EOL) */
      }                            
      s[m] = '\0';                 /* Null-terminate the token */
      Tok[n] = s;                  /* Save pointer to token */
      n++;                         /* Update token count */
      s += m+1;                    /* Begin next token */
    }
  }
  return(n);
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

  /* Separate clock time into hrs, min, sec. */
  for (n = 0; n < 3; n++)
    y[n] = 0.0;
  n = 0;
  s = strtok(time, ":");
  while (s != NULL && n <= 3) {
    if (!getfloat(s, &y[n]))
      return (-1.0);
    s = strtok(NULL, ":");
    n++;
  }

  /* If decimal time with units attached then convert to hours. */
  if (n == 1) {
    /*if (units[0] == '\0')       return(y[0]);*/
    if (strlen(units) == 0)
      return (y[0]);
    if (match(units, w_SECONDS))
      return (y[0] / 3600.0);
    if (match(units, w_MINUTES))
      return (y[0] / 60.0);
    if (match(units, w_HOURS))
      return (y[0]);
    if (match(units, w_DAYS))
      return (y[0] * 24.0);
  }

  /* Convert hh:mm:ss format to decimal hours */
  if (n > 1)
    y[0] = y[0] + y[1] / 60.0 + y[2] / 3600.0;

  /* If am/pm attached then adjust hour accordingly */
  /* (12 am is midnight, 12 pm is noon) */
  if (units[0] == '\0')
    return (y[0]);
  if (match(units, w_AM)) {
    if (y[0] >= 13.0)
      return (-1.0);
    if (y[0] >= 12.0)
      return (y[0] - 12.0);
    else
      return (y[0]);
  }
  if (match(units, w_PM)) {
    if (y[0] >= 13.0)
      return (-1.0);
    if (y[0] >= 12.0)
      return (y[0]);
    else
      return (y[0] + 12.0);
  }
  return (-1.0);
} /* end of hour */

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
  if (*endptr > 0)
    return (0);
  return (1);
}

int setreport(EN_Project *pr, char *s)
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
  parser_data_t *par = &pr->parser;
  par->Ntokens = gettokens(s, par->Tok, MAXTOKS, par->Comment);
  return (reportdata(pr));
}

void inperrmsg(EN_Project *pr, int err, int sect, char *line)
/*
**-------------------------------------------------------------
**  Input:   err     = error code
**           sect    = input data section
**           *line   = line from input file
**  Output:  none
**  Purpose: displays input error message
**-------------------------------------------------------------
*/
{
  parser_data_t *par = &pr->parser;
  
  char errStr[MAXMSG + 1];
  char id[MAXMSG + 1];
  
  EN_geterror(err, errStr, MAXMSG);
  
  /* get text for error message */
  sprintf(pr->Msg, "%s - section: %s", errStr, SectTxt[sect]);
  
  // append ID?
  /* Retrieve ID label of object with input error */
  /* (No ID used for CONTROLS or REPORT sections).*/
  switch (sect) {
    case _CONTROLS:
    case _REPORT:
      // don't append
      break;
    case _ENERGY:
      sprintf(id, " id: %s", par->Tok[1]);
      break;
    default:
      sprintf(id, " id: %s", par->Tok[0]);
      break;
  }
    
  strcat(pr->Msg, id);
  writeline(pr, pr->Msg);

  /* Echo input line for syntax errors, and   */
  /* errors in CONTROLS and OPTIONS sections. */
  if (sect == _CONTROLS || err == 201 || err == 213)
    writeline(pr, line);
  else
    writeline(pr, "");
}

/********************** END OF INPUT2.C ************************/
