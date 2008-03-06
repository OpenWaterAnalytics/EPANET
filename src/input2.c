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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "hash.h"
#include "text.h"
#include "types.h"
#include "funcs.h"
#define  EXTERN  extern
#include "vars.h"

#define   MAXERRS     10  /* Max. input errors reported        */

int    Ntokens,           /* Number of tokens in input line    */
       Ntitle;            /* Number of title lines             */
char   *Tok[MAXTOKS];     /* Array of token strings            */

                          /* Used in INPUT3.C: */
STmplist  *PrevPat;       /* Pointer to pattern list element   */
STmplist  *PrevCurve;     /* Pointer to curve list element     */

                          /* Defined in enumstxt.h in EPANET.C */
extern char *SectTxt[];   /* Input section keywords            */
extern char *RptSectTxt[];


int  netsize()
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: determines number of system components
**--------------------------------------------------------------
*/
{
   char  line[MAXLINE+1];     /* Line from input data file    */
   char  *tok;                /* First token of line          */
   int   sect,newsect;        /* Input data sections          */
   int   errcode = 0;         /* Error code                   */

/* Initialize network component counts */
   MaxJuncs    = 0;
   MaxTanks    = 0;
   MaxPipes    = 0;
   MaxPumps    = 0;
   MaxValves   = 0;
   MaxControls = 0;
   MaxRules    = 0;
   MaxCurves   = 0;
   sect        = -1;

/* Add a default pattern 0 */
   MaxPats = -1;
   addpattern("");

/* Make pass through data file counting number of each component */
   while (fgets(line,MAXLINE,InFile) != NULL)
   {
   /* Skip blank lines & those beginning with a comment */
      tok = strtok(line,SEPSTR);
      if (tok == NULL) continue;
      if (*tok == ';') continue;

   /* Check if line begins with a new section heading */
      if (*tok == '[')
      {
         newsect = findmatch(tok,SectTxt);
         if (newsect >= 0)
         {
            sect = newsect;
            if (sect == _END) break;
            continue;
         }
         else continue;
      }

   /* Add to count of current component */
      switch(sect)
      {
            case _JUNCTIONS:  MaxJuncs++;    break;
            case _RESERVOIRS:
            case _TANKS:      MaxTanks++;    break;
            case _PIPES:      MaxPipes++;    break;
            case _PUMPS:      MaxPumps++;    break;
            case _VALVES:     MaxValves++;   break;
            case _CONTROLS:   MaxControls++; break;
            case _RULES:      addrule(tok);  break; /* See RULES.C */ 
            case _PATTERNS:   errcode = addpattern(tok);
                              break;
            case _CURVES:     errcode = addcurve(tok);
                              break;
      }
      if (errcode) break;
   }

   MaxNodes = MaxJuncs + MaxTanks;
   MaxLinks = MaxPipes + MaxPumps + MaxValves;
   if (MaxPats < 1) MaxPats = 1;
   if (!errcode)
   {
      if (MaxJuncs < 1) errcode = 223;       /* Not enough nodes */
      else if (MaxTanks == 0) errcode = 224; /* No tanks */
   }
   return(errcode);
}                        /*  End of netsize  */


int  readdata()
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: reads contents of input data file
**--------------------------------------------------------------
*/
{
   char  line[MAXLINE+1],     /* Line from input data file       */
         wline[MAXLINE+1];    /* Working copy of input line      */
   int   sect,newsect,        /* Data sections                   */
         errcode = 0,         /* Error code                      */
         inperr,errsum;       /* Error code & total error count  */

/* Allocate input buffer */
   X = (double *) calloc(MAXTOKS, sizeof(double));
   ERRCODE(MEMCHECK(X));

   if (!errcode)
   {

   /* Initialize number of network components */
      Ntitle    = 0;
      Nnodes    = 0;
      Njuncs    = 0;
      Ntanks    = 0;
      Nlinks    = 0;
      Npipes    = 0;
      Npumps    = 0;
      Nvalves   = 0;
      Ncontrols = 0;
      Nrules    = 0;
      Ncurves   = MaxCurves;
      Npats     = MaxPats;
      PrevPat   = NULL;
      PrevCurve = NULL;
      sect      = -1;
      errsum    = 0;

   /* Read each line from input file. */
      while (fgets(line,MAXLINE,InFile) != NULL)
      {

      /* Make copy of line and scan for tokens */
         strcpy(wline,line);
         Ntokens = gettokens(wline);

       /* Skip blank lines and comments */
         if (Ntokens == 0) continue;
         if (*Tok[0] == ';') continue;

      /* Check if max. length exceeded */
         if (strlen(line) >= MAXLINE)
         {
            sprintf(Msg,ERR214);
            writeline(Msg);
            writeline(line);
            errsum++;
         }

      /* Check if at start of a new input section */
         if (*Tok[0] == '[')
         {
            newsect = findmatch(Tok[0],SectTxt);
            if (newsect >= 0)
            {
               sect = newsect;
               if (sect == _END) break;
               continue;
            }
            else
            {
                inperrmsg(201,sect,line);
                errsum++;
                break;
            }
         }

      /* Otherwise process next line of input in current section */
         else
         {
            inperr = newline(sect,line);
            if (inperr > 0)
            {
               inperrmsg(inperr,sect,line);
               errsum++;
            }
         }

      /* Stop if reach end of file or max. error count */
         if (errsum == MAXERRS) break;
      }   /* End of while */

   /* Check for errors */
      if (errsum > 0)  errcode = 200;
   }

/* Check for unlinked nodes */
   if (!errcode) errcode = unlinked();

/* Get pattern & curve data from temp. lists */
   if (!errcode) errcode = getpatterns();
   if (!errcode) errcode = getcurves();
   if (!errcode) errcode = getpumpparams();

/* Free input buffer */
   free(X);
   return(errcode);

}                        /*  End of readdata  */


int  newline(int sect, char *line)
/*
**--------------------------------------------------------------
**  Input:   sect  = current section of input file
**           *line = line read from input file
**  Output:  returns error code or 0 if no error found
**  Purpose: processes a new line of data from input file
**--------------------------------------------------------------
*/
{
   int n;
   switch (sect)
   {
       case _TITLE:       if (Ntitle < 3)
                          {
                             n = strlen(line);
                             if (line[n-1] == 10) line[n-1] = ' ';
                             strncpy(Title[Ntitle],line,MAXMSG);
                             Ntitle++;
                          }
                          return(0);
       case _JUNCTIONS:   return(juncdata());
       case _RESERVOIRS:
       case _TANKS:       return(tankdata());
       case _PIPES:       return(pipedata());
       case _PUMPS:       return(pumpdata());
       case _VALVES:      return(valvedata());
       case _PATTERNS:    return(patterndata());
       case _CURVES:      return(curvedata());
       case _DEMANDS:     return(demanddata());
       case _CONTROLS:    return(controldata());
       case _RULES:       return(ruledata());   /* See RULES.C */
       case _SOURCES:     return(sourcedata());
       case _EMITTERS:    return(emitterdata());
       case _QUALITY:     return(qualdata());
       case _STATUS:      return(statusdata());
       case _ROUGHNESS:   return(0);
       case _ENERGY:      return(energydata());
       case _REACTIONS:   return(reactdata());
       case _MIXING:      return(mixingdata());
       case _REPORT:      return(reportdata());
       case _TIMES:       return(timedata());
       case _OPTIONS:     return(optiondata());

   /* Data in these sections are not used for any computations */
       case _COORDS:      return(0);
       case _LABELS:      return(0);
       case _TAGS:        return(0);
       case _VERTICES:    return(0);
       case _BACKDROP:    return(0);
   }
   return(201);
}                        /* end of newline */


int  getpumpparams(void)
/*
**-------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: computes & checks pump curve parameters
**--------------------------------------------------------------
*/
{
   int   i, j = 0, k, m, n = 0;
   double a,b,c,
	      h0 = 0.0, h1 = 0.0, h2 = 0.0, q1 = 0.0, q2 = 0.0;

   for (i=1; i<=Npumps; i++)
   {
      k = Pump[i].Link;
      if (Pump[i].Ptype == CONST_HP)      /* Constant Hp pump */
      {
         Pump[i].H0 = 0.0;
         Pump[i].R  = -8.814*Link[k].Km;
         Pump[i].N  = -1.0;
         Pump[i].Hmax  = BIG;             /* No head limit      */
         Pump[i].Qmax  = BIG;             /* No flow limit      */
         Pump[i].Q0 = 1.0;                /* Init. flow = 1 cfs */
         continue;
      }

   /* Set parameters for pump curves */
      else if (Pump[i].Ptype == NOCURVE)  /* Pump curve specified */
      {
         j = Pump[i].Hcurve;              /* Get index of head curve */
         if (j == 0)
         {                                /* Error: No head curve */
            sprintf(Msg,ERR226,Link[k].ID);
            writeline(Msg);
            return(200);
         }
         n = Curve[j].Npts;
         if (n == 1)                      /* Only a single h-q point */
         {                                /* supplied so use generic */
            Pump[i].Ptype = POWER_FUNC;   /* power function curve.   */
            q1 = Curve[j].X[0];
            h1 = Curve[j].Y[0];
            h0 = 1.33334*h1;
            q2 = 2.0*q1;
            h2 = 0.0;
         }
         else if (n == 3
              &&  Curve[j].X[0] == 0.0)   /* 3 h-q points supplied with */
         {                                /* shutoff head so use fitted */   
            Pump[i].Ptype = POWER_FUNC;   /* power function curve.      */
            h0 = Curve[j].Y[0];
            q1 = Curve[j].X[1];
            h1 = Curve[j].Y[1];
            q2 = Curve[j].X[2];
            h2 = Curve[j].Y[2];
         }
         else Pump[i].Ptype = CUSTOM;     /* Else use custom pump curve.*/

      /* Compute shape factors & limits of power function pump curves */
         if (Pump[i].Ptype == POWER_FUNC)
         {
            if (!powercurve(h0,h1,h2,q1,q2,&a,&b,&c))
            {                             /* Error: Invalid curve */ 
               sprintf(Msg,ERR227,Link[k].ID);
               writeline(Msg);
               return(200);
            }
            else
            {
               Pump[i].H0 = -a;
               Pump[i].R  = -b;
               Pump[i].N  = c;
               Pump[i].Q0 = q1;
               Pump[i].Qmax  = pow((-a/b),(1.0/c));
               Pump[i].Hmax  = h0;
            }
         }
      }

   /* Assign limits to custom pump curves */
      if (Pump[i].Ptype == CUSTOM)
      {
         for (m=1; m<n; m++)
         {
            if (Curve[j].Y[m] >= Curve[j].Y[m-1])
            {                             /* Error: Invalid curve */
               sprintf(Msg,ERR227,Link[k].ID);
               writeline(Msg);
               return(200);
            }
         }
         Pump[i].Qmax  = Curve[j].X[n-1];
         Pump[i].Q0    = (Curve[j].X[0] + Pump[i].Qmax)/2.0;
         Pump[i].Hmax  = Curve[j].Y[0];
      }
   }   /* Next pump */
   return(0);
}


int   addnodeID(int n, char *id)
/*
**-------------------------------------------------------------
**  Input:   n = node index
**           id = ID label
**  Output:  returns 0 if ID already in use, 1 if not
**  Purpose: adds a node ID to the Node Hash Table
**--------------------------------------------------------------
*/
{
    if (findnode(id)) return(0);         /* see EPANET.C */
    strncpy(Node[n].ID, id, MAXID);
    HTinsert(Nht, Node[n].ID, n);        /* see HASH.C */
    return(1);
}


int   addlinkID(int n, char *id)
/*
**-------------------------------------------------------------
**  Input:   n = link index
**           id = ID label
**  Output:  returns 0 if ID already in use, 1 if not
**  Purpose: adds a link ID to the Link Hash Table
**--------------------------------------------------------------
*/
{
    if (findlink(id)) return(0);         /* see EPANET.C */
    strncpy(Link[n].ID, id, MAXID);
    HTinsert(Lht, Link[n].ID, n);        /* see HASH.C */
    return(1);
}


int  addpattern(char *id)
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
   if (Patlist != NULL && strcmp(id,Patlist->ID) == 0) return(0);

/* Check that pattern was not already created */
   if (findID(id,Patlist) == NULL)
   {

   /* Update pattern count & create new list element */
      (MaxPats)++;
      p = (STmplist *) malloc(sizeof(STmplist));
      if (p == NULL) return(101);

   /* Initialize list element properties */
      else
      {
         p->i = MaxPats;
         strncpy(p->ID,id,MAXID);
         p->x = NULL;
         p->y = NULL;
         p->next = Patlist;
         Patlist = p;
      }
   }
   return(0);
}


int  addcurve(char *id)
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
   if (Curvelist != NULL && strcmp(id,Curvelist->ID) == 0) return(0);

/* Check that curve was not already created */
   if (findID(id,Curvelist) == NULL)
   {

   /* Update curve count & create new list element */
      (MaxCurves)++;
      c = (STmplist *) malloc(sizeof(STmplist));
      if (c == NULL) return(101);

   /* Initialize list element properties */
      else
      {
         c->i = MaxCurves;
         strncpy(c->ID,id,MAXID);
         c->x = NULL;
         c->y = NULL;
         c->next = Curvelist;
         Curvelist = c;
      }
   }
   return(0);
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
    for (item = list; item != NULL; item = item->next)
    {
        if (strcmp(item->ID,id) == 0)
        {
           return(item);
        }
    }
    return(NULL);
}


int  unlinked()
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
   char  *marked;
   int   i,err, errcode;
   errcode = 0;
   err = 0;
   marked   = (char *) calloc(Nnodes+1,sizeof(char));
   ERRCODE(MEMCHECK(marked));
   if (!errcode)
   {
      memset(marked,0,(Nnodes+1)*sizeof(char));
      for (i=1; i<=Nlinks; i++)            /* Mark end nodes of each link */
      {
         marked[Link[i].N1]++;
         marked[Link[i].N2]++;
      }
      for (i=1; i<=Njuncs; i++)            /* Check each junction  */
      {
         if (marked[i] == 0)               /* If not marked then error */
         {
            err++;
            sprintf(Msg,ERR233,Node[i].ID);
            writeline(Msg);
         }
         if (err >= MAXERRS) break;
      }
      if (err > 0) errcode = 200;
   }
   free(marked);
   return(errcode);
}                        /* End of unlinked */


int     getpatterns(void)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: retrieves pattern data from temporary linked list
**-------------------------------------------------------------
*/
{
   int i,j;
   SFloatlist *f;
   STmplist *pat;

/* Start at head of list */
   pat = Patlist;

/* Traverse list of patterns */
   while (pat != NULL)
   {

   /* Get index of current pattern in Pattern array */
      i = pat->i;

   /* Check if this is the default pattern */
      if (strcmp(pat->ID, DefPatID) == 0) DefPat = i;
      if (i >= 0 && i <= MaxPats)
      {
      /* Save pattern ID */
         strcpy(Pattern[i].ID, pat->ID);

      /* Give pattern a length of at least 1 */
         if (Pattern[i].Length == 0) Pattern[i].Length = 1;
         Pattern[i].F = (double *) calloc(Pattern[i].Length, sizeof(double));
         if (Pattern[i].F == NULL) return(101);

      /* Start at head of pattern multiplier list */
      /* (which holds multipliers in reverse order)*/
         f = pat->x;
         j = Pattern[i].Length - 1;

      /* Use at least one multiplier equal to 1.0 */
         if (f == NULL) Pattern[i].F[0] = 1.0;

      /* Traverse list, storing multipliers in Pattern array */
         else while (f != NULL && j >= 0)
         {
            Pattern[i].F[j] = f->value;
            f = f->next;
            j--;
         }
      }
      pat = pat->next;
   }
   return(0);
}


int     getcurves(void)
/*
**-----------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: retrieves curve data from temporary linked list
**-----------------------------------------------------------
*/
{
   int i,j;
   double x;
   SFloatlist *fx, *fy;
   STmplist *c;

/* Start at head of curve list */
   c = Curvelist;

/* Traverse list of curves */
   while (c != NULL)
   {
      i = c->i;
      if (i >= 1 && i <= MaxCurves)
      {

      /* Save curve ID */
         strcpy(Curve[i].ID, c->ID);

      /* Check that curve has data points */
         if (Curve[i].Npts <= 0)
         {
            sprintf(Msg,ERR230,c->ID);
            writeline(Msg);
            return(200);
         }

      /* Allocate memory for curve data */
         Curve[i].X = (double *) calloc(Curve[i].Npts, sizeof(double));
         Curve[i].Y = (double *) calloc(Curve[i].Npts, sizeof(double));
         if (Curve[i].X == NULL || Curve[i].Y == NULL) return(101);

      /* Traverse list of x,y data */
         x = BIG;
         fx = c->x;
         fy = c->y;
         j = Curve[i].Npts - 1;
         while (fx != NULL && fy != NULL && j >= 0)
         {

         /* Check that x data is in ascending order */
            if (fx->value >= x)
            {
               sprintf(Msg,ERR230,c->ID);
               writeline(Msg);
               return(200);
            }
            x = fx->value;

         /* Save x,y data in Curve structure */
            Curve[i].X[j] = fx->value;
            fx = fx->next;
            Curve[i].Y[j] = fy->value;
            fy = fy->next;
            j--;
         }
      }
      c = c->next;
   }
   return(0);
}


int  findmatch(char *line, char *keyword[])
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
      if (match(line,keyword[i])) return(i);
      i++;
   }
   return(-1);
}                        /* end of findmatch */



int  match(char *str, char *substr)
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
   int i,j;

/*** Updated 9/7/00 ***/
/* Fail if substring is empty */
   if (!substr[0]) return(0);

/* Skip leading blanks of str. */
   for (i=0; str[i]; i++)
     if (str[i] != ' ') break;

/* Check if substr matches remainder of str. */
   for (i=i,j=0; substr[j]; i++,j++)
      if (!str[i] || UCHAR(str[i]) != UCHAR(substr[j]))
         return(0);
   return(1);
}                        /* end of match */


/*** Updated 10/25/00 ***/
/* The gettokens function has been totally re-written. */

int  gettokens(char *s)
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
   int  len, m, n;
   char *c;

/* Begin with no tokens */
   for (n=0; n<MAXTOKS; n++) Tok[n] = NULL;
   n = 0;

/* Truncate s at start of comment */
   c = strchr(s,';');
   if (c) *c = '\0';
   len = strlen(s);

/* Scan s for tokens until nothing left */
   while (len > 0 && n < MAXTOKS)
   {
       m = strcspn(s,SEPSTR);          /* Find token length */
       len -= m+1;                     /* Update length of s */
       if (m == 0) s++;                /* No token found */
       else
       {
          if (*s == '"')               /* Token begins with quote */
          {
             s++;                      /* Start token after quote */
             m = strcspn(s,"\"\n\r");  /* Find end quote (or EOL) */
          }                            
          s[m] = '\0';                 /* Null-terminate the token */
          Tok[n] = s;                  /* Save pointer to token */
          n++;                         /* Update token count */
          s += m+1;                    /* Begin next token */
       }
   }
   return(n);
}                        /* End of gettokens */


double  hour(char *time, char *units)
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
   int    n;
   double  y[3];
   char   *s;

/* Separate clock time into hrs, min, sec. */
   for (n=0; n<3; n++) y[n] = 0.0;
   n = 0;
   s = strtok(time,":");
   while (s != NULL && n <= 3)
   {
      if (!getfloat(s,&y[n]))  return(-1.0);
      s = strtok(NULL,":");
      n++;
   }

/* If decimal time with units attached then convert to hours. */
    if (n == 1)
    {
      /*if (units[0] == '\0')       return(y[0]);*/
      if (strlen(units) == 0)     return(y[0]);
      if (match(units,w_SECONDS)) return(y[0]/3600.0);
      if (match(units,w_MINUTES)) return(y[0]/60.0);
      if (match(units,w_HOURS))   return(y[0]);
      if (match(units,w_DAYS))    return(y[0]*24.0);
    }

/* Convert hh:mm:ss format to decimal hours */
    if (n > 1) y[0] = y[0] + y[1]/60.0 + y[2]/3600.0;

/* If am/pm attached then adjust hour accordingly */
/* (12 am is midnight, 12 pm is noon) */
    if (units[0] == '\0')  return(y[0]);
    if (match(units,w_AM))
    {
       if (y[0] >= 13.0) return(-1.0);
       if (y[0] >= 12.0) return(y[0]-12.0);
       else return(y[0]);
    }
    if (match(units,w_PM))
    {
       if (y[0] >= 13.0) return(-1.0);
       if (y[0] >= 12.0) return(y[0]);
       else return(y[0]+12.0);
    }
    return(-1.0);
}                        /* end of hour */


int  getfloat(char *s, double *y)
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
    *y = (double) strtod(s,&endptr);
    if (*endptr > 0) return(0);
    return(1);
}


int  setreport(char *s)
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
   Ntokens = gettokens(s);
   return(reportdata());
}


void  inperrmsg(int err, int sect, char *line)
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
   char   fmt[MAXMSG+1];
   char   id[MAXMSG+1];

/* Retrieve ID label of object with input error */
/* (No ID used for CONTROLS or REPORT sections).*/
   if (sect == _CONTROLS || sect == _REPORT) strcpy(id,"");
   else if (sect == _ENERGY) strcpy(id,Tok[1]);
   else strcpy(id,Tok[0]);

/* Copy error messge to string variable fmt */
   switch (err)
   {
      case 201:   strcpy(fmt,ERR201);  break;
      case 202:   strcpy(fmt,ERR202);  break;
      case 203:   strcpy(fmt,ERR203);  break;
      case 204:   strcpy(fmt,ERR204);  break;
      case 205:   strcpy(fmt,ERR205);  break;
      case 206:   strcpy(fmt,ERR206);  break;
      case 207:   strcpy(fmt,ERR207);  break;
      case 208:   strcpy(fmt,ERR208);  break;
      case 209:   strcpy(fmt,ERR209);  break;
      case 210:   strcpy(fmt,ERR210);  break;
      case 211:   strcpy(fmt,ERR211);  break;
      case 212:   strcpy(fmt,ERR212);  break;
      case 213:   strcpy(id,"");
                  strcpy(fmt,ERR213);  break;
      case 214:   strcpy(id,"");
                  strcpy(fmt,ERR214);  break;
      case 215:   strcpy(fmt,ERR215);  break;
      case 216:   strcpy(fmt,ERR216);  break;
      case 217:   strcpy(fmt,ERR217);  break;
      case 219:   strcpy(fmt,ERR219);  break;
      case 220:   strcpy(fmt,ERR220);  break;

/*** Updated 10/25/00 ***/
      case 222:   strcpy(fmt,ERR222); break;

      default:    return;
   }

/* Write error message to Report file */
   sprintf(Msg,fmt,RptSectTxt[sect],id);
   writeline(Msg);

/* Echo input line for syntax errors, and   */
/* errors in CONTROLS and OPTIONS sections. */
   if (sect == _CONTROLS || err == 201 || err == 213) writeline(line);
   else writeline("");
}

/********************** END OF INPUT2.C ************************/


