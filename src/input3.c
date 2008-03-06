/*
**********************************************************************

INPUT3.C -- Input data parser for EPANET

VERSION:    2.00
DATE:       5/30/00
            9/7/00
            10/25/00
            3/1/01
            6/24/02
            8/15/07    (2.00.11)
            2/14/08    (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL

This module parses data from each line of input from file InFile.
All functions in this module are called from newline() in INPUT2.C.

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

/* Defined in enumstxt.h in EPANET.C */
extern char *MixTxt[];
extern char *Fldname[]; 

/* Defined in INPUT2.C */
extern char      *Tok[MAXTOKS];
extern STmplist  *PrevPat;
extern STmplist  *PrevCurve;
extern int       Ntokens;


int  juncdata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes junction data                             
**  Format:                                                      
**    [JUNCTIONS]                                              
**      id  elev.  (demand)  (demand pattern)                  
**--------------------------------------------------------------
*/
{
   int      n, p = 0;
   double    el,y = 0.0;
   Pdemand  demand;
   STmplist *pat;

/* Add new junction to data base */
   n = Ntokens;
   if (Nnodes == MaxNodes) return(200);
   Njuncs++;
   Nnodes++;
   if (!addnodeID(Njuncs,Tok[0])) return(215);

/* Check for valid data */
   if (n < 2) return(201);
   if (!getfloat(Tok[1],&el)) return(202);
   if (n >= 3  && !getfloat(Tok[2],&y)) return(202);
   if (n >= 4)
   {
      pat = findID(Tok[3],Patlist);
      if (pat == NULL) return(205);
      p = pat->i;
   }

/* Save junction data */
   Node[Njuncs].El  = el;
   Node[Njuncs].C0  = 0.0;
   Node[Njuncs].S   = NULL;
   Node[Njuncs].Ke  = 0.0;
   Node[Njuncs].Rpt = 0;

/* Create a new demand record */
/*** Updated 6/24/02 ***/
   if (n >= 3)
   {
      demand = (struct Sdemand *) malloc(sizeof(struct Sdemand));
      if (demand == NULL) return(101);
      demand->Base = y;
      demand->Pat = p;
      demand->next = Node[Njuncs].D;
      Node[Njuncs].D = demand;
      D[Njuncs] = y;
   }
   else D[Njuncs] = MISSING;
/*** end of update ***/
   return(0);
}                        /* end of juncdata */


int  tankdata()
/*
**--------------------------------------------------------------
**  Input:   none                                              
**  Output:  returns error code                                
**  Purpose: processes tank & reservoir data                   
**  Format:                                                    
**   [RESERVOIRS]                                            
**     id elev (pattern)                                     
**   [TANKS]                                                 
**     id elev (pattern)                                     
**     id elev initlevel minlevel maxlevel diam (minvol vcurve)         
**--------------------------------------------------------------
*/
{
   int   i,               /* Node index */
         n,               /* # data items */
         p = 0,           /* Fixed grade time pattern index */
         vcurve = 0;      /* Volume curve index */
   double el        = 0.0, /* Elevation */
         initlevel = 0.0, /* Initial level */
         minlevel  = 0.0, /* Minimum level */
         maxlevel  = 0.0, /* Maximum level */
         minvol    = 0.0, /* Minimum volume */
         diam      = 0.0, /* Diameter */
         area;            /* X-sect. area */
   STmplist *t;

/* Add new tank to data base */
   n = Ntokens;
   if (Ntanks == MaxTanks
   ||  Nnodes == MaxNodes) return(200);
   Ntanks++;
   Nnodes++;
   i = MaxJuncs + Ntanks;                    /* i = node index.     */
   if (!addnodeID(i,Tok[0])) return(215);    /* Add ID to database. */

/* Check for valid data */
   if (n < 2) return(201);                   /* Too few fields.   */
   if (!getfloat(Tok[1],&el)) return(202);   /* Read elevation    */
   if (n <= 3)                               /* Tank is reservoir.*/
   {
      if (n == 3)                            /* Pattern supplied  */
      {
         t = findID(Tok[2],Patlist);
         if (t == NULL) return(205);
         p = t->i;
      }
   }
   else if (n < 6) return(201);              /* Too few fields for tank.*/
   else
   {
      /* Check for valid input data */
      if (!getfloat(Tok[2],&initlevel)) return(202);
      if (!getfloat(Tok[3],&minlevel))  return(202);
      if (!getfloat(Tok[4],&maxlevel))  return(202);
      if (!getfloat(Tok[5],&diam))      return(202);
      if (diam < 0.0)                   return(202);
      if (n >= 7
      && !getfloat(Tok[6],&minvol))     return(202);

      /* If volume curve supplied check it exists */
      if (n == 8)
      {                           
         t = findID(Tok[7],Curvelist);
         if (t == NULL) return(202);
         vcurve = t->i;
      }
   }

   Node[i].Rpt           = 0;
   Node[i].El            = el;               /* Elevation.           */
   Node[i].C0            = 0.0;              /* Init. quality.       */
   Node[i].S             = NULL;             /* WQ source data       */     
   Node[i].Ke            = 0.0;              /* Emitter coeff.       */
   Tank[Ntanks].Node     = i;                /* Node index.          */
   Tank[Ntanks].H0       = initlevel;        /* Init. level.         */
   Tank[Ntanks].Hmin     = minlevel;         /* Min. level.          */
   Tank[Ntanks].Hmax     = maxlevel;         /* Max level.           */
   Tank[Ntanks].A        = diam;             /* Diameter.            */
   Tank[Ntanks].Pat      = p;                /* Fixed grade pattern. */
   Tank[Ntanks].Kb       = MISSING;          /* Reaction coeff.      */
   /*
   *******************************************************************
    NOTE: The min, max, & initial volumes set here are based on a     
       nominal tank diameter. They will be modified in INPUT1.C if    
       a volume curve is supplied for this tank.                      
   *******************************************************************
   */
   area = PI*SQR(diam)/4.0;
   Tank[Ntanks].Vmin = area*minlevel;
   if (minvol > 0.0) Tank[Ntanks].Vmin = minvol;
   Tank[Ntanks].V0 = Tank[Ntanks].Vmin + area*(initlevel - minlevel);
   Tank[Ntanks].Vmax = Tank[Ntanks].Vmin + area*(maxlevel - minlevel);

   Tank[Ntanks].Vcurve   = vcurve;           /* Volume curve         */
   Tank[Ntanks].MixModel = MIX1;             /* Completely mixed     */
   Tank[Ntanks].V1max    = 1.0;              /* Compart. size ratio  */
   return(0);
}                        /* end of tankdata */


int  pipedata()
/*
**--------------------------------------------------------------
**  Input:   none                                              
**  Output:  returns error code                                
**  Purpose: processes pipe data                               
**  Format:                                                    
**    [PIPE]                                                
**    id  node1  node2  length  diam  rcoeff (lcoeff) (status)          
**--------------------------------------------------------------
*/
{
   int   j1,                     /* Start-node index  */
         j2,                     /* End-node index    */
         n;                      /* # data items      */
   char  type = PIPE,            /* Link type         */
         status = OPEN;          /* Link status       */
   double length,                 /* Link length       */
         diam,                   /* Link diameter     */
         rcoeff,                 /* Roughness coeff.  */
         lcoeff = 0.0;           /* Minor loss coeff. */

/* Add new pipe to data base */
   n = Ntokens;
   if (Nlinks == MaxLinks) return(200);
   Npipes++;
   Nlinks++;
   if (!addlinkID(Nlinks,Tok[0])) return(215);

/* Check for valid data */
   if (n < 6) return(201);
   if ((j1 = findnode(Tok[1])) == 0 ||
       (j2 = findnode(Tok[2])) == 0
      ) return(203);

/*** Updated 10/25/00 ***/
   if (j1 == j2) return(222);    

   if (!getfloat(Tok[3],&length) ||
       !getfloat(Tok[4],&diam)   ||
       !getfloat(Tok[5],&rcoeff)
      ) return(202);

   if (length <= 0.0 ||
       diam   <= 0.0 ||
       rcoeff <= 0.0
      ) return(202);

   /* Case where either loss coeff. or status supplied */
   if (n == 7)
   {
      if      (match(Tok[6],w_CV))        type = CV;
      else if (match(Tok[6],w_CLOSED))    status = CLOSED;
      else if (match(Tok[6],w_OPEN))      status = OPEN;
      else if (!getfloat(Tok[6],&lcoeff)) return(202);
   }

   /* Case where both loss coeff. and status supplied */
   if (n == 8)
   {
      if (!getfloat(Tok[6],&lcoeff))   return(202);
      if      (match(Tok[7],w_CV))     type = CV;
      else if (match(Tok[7],w_CLOSED)) status = CLOSED;
      else if (match(Tok[7],w_OPEN))   status = OPEN;
      else return(202);
   }
   if (lcoeff < 0.0) return(202);

/* Save pipe data */
   Link[Nlinks].N1    = j1;                  /* Start-node index */
   Link[Nlinks].N2    = j2;                  /* End-node index   */
   Link[Nlinks].Len   = length;              /* Length           */
   Link[Nlinks].Diam  = diam;                /* Diameter         */
   Link[Nlinks].Kc    = rcoeff;              /* Rough. coeff     */
   Link[Nlinks].Km    = lcoeff;              /* Loss coeff       */
   Link[Nlinks].Kb    = MISSING;             /* Bulk coeff       */
   Link[Nlinks].Kw    = MISSING;             /* Wall coeff       */
   Link[Nlinks].Type  = type;                /* Link type        */
   Link[Nlinks].Stat  = status;              /* Link status      */
   Link[Nlinks].Rpt   = 0;                   /* Report flag      */
   return(0);
}                        /* end of pipedata */


int  pumpdata()
/*
**--------------------------------------------------------------
** Input:   none                                                
** Output:  returns error code                                  
** Purpose: processes pump data                                 
** Formats:                                                     
**  [PUMP]                                                     
**   (Version 1.x Format):                                              
**   id  node1  node2  power                                   
**   id  node1  node2  h1    q1                                
**   id  node1  node2  h0    h1   q1   h2   q2                 
**   (Version 2 Format):                                              
**   id  node1  node2  KEYWORD value {KEYWORD value ...}       
**   where KEYWORD = [POWER,HEAD,PATTERN,SPEED]                
**--------------------------------------------------------------
*/
{
   int   j,
         j1,                    /* Start-node index */
         j2,                    /* End-node index   */
         m, n;                  /* # data items     */
   double y;
   STmplist *t;                 /* Pattern record   */

/* Add new pump to data base */
   n = Ntokens;
   if (Nlinks == MaxLinks ||
       Npumps == MaxPumps
      ) return(200);
   Nlinks++;
   Npumps++;
   if (!addlinkID(Nlinks,Tok[0])) return(215);

/* Check for valid data */
   if (n < 4) return(201);
   if ((j1 = findnode(Tok[1])) == 0 ||
       (j2 = findnode(Tok[2])) == 0
      ) return(203);

/*** Updated 10/25/00 ***/
   if (j1 == j2) return(222);    

/* Save pump data */
   Link[Nlinks].N1    = j1;               /* Start-node index.  */
   Link[Nlinks].N2    = j2;               /* End-node index.    */
   Link[Nlinks].Diam  = Npumps;           /* Pump index.        */
   Link[Nlinks].Len   = 0.0;              /* Link length.       */
   Link[Nlinks].Kc    = 1.0;              /* Speed factor.      */
   Link[Nlinks].Km    = 0.0;              /* Horsepower.        */
   Link[Nlinks].Kb    = 0.0;
   Link[Nlinks].Kw    = 0.0;
   Link[Nlinks].Type  = PUMP;             /* Link type.         */
   Link[Nlinks].Stat  = OPEN;             /* Link status.       */
   Link[Nlinks].Rpt   = 0;                /* Report flag.       */
   Pump[Npumps].Link = Nlinks;            /* Link index.        */
   Pump[Npumps].Ptype = NOCURVE;          /* Type of pump curve */
   Pump[Npumps].Hcurve = 0;               /* Pump curve index   */
   Pump[Npumps].Ecurve = 0;               /* Effic. curve index */
   Pump[Npumps].Upat   = 0;               /* Utilization pattern*/
   Pump[Npumps].Ecost  = 0.0;             /* Unit energy cost   */
   Pump[Npumps].Epat   = 0;               /* Energy cost pattern*/

/* If 4-th token is a number then input follows Version 1.x format */
/* so retrieve pump curve parameters */
   if (getfloat(Tok[3],&X[0]))
   {
      m = 1;
      for (j=4; j<n; j++)
      {
         if (!getfloat(Tok[j],&X[m])) return(202);
         m++;
      }
      return(getpumpcurve(m));          /* Get pump curve params */
   }

/* Otherwise input follows Version 2 format */
/* so retrieve keyword/value pairs.         */
   m = 4;
   while (m < n)
   {
      if (match(Tok[m-1],w_POWER))          /* Const. HP curve       */
      {
         y = atof(Tok[m]);
         if (y <= 0.0) return(202);
         Pump[Npumps].Ptype = CONST_HP;
         Link[Nlinks].Km = y;
      }
      else if (match(Tok[m-1],w_HEAD))      /* Custom pump curve      */
      {
         t = findID(Tok[m],Curvelist);
         if (t == NULL) return(206);
         Pump[Npumps].Hcurve = t->i;
      }
      else if (match(Tok[m-1],w_PATTERN))   /* Speed/status pattern */
      {
         t = findID(Tok[m],Patlist);
         if (t == NULL) return(205);
         Pump[Npumps].Upat = t->i;
      }
      else if (match(Tok[m-1],w_SPEED))     /* Speed setting */
      {
         if (!getfloat(Tok[m],&y)) return(202);
         if (y < 0.0) return(202);
         Link[Nlinks].Kc = y;
      }
      else return(201);
      m = m + 2;                          /* Skip to next keyword token */
   }
   return(0);
}                        /* end of pumpdata */


int  valvedata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes valve data                                
**  Format:                                                      
**     [VALVE]                                                 
**        id  node1  node2  diam  type  setting (lcoeff)       
**--------------------------------------------------------------
*/
{
   int   j1,                    /* Start-node index   */
         j2,                    /* End-node index     */
         n;                     /* # data items       */
   char  status = ACTIVE,       /* Valve status       */
         type;                  /* Valve type         */
   double diam = 0.0,            /* Valve diameter     */
         setting,               /* Valve setting      */
         lcoeff = 0.0;          /* Minor loss coeff.  */
   STmplist *t;                 /* Curve record       */

/* Add new valve to data base */
   n = Ntokens;
   if (Nlinks == MaxLinks ||
       Nvalves == MaxValves
      ) return(200);
   Nvalves++;
   Nlinks++;
   if (!addlinkID(Nlinks,Tok[0])) return(215);

/* Check for valid data */
   if (n < 6) return(201);
   if ((j1 = findnode(Tok[1])) == 0 ||
       (j2 = findnode(Tok[2])) == 0
      ) return(203);

/*** Updated 10/25/00 ***/
   if (j1 == j2) return(222);    

   if      (match(Tok[4],w_PRV)) type = PRV;
   else if (match(Tok[4],w_PSV)) type = PSV;
   else if (match(Tok[4],w_PBV)) type = PBV;
   else if (match(Tok[4],w_FCV)) type = FCV;
   else if (match(Tok[4],w_TCV)) type = TCV;
   else if (match(Tok[4],w_GPV)) type = GPV;
   else    return(201);                      /* Illegal valve type.*/
   if (!getfloat(Tok[3],&diam)) return(202);
   if (diam <= 0.0) return(202);             /* Illegal diameter.*/
   if (type == GPV)                          /* Headloss curve for GPV */
   {
      t = findID(Tok[5],Curvelist);
      if (t == NULL) return(206);
      setting = t->i;

/*** Updated 9/7/00 ***/
      status = OPEN;

   }
   else if (!getfloat(Tok[5],&setting)) return(202);
   if (n >= 7 &&
       !getfloat(Tok[6],&lcoeff)
      ) return(202);

/* Check that PRV, PSV, or FCV not connected to a tank & */
/* check for illegal connections between pairs of valves.*/
   if ((j1 > Njuncs || j2 > Njuncs) &&
       (type == PRV || type == PSV || type == FCV)
      ) return(219);
   if (!valvecheck(type,j1,j2)) return(220);

/* Save valve data */
   Link[Nlinks].N1     = j1;                 /* Start-node index. */
   Link[Nlinks].N2     = j2;                 /* End-node index.   */
   Link[Nlinks].Diam   = diam;               /* Valve diameter.   */
   Link[Nlinks].Len    = 0.0;                /* Link length.      */
   Link[Nlinks].Kc     = setting;            /* Valve setting.    */
   Link[Nlinks].Km     = lcoeff;             /* Loss coeff        */
   Link[Nlinks].Kb     = 0.0;
   Link[Nlinks].Kw     = 0.0;
   Link[Nlinks].Type   = type;               /* Valve type.       */
   Link[Nlinks].Stat   = status;             /* Valve status.     */
   Link[Nlinks].Rpt    = 0;                  /* Report flag.      */
   Valve[Nvalves].Link = Nlinks;             /* Link index.       */
   return(0);
}                        /* end of valvedata */


int  patterndata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes time pattern data                         
**  Format:                                                      
**     [PATTERNS]                                              
**        id  mult1  mult2 .....                               
**--------------------------------------------------------------
*/
{
   int  i,n;
   double x;
   SFloatlist *f;
   STmplist   *p;
   n = Ntokens - 1;
   if (n < 1) return(201);            /* Too few values        */
   if (                               /* Check for new pattern */
          PrevPat != NULL &&
          strcmp(Tok[0],PrevPat->ID) == 0
      ) p = PrevPat;
   else p = findID(Tok[0],Patlist);
   if (p == NULL) return(205);
   for (i=1; i<=n; i++)               /* Add multipliers to list */
   {
       if (!getfloat(Tok[i],&x)) return(202);
       f = (SFloatlist *) malloc(sizeof(SFloatlist));
       if (f == NULL) return(101);
       f->value = x;
       f->next = p->x;
       p->x = f;
   }
   Pattern[p->i].Length += n;         /* Save # multipliers for pattern */
   PrevPat = p;                       /* Set previous pattern pointer */
   return(0);
}                        /* end of patterndata */


int  curvedata()
/*
**------------------------------------------------------
**  Input:   none                                        
**  Output:  returns error code                          
**  Purpose: processes curve data                        
**  Format:                                              
**     [CURVES]                                        
**      CurveID   x-value  y-value                    
**------------------------------------------------------
*/
{
   double      x,y;
   SFloatlist *fx, *fy;
   STmplist   *c;

   /* Check for valid curve ID */
   if (Ntokens < 3) return(201);
   if (
          PrevCurve != NULL &&
          strcmp(Tok[0],PrevCurve->ID) == 0
      ) c = PrevCurve;
   else c = findID(Tok[0],Curvelist);
   if (c == NULL) return(205);

   /* Check for valid data */
   if (!getfloat(Tok[1],&x)) return(202);
   if (!getfloat(Tok[2],&y)) return(202);

   /* Add new data point to curve's linked list */
   fx = (SFloatlist *) malloc(sizeof(SFloatlist));
   fy = (SFloatlist *) malloc(sizeof(SFloatlist));
   if (fx == NULL || fy == NULL) return(101);
   fx->value = x;
   fx->next = c->x;
   c->x = fx;
   fy->value = y;
   fy->next = c->y;
   c->y = fy;
   Curve[c->i].Npts++;

   /* Save the pointer to this curve */
   PrevCurve = c;
   return(0);
}


int  demanddata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes node demand data                          
**  Format:                                                      
**     [DEMANDS]                                               
**        MULTIPLY  factor                                     
**        node  base_demand  (pattern)                         
**
**  NOTE: Demands entered in this section replace those 
**        entered in the [JUNCTIONS] section
**--------------------------------------------------------------
*/
{
   int  j,n,p = 0;
   double y;
   Pdemand demand;
   STmplist *pat;

/* Extract data from tokens */
   n = Ntokens;
   if (n < 2) return(201); 
   if (!getfloat(Tok[1],&y)) return(202);

/* If MULTIPLY command, save multiplier */
   if (match(Tok[0],w_MULTIPLY))
   {
      if (y <= 0.0) return(202);
      else Dmult = y;
      return(0);
   }

/* Otherwise find node (and pattern) being referenced */
   if ((j = findnode(Tok[0])) == 0) return(208);
   if (j > Njuncs) return(208);
   if (n >= 3)
   {
      pat = findID(Tok[2],Patlist);
      if (pat == NULL)  return(205);
      p = pat->i;
   }

/* Replace any demand entered in [JUNCTIONS] section */
/* (Such demand was temporarily stored in D[]) */

/*** Updated 6/24/02 ***/
   demand = Node[j].D;
   if (demand && D[j] != MISSING)
   {
      demand->Base = y;
      demand->Pat  = p;
      D[j] = MISSING;
   }
/*** End of update ***/

/* Otherwise add a new demand to this junction */
   else
   {
      demand = (struct Sdemand *) malloc(sizeof(struct Sdemand));
      if (demand == NULL) return(101);
      demand->Base = y;
      demand->Pat = p;
      demand->next = Node[j].D;
      Node[j].D = demand;
   }
   return(0);
}                        /* end of demanddata */


int  controldata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes simple controls                         
**  Formats:                                                     
**  [CONTROLS]                                                 
**  LINK  linkID  setting IF NODE      nodeID {BELOW/ABOVE}  level 
**  LINK  linkID  setting AT TIME      value  (units)               
**  LINK  linkID  setting AT CLOCKTIME value  (units)           
**   (0)   (1)      (2)   (3) (4)       (5)     (6)          (7)
**--------------------------------------------------------------
*/
{
   int   i = 0,                /* Node index             */
         k,                    /* Link index             */
         n;                    /* # data items           */
   char  status = ACTIVE,      /* Link status            */
         type;                 /* Link or control type   */
   double setting = MISSING,    /* Link setting           */
         time = 0.0,           /* Simulation time        */
         level = 0.0;          /* Pressure or tank level */

/* Check for sufficient number of input tokens */
   n = Ntokens;
   if (n < 6) return(201);

/* Check that controlled link exists */
   k = findlink(Tok[1]);
   if (k == 0) return(204);
   type = Link[k].Type;
   if (type == CV) return(207);         /* Cannot control check valve. */

/*** Updated 9/7/00 ***/
/* Parse control setting into a status level or numerical setting. */
   if (match(Tok[2],w_OPEN))
   {
      status = OPEN;
      if (type == PUMP) setting = 1.0;
      if (type == GPV)  setting = Link[k].Kc;
   }
   else if (match(Tok[2],w_CLOSED))
   {
      status = CLOSED;
      if (type == PUMP) setting = 0.0;
      if (type == GPV)  setting = Link[k].Kc;
   }
   else if (type == GPV) return(206);
   else if (!getfloat(Tok[2],&setting)) return(202);

/*** Updated 3/1/01 ***/
/* Set status for pump in case speed setting was supplied */
/* or for pipe if numerical setting was supplied */

   if (type == PUMP || type == PIPE)
   {
      if (setting != MISSING)
      {
         if (setting < 0.0)       return(202);
         else if (setting == 0.0) status = CLOSED;
         else                     status = OPEN;
      }
   }

/* Determine type of control */
   if      (match(Tok[4],w_TIME))      type = TIMER;
   else if (match(Tok[4],w_CLOCKTIME)) type = TIMEOFDAY;
   else
   {
      if (n < 8) return(201);
      if ((i = findnode(Tok[5])) == 0) return(203);
      if      (match(Tok[6],w_BELOW)) type = LOWLEVEL;
      else if (match(Tok[6],w_ABOVE)) type = HILEVEL;
      else return(201);
   }

/* Parse control level or time */
   switch (type)
   {
      case TIMER:
      case TIMEOFDAY:
         if (n == 6) time = hour(Tok[5],"");
         if (n == 7) time = hour(Tok[5],Tok[6]);
         if (time < 0.0) return(201);
         break;
      case LOWLEVEL:
      case HILEVEL:   
         if (!getfloat(Tok[7],&level)) return(202);
         break;
   }

/* Fill in fields of control data structure */
   Ncontrols++;
   if (Ncontrols > MaxControls) return(200);
   Control[Ncontrols].Link     = k;
   Control[Ncontrols].Node     = i;
   Control[Ncontrols].Type     = type;
   Control[Ncontrols].Status   = status;
   Control[Ncontrols].Setting  = setting;
   Control[Ncontrols].Time     = (long)(3600.0*time);
   if (type == TIMEOFDAY)
      Control[Ncontrols].Time %= SECperDAY;
   Control[Ncontrols].Grade    = level;
   return(0);
}                        /* end of controldata */


int  sourcedata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes water quality source data                   
**  Formats:                                                     
**     [SOURCE]                                                
**        node  sourcetype  quality  (pattern start stop)
**                                                              
**  NOTE: units of mass-based source are mass/min                
**--------------------------------------------------------------
*/
{
   int   i,                  /* Token with quality value */
         j,                  /* Node index    */
         n,                  /* # data items  */
         p = 0;              /* Time pattern  */
   char  type = CONCEN;      /* Source type   */
   double c0 = 0;             /* Init. quality */
   STmplist *pat;
   Psource  source;

   n = Ntokens;
   if (n < 2) return(201);
   if ((j = findnode(Tok[0])) == 0) return(203);
   /* NOTE: Under old format, SourceType not supplied so let  */
   /*       i = index of token that contains quality value.   */
   i = 2;
   if      (match(Tok[1],w_CONCEN))    type = CONCEN;
   else if (match(Tok[1],w_MASS))      type = MASS;
   else if (match(Tok[1],w_SETPOINT))  type = SETPOINT;
   else if (match(Tok[1],w_FLOWPACED)) type = FLOWPACED;
   else i = 1;
   if (!getfloat(Tok[i],&c0)) return(202);      /* Illegal WQ value */

   if (n > i+1 && strlen(Tok[i+1]) > 0 && strcmp(Tok[i+1], "*") != 0 )         //(2.00.11 - LR)
   {
       pat = findID(Tok[i+1],Patlist);
       if (pat == NULL) return(205);            /* Illegal pattern. */
       p = pat->i;
   }

   source = (struct Ssource *) malloc(sizeof(struct Ssource));
   if (source == NULL) return(101);
   source->C0 = c0;
   source->Pat = p;
   source->Type = type;
   Node[j].S = source;
   return(0);
}                        /* end of sourcedata */


int  emitterdata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes junction emitter data                     
**  Format:                                                     
**     [EMITTER]                                               
**        node   K                                     
**--------------------------------------------------------------
*/
{
   int   j,                  /* Node index    */
         n;                  /* # data items  */
   double k;                  /* Flow coeff,   */

   n = Ntokens;
   if (n < 2) return(201); 
   if ((j = findnode(Tok[0])) == 0) return(203);
   if (j > Njuncs) return(209);                 /* Not a junction.*/
   if (!getfloat(Tok[1],&k)) return(202);
   if (k < 0.0) return(202);
   Node[j].Ke = k;
   return(0);
}


int  qualdata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes initial water quality data                
**  Formats:
**     [QUALITY]                                               
**        node   initqual
**        node1  node2    initqual
**--------------------------------------------------------------
*/
{
   int   j,n;
   long  i,i0,i1;
   double c0;

   if (Nnodes == 0) return(208);        /* No nodes defined yet */
   n = Ntokens;
   if (n < 2) return(0);
   if (n == 2)                          /* Single node entered  */
   {
      if ( (j = findnode(Tok[0])) == 0) return(0);
      if (!getfloat(Tok[1],&c0)) return(209);
      Node[j].C0 = c0;
   }
   else                                 /* Node range entered    */
   {
      if (!getfloat(Tok[2],&c0)) return(209);
   
      /* If numerical range supplied, then use numerical comparison */
      if ((i0 = atol(Tok[0])) > 0 && (i1 = atol(Tok[1])) > 0)
      {
         for (j=1; j<=Nnodes; j++)
         {
            i = atol(Node[j].ID);
            if (i >= i0 && i <= i1) Node[j].C0 = c0;
         }
      }
      else
      {
         for (j=1; j<=Nnodes; j++)
            if ((strcmp(Tok[0],Node[j].ID) <= 0) &&
                (strcmp(Tok[1],Node[j].ID) >= 0)
               ) Node[j].C0 = c0;
      }
   }
   return(0);
}                        /* end of qualdata */


int  reactdata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes reaction coeff. data                      
**  Formats:                                                     
**     [REACTIONS]
**        ORDER     {BULK/WALL/TANK} value                       
**        GLOBAL    BULK             coeff                                  
**        GLOBAL    WALL             coeff                                  
**        BULK      link1  (link2)   coeff                          
**        WALL      link1  (link2)   coeff                          
**        TANK      node1  (node2)   coeff                          
**        LIMITING  POTENTIAL        value                          
**        ROUGHNESS CORRELATION      value
**--------------------------------------------------------------
*/
{
   int   item,j,n;
   long  i,i1,i2;
   double y;

/* Skip line if insufficient data */
   n = Ntokens;
   if (n < 3) return(0);

/* Process input depending on keyword */
   if (match(Tok[0],w_ORDER))                    /* Reaction order */
   {
      if (!getfloat(Tok[n-1],&y)) return(213);
      if      (match(Tok[1],w_BULK)) BulkOrder = y;
      else if (match(Tok[1],w_TANK)) TankOrder = y;
      else if (match(Tok[1],w_WALL))
      {
         if (y == 0.0) WallOrder = 0.0;
         else if (y == 1.0) WallOrder = 1.0;
         else return(213);
      }
      else return(213);
      return(0);
   }
   if (match(Tok[0],w_ROUGHNESS))                /* Roughness factor */
   {
      if (!getfloat(Tok[n-1],&y)) return(213);
      Rfactor = y;
      return(0);
   }
   if (match(Tok[0],w_LIMITING))                 /* Limiting potential */
   {
      if (!getfloat(Tok[n-1],&y)) return(213);
      /*if (y < 0.0) return(213);*/
      Climit = y;
      return(0);
   }
   if (match(Tok[0],w_GLOBAL))                   /* Global rates */
   {
      if (!getfloat(Tok[n-1],&y)) return(213);
      if      (match(Tok[1],w_BULK)) Kbulk = y;
      else if (match(Tok[1],w_WALL)) Kwall = y;
      else return(201);
      return(0);
   }
   if      (match(Tok[0],w_BULK)) item = 1;      /* Individual rates */
   else if (match(Tok[0],w_WALL)) item = 2;
   else if (match(Tok[0],w_TANK)) item = 3;
   else return(201);
   strcpy(Tok[0],Tok[1]);                        /* Save id in Tok[0] */
   if (item == 3)                                /* Tank rates */
   {
      if (!getfloat(Tok[n-1],&y)) return(209);   /* Rate coeff. */
      if (n == 3)
      {
          if ( (j = findnode(Tok[1])) <= Njuncs) return(0);
          Tank[j-Njuncs].Kb = y;
      }
      else
      {
       /* If numerical range supplied, then use numerical comparison */
         if ((i1 = atol(Tok[1])) > 0 && (i2 = atol(Tok[2])) > 0)
         {
            for (j=Njuncs+1; j<=Nnodes; j++)
            {
               i = atol(Node[j].ID);
               if (i >= i1 && i <= i2) Tank[j-Njuncs].Kb = y;
            }
         }
         else for (j=Njuncs+1; j<=Nnodes; j++)
            if ((strcmp(Tok[1],Node[j].ID) <= 0) &&
                (strcmp(Tok[2],Node[j].ID) >= 0)
               ) Tank[j-Njuncs].Kb = y;
      }
   }
   else                                          /* Link rates */
   {
      if (!getfloat(Tok[n-1],&y)) return(211);   /* Rate coeff. */
      if (Nlinks == 0) return(0);
      if (n == 3)                                /* Single link */
      {
         if ( (j = findlink(Tok[1])) == 0) return(0);
         if (item == 1) Link[j].Kb = y;
         else           Link[j].Kw = y;
      }
      else                                       /* Range of links */
      {
       /* If numerical range supplied, then use numerical comparison */
         if ((i1 = atol(Tok[1])) > 0 && (i2 = atol(Tok[2])) > 0)
         {
            for (j=1; j<=Nlinks; j++)
            {
               i = atol(Link[j].ID);
               if (i >= i1 && i <= i2)
               {
                  if (item == 1) Link[j].Kb = y;
                  else           Link[j].Kw = y;
               }
            }
         }
         else for (j=1; j<=Nlinks; j++)
            if ((strcmp(Tok[1],Link[j].ID) <= 0) &&
                (strcmp(Tok[2],Link[j].ID) >= 0) )
            {
               if (item == 1) Link[j].Kb = y;
               else           Link[j].Kw = y;
            }
      }
   }
   return(0);
}                        /* end of reactdata */


int  mixingdata()
/*
**-------------------------------------------------------------
**  Input:   none                                               
**  Output:  returns error code                                 
**  Purpose: processes tank mixing data                         
**  Format:                                                     
**    [MIXING]                                                   
**     TankID  MixModel  FractVolume
**-------------------------------------------------------------
*/
{
   int   i,j,n;
   double v;

   if (Nnodes == 0) return(208);        /* No nodes defined yet */
   n = Ntokens;
   if (n < 2) return(0);
   if ( (j = findnode(Tok[0])) <= Njuncs) return(0);
   if ( (i = findmatch(Tok[1],MixTxt)) < 0) return(201);
   v = 1.0;
   if ( (i == MIX2) &&
        (n == 3) &&
        (!getfloat(Tok[2],&v))          /* Get frac. vol. for 2COMP model */
      ) return(209);
   if (v == 0.0) v = 1.0;               /* v can't be zero */
   n = j - Njuncs;
   if (Tank[n].A == 0.0) return(0);     /* Tank is a reservoir */
   Tank[n].MixModel = (char)i;
   Tank[n].V1max = v;
   return(0);
}


int  statusdata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes link initial status data                          
**  Formats:                                                     
**    [STATUS]
**       link   value
**       link1  (link2)  value                                   
**--------------------------------------------------------------
*/
{
   int   j,n;
   long  i,i0,i1;
   double y = 0.0;
   char  status = ACTIVE;

   if (Nlinks == 0) return(210);
   n = Ntokens - 1;
   if (n < 1) return(201);

/* Check for legal status setting */
   if      (match(Tok[n],w_OPEN))    status = OPEN;
   else if (match(Tok[n],w_CLOSED))  status = CLOSED;
   else if (!getfloat(Tok[n],&y))    return(211);
   if (y < 0.0)                      return(211);

/* Single link ID supplied */
   if (n == 1)
   {
      if ( (j = findlink(Tok[0])) == 0) return(0);
      /* Cannot change status of a Check Valve */
      if (Link[j].Type == CV) return(211);

/*** Updated 9/7/00 ***/      
      /* Cannot change setting for a GPV */
      if (Link[j].Type == GPV
      &&  status == ACTIVE)   return(211);

      changestatus(j,status,y);
   }

/* Range of ID's supplied */
   else
   {
      /* Numerical range supplied */
      if ((i0 = atol(Tok[0])) > 0 && (i1 = atol(Tok[1])) > 0)
      {
         for (j=1; j<=Nlinks; j++)
         {
            i = atol(Link[j].ID);
            if (i >= i0 && i <= i1) changestatus(j,status,y);
         }
      }
      else
         for (j=1; j<=Nlinks; j++)
            if ( (strcmp(Tok[0],Link[j].ID) <= 0) &&
                 (strcmp(Tok[1],Link[j].ID) >= 0)
               ) changestatus(j,status,y);
   }
   return(0);
}              /* end of statusdata */


int  energydata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes pump energy data                          
**  Formats:                                                     
**    [ENERGY]                                                   
**       GLOBAL         {PRICE/PATTERN/EFFIC}  value                 
**       PUMP   id      {PRICE/PATTERN/EFFIC}  value                 
**       DEMAND CHARGE  value                                    
**--------------------------------------------------------------
*/
{
   int j,k,n;
   double y;
   STmplist *t;

/* Check for sufficient data */
   n = Ntokens;
   if (n < 3) return(201);

/* Check first keyword */
   if (match(Tok[0],w_DMNDCHARGE))               /* Demand charge */
   {
      if (!getfloat(Tok[2], &y)) return(213);
      Dcost = y;
      return(0);
   }
   if (match(Tok[0],w_GLOBAL))                   /* Global parameter */
   {
      j = 0;
   }
   else if (match(Tok[0],w_PUMP))                /* Pump-specific parameter */
   {
      if (n < 4) return(201);
      k = findlink(Tok[1]);                      /* Check that pump exists */
      if (k == 0) return(216);
      if (Link[k].Type != PUMP) return(216);
      j = PUMPINDEX(k);
   }
   else return(201);

/* Find type of energy parameter */      
   if (match(Tok[n-2],w_PRICE))                  /* Energy price */
   {
      if (!getfloat(Tok[n-1],&y))
      {
         if (j == 0) return(213);
         else return(217);
      }
      if (j == 0) Ecost = y;
      else Pump[j].Ecost = y;
      return(0);
   }    
   else if (match(Tok[n-2],w_PATTERN))           /* Price pattern */
   {
      t = findID(Tok[n-1],Patlist);              /* Check if pattern exists */
      if (t == NULL)
      {
         if (j == 0) return(213);
         else return(217);
      }
      if (j == 0) Epat = t->i;
      else Pump[j].Epat = t->i;
      return(0);
   }
   else if (match(Tok[n-2],w_EFFIC))             /* Pump efficiency */
   {
      if (j == 0)
      {
         if (!getfloat(Tok[n-1], &y)) return(213);
         if (y <= 0.0) return(213);
         Epump = y;
      }
      else
      {
         t = findID(Tok[n-1],Curvelist);         /* Check if curve exists */ 
         if (t == NULL) return(217);
         Pump[j].Ecurve = t->i;
      }
      return(0);
   }
   return(201);
}


int  reportdata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes report options data                       
**  Formats:                                                     
**    PAGE     linesperpage                                      
**    STATUS   {NONE/YES/FULL}
**    SUMMARY  {YES/NO}
**    MESSAGES {YES/NO}
**    ENERGY   {NO/YES}                                   
**    NODES    {NONE/ALL}                                        
**    NODES    node1  node2 ...                                  
**    LINKS    {NONE/ALL}                                        
**    LINKS    link1  link2 ...                                  
**    FILE     filename
**    variable {YES/NO}                                          
**    variable {BELOW/ABOVE/PRECISION}  value 
**--------------------------------------------------------------
*/
{
   int    i,j,n;
   double  y;

   n = Ntokens - 1;
   if (n < 1) return(201);

/* Value for page size */
   if (match(Tok[0],w_PAGE))
   {
      if (!getfloat(Tok[n],&y))   return(213);
      if (y < 0.0 || y > 255.0) return(213);
      PageSize = (int) y;
      return(0);
   }

/* Request that status reports be written */
   if (match(Tok[0],w_STATUS))
   {
      if (match(Tok[n],w_NO))   Statflag = FALSE;
      if (match(Tok[n],w_YES))  Statflag = TRUE;
      if (match(Tok[n],w_FULL)) Statflag = FULL;
      return(0);
   }

/* Request summary report */
   if (match(Tok[0],w_SUMMARY))
   {
      if (match(Tok[n],w_NO))  Summaryflag = FALSE;
      if (match(Tok[n],w_YES)) Summaryflag = TRUE;
      return(0);
   }

/* Request error/warning message reporting */
   if (match(Tok[0],w_MESSAGES))
   {
      if (match(Tok[n],w_NO))  Messageflag = FALSE;
      if (match(Tok[n],w_YES)) Messageflag = TRUE;
      return(0);
   }
   

/* Request an energy usage report */
   if (match(Tok[0],w_ENERGY))
   {
      if (match(Tok[n],w_NO))  Energyflag = FALSE;
      if (match(Tok[n],w_YES)) Energyflag = TRUE;
      return(0);
   }

/* Particular reporting nodes specified */
   if (match(Tok[0],w_NODE))
   {
      if      (match(Tok[n],w_NONE)) Nodeflag = 0;  /* No nodes */
      else if (match(Tok[n],w_ALL))  Nodeflag = 1;  /* All nodes */
      else
      {
         if (Nnodes == 0) return(208);
         for (i=1; i<=n; i++)
         {
            if ( (j = findnode(Tok[i])) == 0) return(208);
            Node[j].Rpt = 1;
         }
         Nodeflag = 2;
      }
      return(0);
   }

/* Particular reporting links specified */
   if (match(Tok[0],w_LINK))
   {
      if      (match(Tok[n],w_NONE)) Linkflag = 0;
      else if (match(Tok[n],w_ALL))  Linkflag = 1;
      else
      {
         if (Nlinks == 0) return(210);
         for (i=1; i<=n; i++)
         {
            if ( (j = findlink(Tok[i])) == 0) return(210);
            Link[j].Rpt = 1;
         }
         Linkflag = 2;
      }
      return(0);
   }

/* Check if input is a reporting criterion. */

/*** Special case needed to distinguish "HEAD" from "HEADLOSS" ***/            //(2.00.11 - LR)
   if (strcomp(Tok[0], w_HEADLOSS)) i = HEADLOSS;                              //(2.00.11 - LR)
   else i = findmatch(Tok[0],Fldname);                                         //(2.00.11 - LR)
   if (i >= 0)                                                                 //(2.00.11 - LR)
/*****************************************************************/            //(2.00.11 - LR)
   {
      if (i > FRICTION) return(201);
      if (Ntokens == 1 || match(Tok[1],w_YES))
      {
         Field[i].Enabled = TRUE;
         return(0);
      }
      if (match(Tok[1],w_NO))
      {
         Field[i].Enabled = FALSE;
         return(0);
      }
      if (Ntokens < 3) return(201);
      if      (match(Tok[1],w_BELOW))  j = LOW;   /* Get relation operator */
      else if (match(Tok[1],w_ABOVE))  j = HI;    /* or precision keyword  */
      else if (match(Tok[1],w_PRECISION)) j = PREC;
      else return(201);
      if (!getfloat(Tok[2],&y)) return(201);
      if (j == PREC)
      {
         Field[i].Enabled = TRUE;
         Field[i].Precision = ROUND(y);
      }
      else Field[i].RptLim[j] = y;                /* Report limit value */
      return(0);
   }

/* Name of external report file */
   if (match(Tok[0],w_FILE))
   {
      strncpy(Rpt2Fname,Tok[1],MAXFNAME);
      return(0);
   }

/* If get to here then return error condition */
   return(201);
}                        /* end of reportdata */


int  timedata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes time options data                         
**  Formats:                                                     
**    STATISTIC                  {NONE/AVERAGE/MIN/MAX/RANGE}                                          
**    DURATION                   value   (units)                 
**    HYDRAULIC TIMESTEP         value   (units)                 
**    QUALITY TIMESTEP           value   (units)                 
**    MINIMUM TRAVELTIME         value   (units)
**    RULE TIMESTEP              value   (units)                 
**    PATTERN TIMESTEP           value   (units)
**    PATTERN START              value   (units)              
**    REPORT TIMESTEP            value   (units)                 
**    REPORT START               value   (units)                 
**    START CLOCKTIME            value   (AM PM)
**-------------------------------------------------------------
*/
{
   int    n;
   long   t;
   double  y;

   n = Ntokens - 1;
   if (n < 1) return(201);

/* Check if setting time statistic flag */
   if (match(Tok[0],w_STATISTIC))
   {
      if      (match(Tok[n],w_NONE))  Tstatflag = SERIES;
      else if (match(Tok[n],w_NO))    Tstatflag = SERIES;
      else if (match(Tok[n],w_AVG))   Tstatflag = AVG;
      else if (match(Tok[n],w_MIN))   Tstatflag = MIN;
      else if (match(Tok[n],w_MAX))   Tstatflag = MAX;
      else if (match(Tok[n],w_RANGE)) Tstatflag = RANGE;
      else return(201);
      return(0);
   }

/* Convert text time value to numerical value in seconds */
/* Examples:
**    5           = 5 * 3600 sec
**    5 MINUTES   = 5 * 60   sec
**    13:50       = 13*3600 + 50*60 sec
**    1:50 pm     = (12+1)*3600 + 50*60 sec
*/
     
   if (!getfloat(Tok[n],&y))
   {
      if ( (y = hour(Tok[n],"")) < 0.0)
      {
         if ( (y = hour(Tok[n-1],Tok[n])) < 0.0) return(213);
      }
   }
   t = (long)(3600.0*y);

/* Process the value assigned to the matched parameter */
   if      (match(Tok[0],w_DURATION))  Dur = t;      /* Simulation duration */
   else if (match(Tok[0],w_HYDRAULIC)) Hstep = t;    /* Hydraulic time step */
   else if (match(Tok[0],w_QUALITY))   Qstep = t;    /* Quality time step   */
   else if (match(Tok[0],w_RULE))      Rulestep = t; /* Rule time step      */
   else if (match(Tok[0],w_MINIMUM))   return(0);    /* Not used anymore    */
   else if (match(Tok[0],w_PATTERN))
   {
      if (match(Tok[1],w_TIME))       Pstep = t;     /* Pattern time step   */
      else if (match(Tok[1],w_START)) Pstart = t;    /* Pattern start time  */
      else return(201);
   }
   else if (match(Tok[0],w_REPORT))
   {
      if      (match(Tok[1],w_TIME))  Rstep = t;     /* Reporting time step  */
      else if (match(Tok[1],w_START)) Rstart = t;    /* Reporting start time */
      else return(201);
   }                                                 /* Simulation start time*/
   else if (match(Tok[0],w_START))    Tstart = t % SECperDAY; 
   else return(201);
   return(0);
}                        /* end of timedata */


int  optiondata()
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: processes [OPTIONS] data                            
**--------------------------------------------------------------
*/
{
   int i,n;

   n = Ntokens - 1;
   i = optionchoice(n);         /* Option is a named choice    */
   if (i >= 0) return(i);
   return(optionvalue(n));      /* Option is a numerical value */
}                        /* end of optiondata */


int  optionchoice(int n)
/*
**--------------------------------------------------------------
**  Input:   n = index of last input token saved in Tok[]          
**  Output:  returns error code or 0 if option belongs to        
**           those listed below, or -1 otherwise                 
**  Purpose: processes fixed choice [OPTIONS] data               
**  Formats:                                                     
**    UNITS               CFS/GPM/MGD/IMGD/AFD/LPS/LPM/MLD/CMH/CMD/SI
**    PRESSURE            PSI/KPA/M                          
**    HEADLOSS            H-W/D-W/C-M                        
**    HYDRAULICS          USE/SAVE  filename                  
**    QUALITY             NONE/AGE/TRACE/CHEMICAL  (TraceNode) 
**    MAP                 filename                               
**    VERIFY              filename                               
**    UNBALANCED          STOP/CONTINUE {Niter}
**    PATTERN             id
**--------------------------------------------------------------
*/
{
  /* Check if 1st token matches a parameter name and */
  /* process the input for the matched parameter     */
   if (n < 0) return(201);
   if (match(Tok[0],w_UNITS))
   {
      if (n < 1) return(0);
      else if (match(Tok[1],w_CFS))  Flowflag = CFS;
      else if (match(Tok[1],w_GPM))  Flowflag = GPM;
      else if (match(Tok[1],w_AFD))  Flowflag = AFD;
      else if (match(Tok[1],w_MGD))  Flowflag = MGD;
      else if (match(Tok[1],w_IMGD)) Flowflag = IMGD;
      else if (match(Tok[1],w_LPS))  Flowflag = LPS;
      else if (match(Tok[1],w_LPM))  Flowflag = LPM;
      else if (match(Tok[1],w_CMH))  Flowflag = CMH;
      else if (match(Tok[1],w_CMD))  Flowflag = CMD;
      else if (match(Tok[1],w_MLD))  Flowflag = MLD;
      else if (match(Tok[1],w_SI))   Flowflag = LPS;
      else return(201);
   }
   else if (match(Tok[0],w_PRESSURE))
   {
      if (n < 1) return(0);
      else if (match(Tok[1],w_PSI))    Pressflag = PSI;
      else if (match(Tok[1],w_KPA))    Pressflag = KPA;
      else if (match(Tok[1],w_METERS)) Pressflag = METERS;
      else return(201);
   }
   else if (match(Tok[0],w_HEADLOSS))
   {
      if (n < 1) return(0);
      else if (match(Tok[1],w_HW)) Formflag = HW;
      else if (match(Tok[1],w_DW)) Formflag = DW;
      else if (match(Tok[1],w_CM)) Formflag = CM;
      else return(201);
   }
   else if (match(Tok[0],w_HYDRAULIC))
   {
      if (n < 2) return(0);
      else if (match(Tok[1],w_USE))  Hydflag = USE;
      else if (match(Tok[1],w_SAVE)) Hydflag = SAVE;
      else return(201);
      strncpy(HydFname,Tok[2],MAXFNAME);
   }
   else if (match(Tok[0],w_QUALITY))
   {
      if (n < 1) return(0);
      else if (match(Tok[1],w_NONE))  Qualflag = NONE;
      else if (match(Tok[1],w_CHEM))  Qualflag = CHEM;
      else if (match(Tok[1],w_AGE))   Qualflag = AGE;
      else if (match(Tok[1],w_TRACE)) Qualflag = TRACE;
      else
      {
         Qualflag = CHEM;
         strncpy(ChemName,Tok[1],MAXID);
         if (n >= 2) strncpy(ChemUnits,Tok[2],MAXID);
      }
      if (Qualflag == TRACE)                  /* Source tracing option */
      {
      /* Copy Trace Node ID to Tok[0] for error reporting */
         strcpy(Tok[0],"");
         if (n < 2) return(212);
         strcpy(Tok[0],Tok[2]);
         TraceNode = findnode(Tok[2]);
         if (TraceNode == 0) return(212);
         strncpy(ChemName,u_PERCENT,MAXID);
         strncpy(ChemUnits,Tok[2],MAXID);
      }
      if (Qualflag == AGE)
      {
         strncpy(ChemName,w_AGE,MAXID);
         strncpy(ChemUnits,u_HOURS,MAXID);
      }
   }
   else if (match(Tok[0],w_MAP))
   {
      if (n < 1) return(0);
      strncpy(MapFname,Tok[1],MAXFNAME);        /* Map file name */
   }
   else if (match(Tok[0],w_VERIFY))
   {
      /* Backward compatibility for verification file */
   }
   else if (match(Tok[0],w_UNBALANCED))         /* Unbalanced option */
   {
      if (n < 1) return(0);
      if (match(Tok[1],w_STOP)) ExtraIter = -1;
      else if (match(Tok[1],w_CONTINUE))
      {
         if (n >= 2) ExtraIter = atoi(Tok[2]);
         else ExtraIter = 0;
      }
      else return(201);
   }
   else if (match(Tok[0],w_PATTERN))            /* Pattern option */
   {
      if (n < 1) return(0);
      strncpy(DefPatID,Tok[1],MAXID);
   }
   else return(-1);
   return(0);
}                        /* end of optionchoice */


int  optionvalue(int n)
/*
**------------------------------------------------------------- 
**  Input:   *line = line read from input file                   
**  Output:  returns error code                                  
**  Purpose: processes numerical value [OPTIONS] data            
**  Formats:
**    DEMAND MULTIPLIER   value
**    EMITTER EXPONENT    value                                                     
**    VISCOSITY           value                                  
**    DIFFUSIVITY         value                                  
**    SPECIFIC GRAVITY    value                                  
**    TRIALS              value                                  
**    ACCURACY            value                                  
**    TOLERANCE           value                                  
**    SEGMENTS            value  (not used)                                 
**  ------ Undocumented Options -----                            
**    HTOL                value                                  
**    QTOL                value                                  
**    RQTOL               value                                  
**    CHECKFREQ           value                                  
**    MAXCHECK            value
**    DAMPLIMIT           value                                                //(2.00.12 - LR)                                  
**--------------------------------------------------------------
*/
{
   int    nvalue = 1;   /* Index of token with numerical value */
   double  y;

/* Check for obsolete SEGMENTS keyword */
   if (match(Tok[0],w_SEGMENTS)) return(0);

/* Check for missing value (which is permissible) */
   if (match(Tok[0],w_SPECGRAV) || match(Tok[0],w_EMITTER)
   || match(Tok[0],w_DEMAND)) nvalue = 2;
   if (n < nvalue) return(0);

/* Check for valid numerical input */
   if (!getfloat(Tok[nvalue],&y)) return(213);

/* Check for WQ tolerance option (which can be 0) */
   if (match(Tok[0],w_TOLERANCE))
   {
      if (y < 0.0) return(213);
      Ctol = y;         /* Quality tolerance*/
      return(0);
   }

/* Check for Diffusivity option */
   if (match(Tok[0],w_DIFFUSIVITY))
   {
      if (y < 0.0) return(213);
      Diffus = y;
      return(0);
   }

/* Check for Damping Limit option */                                           //(2.00.12 - LR)
   if (match(Tok[0],w_DAMPLIMIT))
   {
      DampLimit = y;
      return(0);
   }

/* All other options must be > 0 */
   if (y <= 0.0) return(213);

/* Assign value to specified option */
   if      (match(Tok[0],w_VISCOSITY))   Viscos = y;       /* Viscosity */
   else if (match(Tok[0],w_SPECGRAV))    SpGrav = y;       /* Spec. gravity */
   else if (match(Tok[0],w_TRIALS))      MaxIter = (int)y; /* Max. trials */
   else if (match(Tok[0],w_ACCURACY))                      /* Accuracy */
   {
      y = MAX(y,1.e-5);                                  
      y = MIN(y,1.e-1);
      Hacc = y;
   }
   else if (match(Tok[0],w_HTOL))        Htol = y;
   else if (match(Tok[0],w_QTOL))        Qtol = y;
   else if (match(Tok[0],w_RQTOL))
   {
      if (y >= 1.0) return(213);
      RQtol = y;
   }
   else if (match(Tok[0],w_CHECKFREQ))   CheckFreq = (int)y;
   else if (match(Tok[0],w_MAXCHECK))    MaxCheck = (int)y;
   else if (match(Tok[0],w_EMITTER))     Qexp = 1.0/y;
   else if (match(Tok[0],w_DEMAND))      Dmult = y;
   else return(201);
   return(0);
}                        /* end of optionvalue */


int  getpumpcurve(int n)
/*
**--------------------------------------------------------
**  Input:   n = number of parameters for pump curve
**  Output:  returns error code
**  Purpose: processes pump curve data for Version 1.1-
**           style input data
**  Notes:
**    1. Called by pumpdata() in INPUT3.C
**    2. Current link index & pump index of pump being
**       processed is found in global variables Nlinks
**       and Npumps, respectively
**    3. Curve data read from input line is found in
**       global variables X[0],...X[n-1]
**---------------------------------------------------------
*/
{
   double a,b,c,h0,h1,h2,q1,q2;

   if (n == 1)                /* Const. HP curve       */
   {
      if (X[0] <= 0.0) return(202);
      Pump[Npumps].Ptype = CONST_HP;
      Link[Nlinks].Km = X[0];
   }
   else
   {
      if (n == 2)             /* Generic power curve   */
      {
         q1 = X[1];
         h1 = X[0];
         h0 = 1.33334*h1;
         q2 = 2.0*q1;
         h2 = 0.0;
      }
      else if (n >= 5)        /* 3-pt. power curve     */
      {
         h0 = X[0];
         h1 = X[1];
         q1 = X[2];
         h2 = X[3];
         q2 = X[4];
      }
      else return(202);
      Pump[Npumps].Ptype = POWER_FUNC;
      if (!powercurve(h0,h1,h2,q1,q2,&a,&b,&c)) return(206);
      Pump[Npumps].H0 = -a;
      Pump[Npumps].R  = -b;
      Pump[Npumps].N  = c;
      Pump[Npumps].Q0 = q1;
      Pump[Npumps].Qmax  = pow((-a/b),(1.0/c));
      Pump[Npumps].Hmax  = h0;
   }
   return(0);
}


int  powercurve(double h0, double h1, double h2, double q1,
                double q2, double *a, double *b, double *c)
/*
**---------------------------------------------------------
**  Input:   h0 = shutoff head
**           h1 = design head
**           h2 = head at max. flow
**           q1 = design flow
**           q2 = max. flow
**  Output:  *a, *b, *c = pump curve coeffs. (H = a-bQ^c),
**           Returns 1 if sucessful, 0 otherwise.
**  Purpose: computes coeffs. for pump curve
**----------------------------------------------------------
*/
{
    double h4,h5;
    if (
          h0      < TINY ||
          h0 - h1 < TINY ||
          h1 - h2 < TINY ||
          q1      < TINY ||
          q2 - q1 < TINY
                           ) return(0);
    *a = h0;
    h4 = h0 - h1;
    h5 = h0 - h2;
    *c = log(h5/h4)/log(q2/q1);
    if (*c <= 0.0 || *c > 20.0) return(0);
    *b = -h4/pow(q1,*c);

    /*** Updated 6/24/02 ***/
    if (*b >= 0.0) return(0);

    return(1);
}


int  valvecheck(int type, int j1, int j2)
/*
**--------------------------------------------------------------
**  Input:   type = valve type                                   
**           j1   = index of upstream node                       
**           j2   = index of downstream node                     
**  Output:  returns 1 for legal connection, 0 otherwise         
**  Purpose: checks for legal connections between PRVs & PSVs    
**--------------------------------------------------------------
*/
{
   int  k, vk, vj1, vj2, vtype;

   /* Examine each existing valve */
   for (k=1; k<=Nvalves; k++)
   {
      vk = Valve[k].Link;
      vj1 = Link[vk].N1;
      vj2 = Link[vk].N2;
      vtype = Link[vk].Type;

      /* Cannot have two PRVs sharing downstream nodes or in series */
      if (vtype == PRV && type == PRV)
      {
         if (vj2 == j2 ||
             vj2 == j1 ||
             vj1 == j2   ) return(0);
      }

      /* Cannot have two PSVs sharing upstream nodes or in series */
      if (vtype == PSV && type == PSV)
      {
         if (vj1 == j1 ||
             vj1 == j2 ||
             vj2 == j1   ) return(0);
      }

      /* Cannot have PSV connected to downstream node of PRV */
      if (vtype == PSV && type == PRV && vj1 == j2) return(0);
      if (vtype == PRV && type == PSV && vj2 == j1) return(0);

/*** Updated 3/1/01 ***/
      /* Cannot have PSV connected to downstream node of FCV */
      /* nor have PRV connected to upstream node of FCV */
      if (vtype == FCV && type == PSV && vj2 == j1) return(0);
      if (vtype == FCV && type == PRV && vj1 == j2) return(0);

/*** Updated 4/14/05 ***/
      if (vtype == PSV && type == FCV && vj1 == j2) return (0);
      if (vtype == PRV && type == FCV && vj2 == j1) return (0);
   }
   return(1);
}                   /* End of valvecheck */


void  changestatus(int j, char status, double y)
/*
**--------------------------------------------------------------
**  Input:   j      = link index                                   
**           status = status setting (OPEN, CLOSED)                       
**           y      = numerical setting (pump speed, valve
**                    setting)
**  Output:  none
**  Purpose: changes status or setting of a link
**
**  NOTE: If status = ACTIVE, then a numerical setting (y) was
**        supplied. If status = OPEN/CLOSED, then numerical
**        setting is 0.
**--------------------------------------------------------------
*/
{
   if (Link[j].Type == PIPE || Link[j].Type == GPV)
   {
      if (status != ACTIVE) Link[j].Stat = status;
   }
   else if (Link[j].Type == PUMP)
   {
      if (status == ACTIVE)
      {
         Link[j].Kc = y;
         status = OPEN;
         if (y == 0.0) status = CLOSED;
      }
      else if (status == OPEN) Link[j].Kc = 1.0;
      Link[j].Stat = status;
   }
   else if (Link[j].Type >= PRV)
   {
      Link[j].Kc = y;
      Link[j].Stat = status;
      if (status != ACTIVE) Link[j].Kc = MISSING;
   }
}                        /* end of changestatus */

/********************** END OF INPUT3.C ************************/

