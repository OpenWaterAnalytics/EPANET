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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
//#include "epanet2.h"
#include "funcs.h"
#include "hash.h"
#include "text.h"
#include "types.h"
#include <math.h>

/* Defined in enumstxt.h in EPANET.C */
extern char *MixTxt[];
extern char *Fldname[];
extern char *DemandModelTxt[];

/* Defined in INPUT2.C */

int juncdata(EN_Project *pr)
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
  int p = 0;
  double el, y = 0.0;
  Pdemand demand;
  STmplist *pat;

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  hydraulics_t *hyd = &pr->hydraulics;
  int n;
  int Njuncs;
  Snode *node;

  /* Add new junction to data base */
  n = par->Ntokens;
  if (net->Nnodes == par->MaxNodes) {
    return (200);
  }
  net->Njuncs++;
  net->Nnodes++;

  Njuncs = net->Njuncs;

  if (!addnodeID(net, net->Njuncs, par->Tok[0])) {
    return (215);
  }
  /* Check for valid data */
  if (n < 2)
    return (201);
  if (!getfloat(par->Tok[1], &el))
    return (202);
  if (n >= 3 && !getfloat(par->Tok[2], &y))
    return (202);
  if (n >= 4) {
    pat = findID(par->Tok[3], par->Patlist);
    if (pat == NULL)
      return (205);
    p = pat->i;
  }

  /* Save junction data */
  node = &net->Node[Njuncs];
  node->El = el;
  node->C0 = 0.0;
  node->S = NULL;
  node->Ke = 0.0;
  node->Rpt = 0;
  node->Type = EN_JUNCTION;
  strcpy(node->Comment, par->Comment);


  // create a demand record, even if no demand is specified here.
  // perhaps the [DEMANDS] section contains data, but not always.

  demand = (struct Sdemand *) malloc(sizeof(struct Sdemand));
  if (demand == NULL) {
    return(101);
  }
  demand->Base = y;
  demand->Pat = p;
  strncpy(demand->Name, "", MAXMSG);
  demand->next = NULL;
  node->D = demand;
  hyd->NodeDemand[Njuncs] = y;

  return (0);
} /* end of juncdata */

int tankdata(EN_Project *pr)
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
  int n,               /* # data items */
      p = 0,           /* Fixed grade time pattern index */
      vcurve = 0;      /* Volume curve index */
  double el = 0.0,     /* Elevation */
      initlevel = 0.0, /* Initial level */
      minlevel = 0.0,  /* Minimum level */
      maxlevel = 0.0,  /* Maximum level */
      minvol = 0.0,    /* Minimum volume */
      diam = 0.0,      /* Diameter */
      area;            /* X-sect. area */
  STmplist *t;

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  Snode *node;
  Stank *tank;
  int i;

  /* Add new tank to data base */
  n = par->Ntokens;
  if (net->Ntanks == par->MaxTanks || net->Nnodes == par->MaxNodes) {
    return (200);
  }
  net->Ntanks++;
  net->Nnodes++;

  i = par->MaxJuncs + net->Ntanks; /* i = node index.     */
  if (!addnodeID(net, i, par->Tok[0])) {
    return (215); /* Add ID to database. */
  }

  /* Check for valid data */
  if (n < 2)
    return (201); /* Too few fields.   */
  if (!getfloat(par->Tok[1], &el))
    return (202); /* Read elevation    */

  if (n <= 3) {   /* Tank is reservoir.*/
    if (n == 3) { /* Pattern supplied  */
      t = findID(par->Tok[2], par->Patlist);
      if (t == NULL)
        return (205);
      p = t->i;
    }
  } else if (n < 6) {
    return (201); /* Too few fields for tank.*/
  } else {
    /* Check for valid input data */
    if (!getfloat(par->Tok[2], &initlevel))
      return (202);
    if (!getfloat(par->Tok[3], &minlevel))
      return (202);
    if (!getfloat(par->Tok[4], &maxlevel))
      return (202);
    if (!getfloat(par->Tok[5], &diam))
      return (202);
    if (diam < 0.0)
      return (202);
    if (n >= 7 && !getfloat(par->Tok[6], &minvol))
      return (202);

    /* If volume curve supplied check it exists */
    if (n == 8) {
      t = findID(par->Tok[7], par->Curvelist);
      if (t == NULL) {
        return (202);
      }
      vcurve = t->i;
      net->Curve[t->i].Type = V_CURVE;
    }
  }

  node = &net->Node[i];
  tank = &net->Tank[net->Ntanks];

  node->Rpt = 0;
  node->El = el;  /* Elevation.           */
  node->C0 = 0.0; /* Init. quality.       */
  node->S = NULL; /* WQ source data       */
  node->Ke = 0.0; /* Emitter coeff.       */
  node->Type = (diam == 0) ? EN_RESERVOIR : EN_TANK;
  strcpy(node->Comment, par->Comment);
  tank->Node = i;        /* Node index.          */
  tank->H0 = initlevel;  /* Init. level.         */
  tank->Hmin = minlevel; /* Min. level.          */
  tank->Hmax = maxlevel; /* Max level.           */
  tank->A = diam;        /* Diameter.            */
  tank->Pat = p;         /* Fixed grade pattern. */
  tank->Kb = MISSING;    /* Reaction coeff.      */
  /*
  *******************************************************************
   NOTE: The min, max, & initial volumes set here are based on a
      nominal tank diameter. They will be modified in INPUT1.C if
      a volume curve is supplied for this tank.
  *******************************************************************
  */
  area = PI * SQR(diam) / 4.0;
  tank->Vmin = area * minlevel;
  if (minvol > 0.0)
    tank->Vmin = minvol;
  tank->V0 = tank->Vmin + area * (initlevel - minlevel);
  tank->Vmax = tank->Vmin + area * (maxlevel - minlevel);

  tank->Vcurve = vcurve; /* Volume curve         */
  tank->MixModel = MIX1; /* Completely mixed     */
  tank->V1max = 1.0;     /* Compart. size ratio  */
  return (0);
} /* end of tankdata */

int pipedata(EN_Project *pr)
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
  int j1,                     /* Start-node index  */
      j2,                     /* End-node index    */
      n;                      /* # data items      */
  EN_LinkType type = EN_PIPE; /* Link type         */
  StatType status = OPEN;     /* Link status       */
  double length,              /* Link length       */
      diam,                   /* Link diameter     */
      rcoeff,                 /* Roughness coeff.  */
      lcoeff = 0.0;           /* Minor loss coeff. */

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  Slink *link;

  /* Add new pipe to data base */
  n = par->Ntokens;
  if (net->Nlinks == par->MaxLinks)
    return (200);
  net->Npipes++;
  net->Nlinks++;
  if (!addlinkID(net, net->Nlinks, par->Tok[0]))
    return (215);

  /* Check for valid data */
  if (n < 6)
    return (201);
  if ((j1 = findnode(net,par->Tok[1])) == 0 || (j2 = findnode(net,par->Tok[2])) == 0)
    return (203);

  /*** Updated 10/25/00 ***/
  if (j1 == j2)
    return (222);

  if (!getfloat(par->Tok[3], &length) || !getfloat(par->Tok[4], &diam) ||
      !getfloat(par->Tok[5], &rcoeff))
    return (202);

  if (length <= 0.0 || diam <= 0.0 || rcoeff <= 0.0)
    return (202);

  /* Case where either loss coeff. or status supplied */
  if (n == 7) {
    if (match(par->Tok[6], w_CV))
      type = EN_CVPIPE;
    else if (match(par->Tok[6], w_CLOSED))
      status = CLOSED;
    else if (match(par->Tok[6], w_OPEN))
      status = OPEN;
    else if (!getfloat(par->Tok[6], &lcoeff))
      return (202);
  }

  /* Case where both loss coeff. and status supplied */
  if (n == 8) {
    if (!getfloat(par->Tok[6], &lcoeff))
      return (202);
    if (match(par->Tok[7], w_CV))
      type = EN_CVPIPE;
    else if (match(par->Tok[7], w_CLOSED))
      status = CLOSED;
    else if (match(par->Tok[7], w_OPEN))
      status = OPEN;
    else
      return (202);
  }
  if (lcoeff < 0.0)
    return (202);

  /* Save pipe data */
  link = &net->Link[net->Nlinks];

  link->N1 = j1;       /* Start-node index */
  link->N2 = j2;       /* End-node index   */
  link->Len = length;  /* Length           */
  link->Diam = diam;   /* Diameter         */
  link->Kc = rcoeff;   /* Rough. coeff     */
  link->Km = lcoeff;   /* Loss coeff       */
  link->Kb = MISSING;  /* Bulk coeff       */
  link->Kw = MISSING;  /* Wall coeff       */
  link->Type = type;   /* Link type        */
  link->Stat = status; /* Link status      */
  link->Rpt = 0;       /* Report flag      */
  strcpy(link->Comment, par->Comment);
  return (0);
} /* end of pipedata */

int pumpdata(EN_Project *pr)
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
  int j, j1, /* Start-node index */
      j2,    /* End-node index   */
      m, n;  /* # data items     */
  double y;
  STmplist *t; /* Pattern record   */

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  Slink *link;
  Spump *pump;

  /* Add new pump to data base */
  n = par->Ntokens;
  if (net->Nlinks == par->MaxLinks || net->Npumps == par->MaxPumps)
    return (200);
  net->Nlinks++;
  net->Npumps++;
  if (!addlinkID(net, net->Nlinks, par->Tok[0]))
    return (215);

  /* Check for valid data */
  if (n < 4)
    return (201);
  if ((j1 = findnode(net,par->Tok[1])) == 0 || (j2 = findnode(net,par->Tok[2])) == 0)
    return (203);

  /*** Updated 10/25/00 ***/
  if (j1 == j2)
    return (222);

  /* Save pump data */
  link = &net->Link[net->Nlinks];
  pump = &net->Pump[net->Npumps];

  link->N1 = j1;   /* Start-node index.  */
  link->N2 = j2;   /* End-node index.    */
  link->Diam = 0;  /* no longer Pump index. */
  link->Len = 0.0; /* Link length.       */
  link->Kc = 1.0;  /* Speed factor.      */
  link->Km = 0.0;  /* Horsepower.        */
  link->Kb = 0.0;
  link->Kw = 0.0;
  link->Type = EN_PUMP;  /* Link type.         */
  link->Stat = OPEN;     /* Link status.       */
  link->Rpt = 0;         /* Report flag.       */
  strcpy(link->Comment, par->Comment);
  pump->Link = net->Nlinks;   /* Link index.        */
  pump->Ptype = NOCURVE; /* Type of pump curve -- "NOCURVE" is a placeholder. this may be modified in getpumpparams() */
  pump->Hcurve = 0;      /* Pump curve index   */
  pump->Ecurve = 0;      /* Effic. curve index */
  pump->Upat = 0;        /* Utilization pattern*/
  pump->Ecost = 0.0;     /* Unit energy cost   */
  pump->Epat = 0;        /* Energy cost pattern*/

  /* If 4-th token is a number then input follows Version 1.x format */
  /* so retrieve pump curve parameters */
  if (getfloat(par->Tok[3], &par->X[0])) {
    m = 1;
    for (j = 4; j < n; j++) {
      if (!getfloat(par->Tok[j], &par->X[m])) {
        return (202);
      }
      m++;
    }
    return (getpumpcurve(pr,m)); /* Get pump curve params */
  }

  /* Otherwise input follows Version 2 format */
  /* so retrieve keyword/value pairs.         */
  m = 4;
  while (m < n) {
    if (match(par->Tok[m - 1], w_POWER)) /* Const. HP curve       */
    {
      y = atof(par->Tok[m]);
      if (y <= 0.0)
        return (202);
      pump->Ptype = CONST_HP;
      link->Km = y;
    }
    else if (match(par->Tok[m - 1], w_HEAD)) /* Custom pump curve      */
    {
      t = findID(par->Tok[m], par->Curvelist);
      if (t == NULL)
        return (206);
      pump->Hcurve = t->i;
    }
    else if (match(par->Tok[m - 1], w_PATTERN)) /* Speed/status pattern */
    {
      t = findID(par->Tok[m], par->Patlist);
      if (t == NULL)
        return (205);
      pump->Upat = t->i;
    }
    else if (match(par->Tok[m - 1], w_SPEED)) /* Speed setting */
    {
      if (!getfloat(par->Tok[m], &y))
        return (202);
      if (y < 0.0)
        return (202);
      link->Kc = y;
    }
    else {
      return (201);
    }
    m = m + 2; /* Skip to next keyword token */
  }
  return (0);
} /* end of pumpdata */

int valvedata(EN_Project *pr)
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
  int j1,               /* Start-node index   */
      j2,               /* End-node index     */
      n;                /* # data items       */
  char status = ACTIVE, /* Valve status       */
      type;             /* Valve type         */
  double diam = 0.0,    /* Valve diameter     */
      setting,          /* Valve setting      */
      lcoeff = 0.0;     /* Minor loss coeff.  */
  STmplist *t;          /* Curve record       */

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  Slink *link;

  /* Add new valve to data base */
  n = par->Ntokens;
  if (net->Nlinks == par->MaxLinks || net->Nvalves == par->MaxValves)
    return (200);
  net->Nvalves++;
  net->Nlinks++;
  if (!addlinkID(net, net->Nlinks, par->Tok[0]))
    return (215);

  /* Check for valid data */
  if (n < 6)
    return (201);
  if ((j1 = findnode(net,par->Tok[1])) == 0 || (j2 = findnode(net,par->Tok[2])) == 0)
    return (203);

  /*** Updated 10/25/00 ***/
  if (j1 == j2)
    return (222);

  if (match(par->Tok[4], w_PRV))
    type = EN_PRV;
  else if (match(par->Tok[4], w_PSV))
    type = EN_PSV;
  else if (match(par->Tok[4], w_PBV))
    type = EN_PBV;
  else if (match(par->Tok[4], w_FCV))
    type = EN_FCV;
  else if (match(par->Tok[4], w_TCV))
    type = EN_TCV;
  else if (match(par->Tok[4], w_GPV))
    type = EN_GPV;
  else
    return (201); /* Illegal valve type.*/
  if (!getfloat(par->Tok[3], &diam))
    return (202);
  if (diam <= 0.0)
    return (202);     /* Illegal diameter.*/
  if (type == EN_GPV) { /* Headloss curve for GPV */
    t = findID(par->Tok[5], par->Curvelist);
    if (t == NULL) {
      return (206);
    }
    setting = t->i;
    net->Curve[t->i].Type = H_CURVE;

    /*** Updated 9/7/00 ***/
    status = OPEN;
  } else if (!getfloat(par->Tok[5], &setting))
    return (202);
  if (n >= 7 && !getfloat(par->Tok[6], &lcoeff))
    return (202);

  /* Check that PRV, PSV, or FCV not connected to a tank & */
  /* check for illegal connections between pairs of valves.*/
  if ((j1 > net->Njuncs || j2 > net->Njuncs) &&
      (type == EN_PRV || type == EN_PSV || type == EN_FCV))
    return (219);
  if (!valvecheck(pr, type, j1, j2))
    return (220);

  /* Save valve data */
  link = &net->Link[net->Nlinks];
  link->N1 = j1;      /* Start-node index. */
  link->N2 = j2;      /* End-node index.   */
  link->Diam = diam;  /* Valve diameter.   */
  link->Len = 0.0;    /* Link length.      */
  link->Kc = setting; /* Valve setting.    */
  link->Km = lcoeff;  /* Loss coeff        */
  link->Kb = 0.0;
  link->Kw = 0.0;
  link->Type = type;     /* Valve type.       */
  link->Stat = status;   /* Valve status.     */
  link->Rpt = 0;         /* Report flag.      */
  strcpy(link->Comment, par->Comment);
  net->Valve[net->Nvalves].Link = net->Nlinks; /* Link index.       */
  return (0);
} /* end of valvedata */

int patterndata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  int i, n;
  double x;
  SFloatlist *f;
  STmplist *p;
  n = par->Ntokens - 1;
  if (n < 1)
    return (201); /* Too few values        */
  if (            /* Check for new pattern */
      par->PrevPat != NULL && strcmp(par->Tok[0], par->PrevPat->ID) == 0)
    p = par->PrevPat;
  else
    p = findID(par->Tok[0], par->Patlist);
  if (p == NULL)
    return (205);
  for (i = 1; i <= n; i++) /* Add multipliers to list */
  {
    if (!getfloat(par->Tok[i], &x))
      return (202);
    f = (SFloatlist *)malloc(sizeof(SFloatlist));
    if (f == NULL)
      return (101);
    f->value = x;
    f->next = p->x;
    p->x = f;
  }
  net->Pattern[p->i].Length += n; /* Save # multipliers for pattern */
  par->PrevPat = p;               /* Set previous pattern pointer */
  return (0);
} /* end of patterndata */

int curvedata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  double x, y;
  SFloatlist *fx, *fy;
  STmplist *c;

  /* Check for valid curve ID */
  if (par->Ntokens < 3)
    return (201);
  if (par->PrevCurve != NULL && strcmp(par->Tok[0], par->PrevCurve->ID) == 0)
    c = par->PrevCurve;
  else
    c = findID(par->Tok[0], par->Curvelist);
  if (c == NULL)
    return (205);

  /* Check for valid data */
  if (!getfloat(par->Tok[1], &x))
    return (202);
  if (!getfloat(par->Tok[2], &y))
    return (202);

  /* Add new data point to curve's linked list */
  fx = (SFloatlist *)malloc(sizeof(SFloatlist));
  fy = (SFloatlist *)malloc(sizeof(SFloatlist));
  if (fx == NULL || fy == NULL) {
    return (101);
  }
  fx->value = x;
  fx->next = c->x;
  c->x = fx;
  fy->value = y;
  fy->next = c->y;
  c->y = fy;
  net->Curve[c->i].Npts++;

  /* Save the pointer to this curve */
  par->PrevCurve = c;
  return (0);
}

int coordata(EN_Project *pr)
/*
 **--------------------------------------------------------------
 **  Input:   none
 **  Output:  returns error code
 **  Purpose: processes coordinate data
 **  Format:
 **    [COORD]
 **      id  x  y
 **--------------------------------------------------------------
 */
{
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  double x, y;
  int j;
  Scoord *coord;
  Snode *node;

  /* Check for valid node ID */
  if (par->Ntokens < 3)
    return (201);

  /* Check for valid data */
  if ((j = findnode(net, par->Tok[0])) == 0)
    return (203);
  if (!getfloat(par->Tok[1], &x))
    return (202);
  if (!getfloat(par->Tok[2], &y))
    return (202);

  /* Save coord data */
  coord = &net->Coord[j];
  node = &net->Node[j];
  strncpy(coord->ID, node->ID, MAXID);
  coord->X = x;
  coord->Y = y;
  coord->HaveCoords = TRUE;

  return (0);
} /* end of coordata */

int demanddata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  parser_data_t *par = &pr->parser;

  int j, n, p = 0;
  double y;
  Pdemand demand;
  Pdemand cur_demand;
  STmplist *pat;

  /* Extract data from tokens */
  n = par->Ntokens;
  if (n < 2)
    return (201);
  if (!getfloat(par->Tok[1], &y))
    return (202);

  /* If MULTIPLY command, save multiplier */
  if (match(par->Tok[0], w_MULTIPLY)) {
    if (y <= 0.0)
      return (202);
    else
      hyd->Dmult = y;
    return (0);
  }

  /* Otherwise find node (and pattern) being referenced */
  if ((j = findnode(net, par->Tok[0])) == 0)
    return (208);
  if (j > net->Njuncs)
    return (208);
  if (n >= 3) {
    pat = findID(par->Tok[2], par->Patlist);
    if (pat == NULL)
      return (205);
    p = pat->i;
  }

  /* Replace any demand entered in [JUNCTIONS] section */

  demand = net->Node[j].D;
  if (hyd->NodeDemand[j] != MISSING) {
    // first category encountered will overwrite "dummy" demand category
    // with what is specified in this section
    demand->Base = y;
    demand->Pat = p;
    strncpy(demand->Name, par->Comment, MAXMSG);
    hyd->NodeDemand[j] = MISSING; // marker - next iteration will append a new category.
  }
  else { // add new demand to junction
    cur_demand = net->Node[j].D;
    while (cur_demand->next != NULL) {
      cur_demand = cur_demand->next;
    }
    demand = (struct Sdemand *)malloc(sizeof(struct Sdemand));
    if (demand == NULL)
      return (101);
    demand->Base = y;
    demand->Pat = p;
    strncpy(demand->Name, par->Comment, MAXMSG);
    demand->next = NULL;
    cur_demand->next = demand;
  }
  return (0);
} /* end of demanddata */

int controldata(EN_Project *pr)
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

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  int i = 0,                /* Node index             */
      k,                    /* Link index             */
      n;                    /* # data items           */
  StatType status = ACTIVE; /* Link status            */
  ControlType c_type;       /* control type   */
  EN_LinkType l_type;       /* Link Type */
  double setting = MISSING, /* Link setting           */
      time = 0.0,           /* Simulation time        */
      level = 0.0;          /* Pressure or tank level */
  Scontrol *control;

  /* Check for sufficient number of input tokens */
  n = par->Ntokens;
  if (n < 6)
    return (201);

  /* Check that controlled link exists */
  k = findlink(net,par->Tok[1]);
  if (k == 0)
    return (204);
  l_type = net->Link[k].Type;
  if (l_type == EN_CVPIPE) {
    return (207); /* Cannot control check valve. */
  }
  /*** Updated 9/7/00 ***/
  /* Parse control setting into a status level or numerical setting. */
  if (match(par->Tok[2], w_OPEN)) {
    status = OPEN;
    if (l_type == EN_PUMP)
      setting = 1.0;
    if (l_type == EN_GPV)
      setting = net->Link[k].Kc;
  } else if (match(par->Tok[2], w_CLOSED)) {
    status = CLOSED;
    if (l_type == EN_PUMP)
      setting = 0.0;
    if (l_type == EN_GPV)
      setting = net->Link[k].Kc;
  } else if (l_type == EN_GPV) {
    return (206);
  } else if (!getfloat(par->Tok[2], &setting)) {
    return (202);
  }

  /*** Updated 3/1/01 ***/
  /* Set status for pump in case speed setting was supplied */
  /* or for pipe if numerical setting was supplied */

  if (l_type == EN_PUMP || l_type == EN_PIPE) {
    if (setting != MISSING) {
      if (setting < 0.0)
        return (202);
      else if (setting == 0.0)
        status = CLOSED;
      else
        status = OPEN;
    }
  }

  /* Determine type of control */
  if (match(par->Tok[4], w_TIME)) {
    c_type = TIMER;
  } else if (match(par->Tok[4], w_CLOCKTIME)) {
    c_type = TIMEOFDAY;
  } else {
    if (n < 8)
      return (201);

    if ((i = findnode(net, par->Tok[5])) == 0)
      return (203);

    if (match(par->Tok[6], w_BELOW))
      c_type = LOWLEVEL;
    else if (match(par->Tok[6], w_ABOVE))
      c_type = HILEVEL;
    else
      return (201);
  }

  /* Parse control level or time */
  switch (c_type) {
  case TIMER:
  case TIMEOFDAY:
    if (n == 6)
      time = hour(par->Tok[5], "");
    if (n == 7)
      time = hour(par->Tok[5], par->Tok[6]);
    if (time < 0.0)
      return (201);
    break;
  case LOWLEVEL:
  case HILEVEL:
    if (!getfloat(par->Tok[7], &level))
      return (202);
    break;
  }

  /* Fill in fields of control data structure */
  net->Ncontrols++;
  if (net->Ncontrols > par->MaxControls)
    return (200);

  control = &net->Control[net->Ncontrols];
  control->Link = k;
  control->Node = i;
  control->Type = c_type;
  control->Status = status;
  control->Setting = setting;
  control->Time = (long)(3600.0 * time);
  if (c_type == TIMEOFDAY)
    control->Time %= SECperDAY;
  control->Grade = level;
  return (0);
} /* end of controldata */

int sourcedata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  int i,              /* Token with quality value */
      j,              /* Node index    */
      n,              /* # data items  */
      p = 0;          /* Time pattern  */
  char type = CONCEN; /* Source type   */
  double c0 = 0;      /* Init. quality */
  STmplist *pat;
  Psource source;

  n = par->Ntokens;
  if (n < 2)
    return (201);
  if ((j = findnode(net, par->Tok[0])) == 0)
    return (203);
  /* NOTE: Under old format, SourceType not supplied so let  */
  /*       i = index of token that contains quality value.   */
  i = 2;
  if (match(par->Tok[1], w_CONCEN))
    type = CONCEN;
  else if (match(par->Tok[1], w_MASS))
    type = MASS;
  else if (match(par->Tok[1], w_SETPOINT))
    type = SETPOINT;
  else if (match(par->Tok[1], w_FLOWPACED))
    type = FLOWPACED;
  else
    i = 1;
  if (!getfloat(par->Tok[i], &c0))
    return (202); /* Illegal WQ value */

  if (n > i + 1 && strlen(par->Tok[i + 1]) > 0 &&
      strcmp(par->Tok[i + 1], "*") != 0)
  {
    pat = findID(par->Tok[i + 1], par->Patlist);
    if (pat == NULL)
      return (205); /* Illegal pattern. */
    p = pat->i;
  }

  source = (struct Ssource *)malloc(sizeof(struct Ssource));
  if (source == NULL)
    return (101);
  source->C0 = c0;
  source->Pat = p;
  source->Type = type;
  net->Node[j].S = source;
  return (0);
} /* end of sourcedata */

int emitterdata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  int j,    /* Node index    */
      n;    /* # data items  */
  double k; /* Flow coeff,   */

  n = par->Ntokens;
  if (n < 2)
    return (201);
  if ((j = findnode(net,par->Tok[0])) == 0)
    return (203);
  if (j > net->Njuncs)
    return (209); /* Not a junction.*/
  if (!getfloat(par->Tok[1], &k))
    return (202);
  if (k < 0.0)
    return (202);
  net->Node[j].Ke = k;
  return (0);
}

int qualdata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;
  Snode *Node = net->Node;

  int j, n;
  long i, i0, i1;
  double c0;

  if (net->Nnodes == 0)
    return (208); /* No nodes defined yet */
  n = par->Ntokens;
  if (n < 2)
    return (0);
  if (n == 2) /* Single node entered  */
  {
    if ((j = findnode(net,par->Tok[0])) == 0)
      return (0);
    if (!getfloat(par->Tok[1], &c0))
      return (209);
    Node[j].C0 = c0;
  } else /* Node range entered    */
  {
    if (!getfloat(par->Tok[2], &c0))
      return (209);

    /* If numerical range supplied, then use numerical comparison */
    if ((i0 = atol(par->Tok[0])) > 0 && (i1 = atol(par->Tok[1])) > 0) {
      for (j = 1; j <= net->Nnodes; j++) {
        i = atol(Node[j].ID);
        if (i >= i0 && i <= i1)
          Node[j].C0 = c0;
      }
    } else {
      for (j = 1; j <= net->Nnodes; j++)
        if ((strcmp(par->Tok[0], Node[j].ID) <= 0) &&
            (strcmp(par->Tok[1], Node[j].ID) >= 0))
          Node[j].C0 = c0;
    }
  }
  return (0);
} /* end of qualdata */

int reactdata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;

  int item, j, n;
  long i, i1, i2;
  double y;

  /* Skip line if insufficient data */
  n = par->Ntokens;
  if (n < 3)
    return (0);

  /* Process input depending on keyword */
  if (match(par->Tok[0], w_ORDER)) /* Reaction order */
  {
    if (!getfloat(par->Tok[n - 1], &y))
      return (213);
    if (match(par->Tok[1], w_BULK))
      qu->BulkOrder = y;
    else if (match(par->Tok[1], w_TANK))
      qu->TankOrder = y;
    else if (match(par->Tok[1], w_WALL)) {
      if (y == 0.0)
        qu->WallOrder = 0.0;
      else if (y == 1.0)
        qu->WallOrder = 1.0;
      else
        return (213);
    } else
      return (213);
    return (0);
  }
  if (match(par->Tok[0], w_ROUGHNESS)) /* Roughness factor */
  {
    if (!getfloat(par->Tok[n - 1], &y))
      return (213);
    qu->Rfactor = y;
    return (0);
  }
  if (match(par->Tok[0], w_LIMITING)) /* Limiting potential */
  {
    if (!getfloat(par->Tok[n - 1], &y))
      return (213);
    /*if (y < 0.0) return(213);*/
    qu->Climit = y;
    return (0);
  }
  if (match(par->Tok[0], w_GLOBAL)) /* Global rates */
  {
    if (!getfloat(par->Tok[n - 1], &y))
      return (213);
    if (match(par->Tok[1], w_BULK))
      qu->Kbulk = y;
    else if (match(par->Tok[1], w_WALL))
      qu->Kwall = y;
    else
      return (201);
    return (0);
  }
  if (match(par->Tok[0], w_BULK))
    item = 1; /* Individual rates */
  else if (match(par->Tok[0], w_WALL))
    item = 2;
  else if (match(par->Tok[0], w_TANK))
    item = 3;
  else
    return (201);
  strcpy(par->Tok[0], par->Tok[1]); /* Save id in par->Tok[0] */
  if (item == 3)                    /* Tank rates */
  {
    if (!getfloat(par->Tok[n - 1], &y))
      return (209); /* Rate coeff. */
    if (n == 3) {
      if ((j = findnode(net,par->Tok[1])) <= net->Njuncs)
        return (0);
      net->Tank[j - net->Njuncs].Kb = y;
    } else {
      /* If numerical range supplied, then use numerical comparison */
      if ((i1 = atol(par->Tok[1])) > 0 && (i2 = atol(par->Tok[2])) > 0) {
        for (j = net->Njuncs + 1; j <= net->Nnodes; j++) {
          i = atol(net->Node[j].ID);
          if (i >= i1 && i <= i2)
            net->Tank[j - net->Njuncs].Kb = y;
        }
      } else
        for (j = net->Njuncs + 1; j <= net->Nnodes; j++)
          if ((strcmp(par->Tok[1], net->Node[j].ID) <= 0) &&
              (strcmp(par->Tok[2], net->Node[j].ID) >= 0))
            net->Tank[j - net->Njuncs].Kb = y;
    }
  } else /* Link rates */
  {
    if (!getfloat(par->Tok[n - 1], &y))
      return (211); /* Rate coeff. */
    if (net->Nlinks == 0)
      return (0);
    if (n == 3) /* Single link */
    {
      if ((j = findlink(net, par->Tok[1])) == 0)
        return (0);
      if (item == 1)
        net->Link[j].Kb = y;
      else
        net->Link[j].Kw = y;
    } else /* Range of links */
    {
      /* If numerical range supplied, then use numerical comparison */
      if ((i1 = atol(par->Tok[1])) > 0 && (i2 = atol(par->Tok[2])) > 0) {
        for (j = 1; j <= net->Nlinks; j++) {
          i = atol(net->Link[j].ID);
          if (i >= i1 && i <= i2) {
            if (item == 1)
              net->Link[j].Kb = y;
            else
              net->Link[j].Kw = y;
          }
        }
      } else
        for (j = 1; j <= net->Nlinks; j++)
          if ((strcmp(par->Tok[1], net->Link[j].ID) <= 0) &&
              (strcmp(par->Tok[2], net->Link[j].ID) >= 0)) {
            if (item == 1)
              net->Link[j].Kb = y;
            else
              net->Link[j].Kw = y;
          }
    }
  }
  return (0);
} /* end of reactdata */

int mixingdata(EN_Project *pr)
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

  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  int i, j, n;
  double v;

  if (net->Nnodes == 0)
    return (208); /* No nodes defined yet */
  n = par->Ntokens;
  if (n < 2)
    return (0);
  if ((j = findnode(net, par->Tok[0])) <= net->Njuncs)
    return (0);
  if ((i = findmatch(par->Tok[1], MixTxt)) < 0)
    return (201);
  v = 1.0;
  if ((i == MIX2) && (n == 3) &&
      (!getfloat(par->Tok[2], &v)) /* Get frac. vol. for 2COMP model */
      )
    return (209);
  if (v == 0.0)
    v = 1.0; /* v can't be zero */
  n = j - net->Njuncs;
  if (net->Tank[n].A == 0.0)
    return (0); /* Tank is a reservoir */
  net->Tank[n].MixModel = (char)i;
  net->Tank[n].V1max = v;
  return (0);
}

int statusdata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  int j, n;
  long i, i0, i1;
  double y = 0.0;
  char status = ACTIVE;

  if (net->Nlinks == 0)
    return (210);
  n = par->Ntokens - 1;
  if (n < 1)
    return (201);

  /* Check for legal status setting */
  if (match(par->Tok[n], w_OPEN))
    status = OPEN;
  else if (match(par->Tok[n], w_CLOSED))
    status = CLOSED;
  else if (!getfloat(par->Tok[n], &y))
    return (211);
  if (y < 0.0)
    return (211);

  /* Single link ID supplied */
  if (n == 1) {
    if ((j = findlink(net, par->Tok[0])) == 0)
      return (0);
    /* Cannot change status of a Check Valve */
    if (net->Link[j].Type == EN_CVPIPE)
      return (211);

    /*** Updated 9/7/00 ***/
    /* Cannot change setting for a GPV */
    if (net->Link[j].Type == EN_GPV && status == ACTIVE)
      return (211);

    changestatus(net, j, status, y);
  }

  /* Range of ID's supplied */
  else {
    /* Numerical range supplied */
    if ((i0 = atol(par->Tok[0])) > 0 && (i1 = atol(par->Tok[1])) > 0) {
      for (j = 1; j <= net->Nlinks; j++) {
        i = atol(net->Link[j].ID);
        if (i >= i0 && i <= i1)
          changestatus(net, j, status, y);
      }
    } else
      for (j = 1; j <= net->Nlinks; j++)
        if ((strcmp(par->Tok[0], net->Link[j].ID) <= 0) &&
            (strcmp(par->Tok[1], net->Link[j].ID) >= 0))
          changestatus(net, j, status, y);
  }
  return (0);
} /* end of statusdata */

int energydata(EN_Project *pr)
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
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  parser_data_t *par = &pr->parser;

  Slink *Link = net->Link;
  Spump *Pump = net->Pump;

  int j, k, n;
  double y;
  STmplist *t;

  /* Check for sufficient data */
  n = par->Ntokens;
  if (n < 3)
    return (201);

  /* Check first keyword */
  if (match(par->Tok[0], w_DMNDCHARGE)) /* Demand charge */
  {
    if (!getfloat(par->Tok[2], &y))
      return (213);
    hyd->Dcost = y;
    return (0);
  }
  if (match(par->Tok[0], w_GLOBAL)) /* Global parameter */
  {
    j = 0;
  } else if (match(par->Tok[0], w_PUMP)) /* Pump-specific parameter */
  {
    if (n < 4)
      return (201);
    k = findlink(net,par->Tok[1]); /* Check that pump exists */
    if (k == 0)
      return (216);
    if (Link[k].Type != EN_PUMP)
      return (216);
    j = findpump(net, k);
  } else
    return (201);

  /* Find type of energy parameter */
  if (match(par->Tok[n - 2], w_PRICE)) /* Energy price */
  {
    if (!getfloat(par->Tok[n - 1], &y)) {
      if (j == 0)
        return (213);
      else
        return (217);
    }
    if (j == 0)
      hyd->Ecost = y;
    else
      Pump[j].Ecost = y;
    return (0);
  } else if (match(par->Tok[n - 2], w_PATTERN)) /* Price pattern */
  {
    t = findID(par->Tok[n - 1], par->Patlist); /* Check if pattern exists */
    if (t == NULL) {
      if (j == 0)
        return (213);
      else
        return (217);
    }
    if (j == 0)
      hyd->Epat = t->i;
    else
      Pump[j].Epat = t->i;
    return (0);
  } else if (match(par->Tok[n - 2], w_EFFIC)) /* Pump efficiency */
  {
    if (j == 0) {
      if (!getfloat(par->Tok[n - 1], &y))
        return (213);
      if (y <= 0.0)
        return (213);
      hyd->Epump = y;
    } else {
      t = findID(par->Tok[n - 1], par->Curvelist); /* Check if curve exists */
      if (t == NULL)
        return (217);
      Pump[j].Ecurve = t->i;
      net->Curve[t->i].Type = E_CURVE;
    }
    return (0);
  }
  return (201);
}

int reportdata(EN_Project *pr)
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

  EN_Network *net = &pr->network;
  report_options_t *rep = &pr->report;
  parser_data_t *par = &pr->parser;


  int i, j, n;
  double y;

  n = par->Ntokens - 1;
  if (n < 1)
    return (201);

  /* Value for page size */
  if (match(par->Tok[0], w_PAGE)) {
    if (!getfloat(par->Tok[n], &y))
      return (213);
    if (y < 0.0 || y > 255.0)
      return (213);
    rep->PageSize = (int)y;
    return (0);
  }

  /* Request that status reports be written */
  if (match(par->Tok[0], w_STATUS)) {
    if (match(par->Tok[n], w_NO))
      rep->Statflag = FALSE;
    if (match(par->Tok[n], w_YES))
      rep->Statflag = TRUE;
    if (match(par->Tok[n], w_FULL))
      rep->Statflag = FULL;
    return (0);
  }

  /* Request summary report */
  if (match(par->Tok[0], w_SUMMARY)) {
    if (match(par->Tok[n], w_NO))
      rep->Summaryflag = FALSE;
    if (match(par->Tok[n], w_YES))
      rep->Summaryflag = TRUE;
    return (0);
  }

  /* Request error/warning message reporting */
  if (match(par->Tok[0], w_MESSAGES)) {
    if (match(par->Tok[n], w_NO))
      rep->Messageflag = FALSE;
    if (match(par->Tok[n], w_YES))
      rep->Messageflag = TRUE;
    return (0);
  }

  /* Request an energy usage report */
  if (match(par->Tok[0], w_ENERGY)) {
    if (match(par->Tok[n], w_NO))
      rep->Energyflag = FALSE;
    if (match(par->Tok[n], w_YES))
      rep->Energyflag = TRUE;
    return (0);
  }

  /* Particular reporting nodes specified */
  if (match(par->Tok[0], w_NODE)) {
    if (match(par->Tok[n], w_NONE))
      rep->Nodeflag = 0; /* No nodes */
    else if (match(par->Tok[n], w_ALL))
      rep->Nodeflag = 1; /* All nodes */
    else {
      if (net->Nnodes == 0)
        return (208);
      for (i = 1; i <= n; i++) {
        if ((j = findnode(net,par->Tok[i])) == 0)
          return (208);
        net->Node[j].Rpt = 1;
      }
      rep->Nodeflag = 2;
    }
    return (0);
  }

  /* Particular reporting links specified */
  if (match(par->Tok[0], w_LINK)) {
    if (match(par->Tok[n], w_NONE))
      rep->Linkflag = 0;
    else if (match(par->Tok[n], w_ALL))
      rep->Linkflag = 1;
    else {
      if (net->Nlinks == 0)
        return (210);
      for (i = 1; i <= n; i++) {
        if ((j = findlink(net,par->Tok[i])) == 0)
          return (210);
        net->Link[j].Rpt = 1;
      }
      rep->Linkflag = 2;
    }
    return (0);
  }

  /* Check if input is a reporting criterion. */

  /*** Special case needed to distinguish "HEAD" from "HEADLOSS" ***/ //(2.00.11
                                                                      //- LR)
  if (strcomp(par->Tok[0], t_HEADLOSS))
    i = HEADLOSS;
  else
    i = findmatch(par->Tok[0], Fldname);
  if (i >= 0)
  /*****************************************************************/ //(2.00.11
                                                                      //- LR)
  {
    if (i > FRICTION)
      return (201);
    if (par->Ntokens == 1 || match(par->Tok[1], w_YES)) {
      rep->Field[i].Enabled = TRUE;
      return (0);
    }
    if (match(par->Tok[1], w_NO)) {
      rep->Field[i].Enabled = FALSE;
      return (0);
    }
    if (par->Ntokens < 3)
      return (201);
    if (match(par->Tok[1], w_BELOW))
      j = LOW; /* Get relation operator */
    else if (match(par->Tok[1], w_ABOVE))
      j = HI; /* or precision keyword  */
    else if (match(par->Tok[1], w_PRECISION))
      j = PREC;
    else
      return (201);
    if (!getfloat(par->Tok[2], &y))
      return (201);
    if (j == PREC) {
      rep->Field[i].Enabled = TRUE;
      rep->Field[i].Precision = ROUND(y);
    } else
      rep->Field[i].RptLim[j] = y; /* Report limit value */
    return (0);
  }

  /* Name of external report file */
  if (match(par->Tok[0], w_FILE)) {
    strncpy(rep->Rpt2Fname, par->Tok[1], MAXFNAME);
    return (0);
  }

  /* If get to here then return error condition */
  return (201);
} /* end of reportdata */

int timedata(EN_Project *pr)
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

  report_options_t *rep = &pr->report;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;
  time_options_t *time = &pr->time_options;


  int n;
  long t;
  double y;

  n = par->Ntokens - 1;
  if (n < 1)
    return (201);

  /* Check if setting time statistic flag */
  if (match(par->Tok[0], w_STATISTIC)) {
    if (match(par->Tok[n], w_NONE))
      rep->Tstatflag = SERIES;
    else if (match(par->Tok[n], w_NO))
      rep->Tstatflag = SERIES;
    else if (match(par->Tok[n], w_AVG))
      rep->Tstatflag = AVG;
    else if (match(par->Tok[n], w_MIN))
      rep->Tstatflag = MIN;
    else if (match(par->Tok[n], w_MAX))
      rep->Tstatflag = MAX;
    else if (match(par->Tok[n], w_RANGE))
      rep->Tstatflag = RANGE;
    else
      return (201);
    return (0);
  }

  /* Convert text time value to numerical value in seconds */
  /* Examples:
  **    5           = 5 * 3600 sec
  **    5 MINUTES   = 5 * 60   sec
  **    13:50       = 13*3600 + 50*60 sec
  **    1:50 pm     = (12+1)*3600 + 50*60 sec
  */

  if (!getfloat(par->Tok[n], &y)) {
    if ((y = hour(par->Tok[n], "")) < 0.0) {
      if ((y = hour(par->Tok[n - 1], par->Tok[n])) < 0.0)
        return (213);
    }
  }
  t = (long)(3600.0 * y + 0.5);
  /* Process the value assigned to the matched parameter */
  if (match(par->Tok[0], w_DURATION))
    time->Dur = t; /* Simulation duration */
  else if (match(par->Tok[0], w_HYDRAULIC))
    time->Hstep = t; /* Hydraulic time step */
  else if (match(par->Tok[0], w_QUALITY))
    qu->Qstep = t; /* Quality time step   */
  else if (match(par->Tok[0], w_RULE))
    time->Rulestep = t; /* Rule time step      */
  else if (match(par->Tok[0], w_MINIMUM))
    return (0); /* Not used anymore    */
  else if (match(par->Tok[0], w_PATTERN)) {
    if (match(par->Tok[1], w_TIME))
      time->Pstep = t; /* Pattern time step   */
    else if (match(par->Tok[1], w_START))
      time->Pstart = t; /* Pattern start time  */
    else
      return (201);
  } else if (match(par->Tok[0], w_REPORT)) {
    if (match(par->Tok[1], w_TIME))
      time->Rstep = t; /* Reporting time step  */
    else if (match(par->Tok[1], w_START))
      time->Rstart = t; /* Reporting start time */
    else
      return (201);
  } /* Simulation start time*/
  else if (match(par->Tok[0], w_START))
    time->Tstart = t % SECperDAY;
  else
    return (201);
  return (0);
} /* end of timedata */

int optiondata(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: processes [OPTIONS] data
**--------------------------------------------------------------
*/
{
  parser_data_t *par = &pr->parser;

  int i, n;

  n = par->Ntokens - 1;
  i = optionchoice(pr,n); /* Option is a named choice    */
  if (i >= 0)
    return (i);
  return (optionvalue(pr,n)); /* Option is a numerical value */
} /* end of optiondata */

int optionchoice(EN_Project *pr, int n)
/*
**--------------------------------------------------------------
**  Input:   n = index of last input token saved in par->Tok[]
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
**    DEMAND MODEL        DDA/PDA/PPA
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;
  out_file_t *out = &pr->out_files;
  int choice;

  /* Check if 1st token matches a parameter name and */
  /* process the input for the matched parameter     */
  if (n < 0) return (201);
  if (match(par->Tok[0], w_UNITS))
  {
    if (n < 1) return (0);
    else if (match(par->Tok[1], w_CFS))   par->Flowflag = CFS;
    else if (match(par->Tok[1], w_GPM))   par->Flowflag = GPM;
    else if (match(par->Tok[1], w_AFD))   par->Flowflag = AFD;
    else if (match(par->Tok[1], w_MGD))   par->Flowflag = MGD;
    else if (match(par->Tok[1], w_IMGD))  par->Flowflag = IMGD;
    else if (match(par->Tok[1], w_LPS))   par->Flowflag = LPS;
    else if (match(par->Tok[1], w_LPM))   par->Flowflag = LPM;
    else if (match(par->Tok[1], w_CMH))   par->Flowflag = CMH;
    else if (match(par->Tok[1], w_CMD))   par->Flowflag = CMD;
    else if (match(par->Tok[1], w_MLD))   par->Flowflag = MLD;
    else if (match(par->Tok[1], w_SI))    par->Flowflag = LPS;
    else return (201);
  }

  else if (match(par->Tok[0], w_PRESSURE))
  {
    if (n < 1) return (0);
    else if (match(par->Tok[1], w_EXPONENT)) return -1;
    else if (match(par->Tok[1], w_PSI))    par->Pressflag = PSI;
    else if (match(par->Tok[1], w_KPA))    par->Pressflag = KPA;
    else if (match(par->Tok[1], w_METERS)) par->Pressflag = METERS;
    else return (201);
  }

  else if (match(par->Tok[0], w_HEADLOSS))
  {
    if (n < 1)  return (0);
    else if (match(par->Tok[1], w_HW))   hyd->Formflag = HW;
    else if (match(par->Tok[1], w_DW))   hyd->Formflag = DW;
    else if (match(par->Tok[1], w_CM))   hyd->Formflag = CM;
    else return (201);
  }

  else if (match(par->Tok[0], w_HYDRAULIC))
  {
    if (n < 2) return (0);
    else if (match(par->Tok[1], w_USE))  out->Hydflag = USE;
    else if (match(par->Tok[1], w_SAVE)) out->Hydflag = SAVE;
    else return (201);
    strncpy(out->HydFname, par->Tok[2], MAXFNAME);
  }

  else if (match(par->Tok[0], w_QUALITY))
  {
    if (n < 1) return (0);
    else if (match(par->Tok[1], w_NONE))  qu->Qualflag = NONE;
    else if (match(par->Tok[1], w_CHEM))  qu->Qualflag = CHEM;
    else if (match(par->Tok[1], w_AGE))   qu->Qualflag = AGE;
    else if (match(par->Tok[1], w_TRACE)) qu->Qualflag = TRACE;
    else
    {
        qu->Qualflag = CHEM;
        strncpy(qu->ChemName, par->Tok[1], MAXID);
        if (n >= 2) strncpy(qu->ChemUnits, par->Tok[2], MAXID);
    }
    if (qu->Qualflag == TRACE) /* Source tracing option */
    {
      /* Copy Trace Node ID to par->Tok[0] for error reporting */
      strcpy(par->Tok[0], "");
      if (n < 2) return (212);
      strcpy(par->Tok[0], par->Tok[2]);
      qu->TraceNode = findnode(net,par->Tok[2]);
      if (qu->TraceNode == 0)  return (212);
      strncpy(qu->ChemName, u_PERCENT, MAXID);
      strncpy(qu->ChemUnits, par->Tok[2], MAXID);
    }
    if (qu->Qualflag == AGE)
    {
      strncpy(qu->ChemName, w_AGE, MAXID);
      strncpy(qu->ChemUnits, u_HOURS, MAXID);
    }
  }

  else if (match(par->Tok[0], w_MAP))
  {
    if (n < 1) return (0);
    strncpy(pr->MapFname, par->Tok[1], MAXFNAME); /* Map file name */
  }

  else if (match(par->Tok[0], w_VERIFY))
  {
    /* Backward compatibility for verification file */
  }

  else if (match(par->Tok[0], w_UNBALANCED)) /* Unbalanced option */
  {
    if (n < 1) return (0);
    if (match(par->Tok[1], w_STOP)) hyd->ExtraIter = -1;
    else if (match(par->Tok[1], w_CONTINUE))
    {
      if (n >= 2)  hyd->ExtraIter = atoi(par->Tok[2]);
      else         hyd->ExtraIter = 0;
    }
    else return (201);
  }

  else if (match(par->Tok[0], w_PATTERN)) /* Pattern option */
  {
    if (n < 1) return (0);
    strncpy(par->DefPatID, par->Tok[1], MAXID);
  }

  else if (match(par->Tok[0], w_DEMAND))
  {
      if (n < 2) return 0;
      if (!match(par->Tok[1], w_MODEL)) return -1;
      choice = findmatch(par->Tok[2], DemandModelTxt);
      if (choice < 0) return 201;
      hyd->DemandModel = choice;
  }
  else return (-1);
  return (0);
} /* end of optionchoice */

int optionvalue(EN_Project *pr, int n)
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

**    HEADLIMIT           value
**    FLOWLIMIT           value
**    MINIMUM PRESSURE    value
**    REQUIRED PRESSURE   value
**    PRESSURE EXPONENT   value

**    TOLERANCE           value
**    SEGMENTS            value  (not used)
**  ------ Undocumented Options -----
**    HTOL                value
**    QTOL                value
**    RQTOL               value
**    CHECKFREQ           value
**    MAXCHECK            value
**    DAMPLIMIT           value
**--------------------------------------------------------------
*/
{
  hydraulics_t *hyd = &pr->hydraulics;
  quality_t *qu = &pr->quality;
  parser_data_t *par = &pr->parser;
  char* tok0 = par->Tok[0];


  int nvalue = 1; /* Index of token with numerical value */
  double y;

  /* Check for obsolete SEGMENTS keyword */
  if (match(tok0, w_SEGMENTS)) return (0);

  /* Check for missing value (which is permissible) */
  if (match(tok0, w_SPECGRAV) || match(tok0, w_EMITTER) ||
      match(tok0, w_DEMAND)   || match(tok0, w_MINIMUM) ||
      match(tok0, w_REQUIRED) || match(tok0, w_PRESSURE) ||
      match(tok0, w_PRECISION))
  {
    nvalue = 2;
  }
  if (n < nvalue) return (0);

  /* Check for valid numerical input */
  if (!getfloat(par->Tok[nvalue], &y)) return (213);

  /* Check for WQ tolerance option (which can be 0) */
  if (match(tok0, w_TOLERANCE))
  {
    if (y < 0.0) return (213);
    qu->Ctol = y; /* Quality tolerance*/
    return (0);
  }

  /* Check for Diffusivity option */
  if (match(tok0, w_DIFFUSIVITY))
  {
    if (y < 0.0) return (213);
    qu->Diffus = y;
    return (0);
  }

  /* Check for Damping Limit option */
  if (match(tok0, w_DAMPLIMIT))
  {
    hyd->DampLimit = y;
    return (0);
  }

  /* Check for flow change limit*/
  else if (match(tok0, w_FLOWCHANGE))
  {
      if (y < 0.0) return 213;
      hyd->FlowChangeLimit = y;
      return 0;
  }

  /* Check for head error limit*/
  else if (match(tok0, w_HEADERROR))
  {
      if (y < 0.0) return 213;
      hyd->HeadErrorLimit = y;
      return 0;
  }

  /* Check for pressure dependent demand params */
  else if (match(tok0, w_MINIMUM))
  {
      if (y < 0.0) return 213;
      hyd->Pmin = y;
      return 0;
  }
  else if (match(tok0, w_REQUIRED))
  {
      if (y < 0.0) return 213;
      hyd->Preq = y;
      return 0;
  }
  else if (match(tok0, w_PRESSURE))
  {
      if (y < 0.0) return 213;
      hyd->Pexp = y;
      return 0;
  }

  /* All other options must be > 0 */
  if (y <= 0.0) return (213);

  /* Assign value to specified option */
  if (match(tok0, w_VISCOSITY))     hyd->Viscos = y; /* Viscosity */
  else if (match(tok0, w_SPECGRAV)) hyd->SpGrav = y; /* Spec. gravity */
  else if (match(tok0, w_TRIALS))   hyd->MaxIter = (int)y;  /* Max. trials */
  else if (match(tok0, w_ACCURACY)) /* Accuracy */
  {
    y = MAX(y, 1.e-5);
    y = MIN(y, 1.e-1);
    hyd->Hacc = y;
  }
  else if (match(tok0, w_HTOL))  hyd->Htol = y;
  else if (match(tok0, w_QTOL))  hyd->Qtol = y;
  else if (match(tok0, w_RQTOL))
  {
    if (y >= 1.0) return (213);
    hyd->RQtol = y;
  }
  else if (match(tok0, w_CHECKFREQ))  hyd->CheckFreq = (int)y;
  else if (match(tok0, w_MAXCHECK))   hyd->MaxCheck = (int)y;
  else if (match(tok0, w_EMITTER))    hyd->Qexp = 1.0 / y;
  else if (match(tok0, w_DEMAND))     hyd->Dmult = y;
  else return (201);
  return (0);
} /* end of optionvalue */

int getpumpcurve(EN_Project *pr, int n)
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
  EN_Network *net = &pr->network;
  parser_data_t *par = &pr->parser;

  double a, b, c, h0, h1, h2, q1, q2;

  Spump *pump = &net->Pump[net->Npumps];

  if (n == 1) /* Const. HP curve       */
  {
    if (par->X[0] <= 0.0)
      return (202);
    pump->Ptype = CONST_HP;
    net->Link[net->Nlinks].Km = par->X[0];
  } else {
    if (n == 2) /* Generic power curve   */
    {
      q1 = par->X[1];
      h1 = par->X[0];
      h0 = 1.33334 * h1;
      q2 = 2.0 * q1;
      h2 = 0.0;
    }
    else if (n >= 5) /* 3-pt. power curve     */
    {
      h0 = par->X[0];
      h1 = par->X[1];
      q1 = par->X[2];
      h2 = par->X[3];
      q2 = par->X[4];
    } else
      return (202);
    pump->Ptype = POWER_FUNC;
    if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c))
      return (206);
    pump->H0 = -a;
    pump->R = -b;
    pump->N = c;
    pump->Q0 = q1;
    pump->Qmax = pow((-a / b), (1.0 / c));
    pump->Hmax = h0;
  }
  return (0);
}

int powercurve(double h0, double h1, double h2, double q1, double q2,
               double *a, double *b, double *c)
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
  double h4, h5;
  if (h0 < TINY || h0 - h1 < TINY || h1 - h2 < TINY || q1 < TINY ||
      q2 - q1 < TINY)
    return (0);
  *a = h0;
  h4 = h0 - h1;
  h5 = h0 - h2;
  *c = log(h5 / h4) / log(q2 / q1);
  if (*c <= 0.0 || *c > 20.0)
    return (0);
  *b = -h4 / pow(q1, *c);

  /*** Updated 6/24/02 ***/
  if (*b >= 0.0)
    return (0);

  return (1);
}

int valvecheck(EN_Project *pr, int type, int j1, int j2)
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
  EN_Network *net = &pr->network;

  int k, vj1, vj2;
  EN_LinkType vtype;

  /* Examine each existing valve */
  for (k = 1; k <= net->Nvalves; k++) {
    Svalve *valve = &net->Valve[k];
    Slink *link = &net->Link[valve->Link];
    vj1 = link->N1;
    vj2 = link->N2;
    vtype = link->Type;

    /* Cannot have two PRVs sharing downstream nodes or in series */
    if (vtype == EN_PRV && type == EN_PRV) {
      if (vj2 == j2 || vj2 == j1 || vj1 == j2)
        return (0);
    }

    /* Cannot have two PSVs sharing upstream nodes or in series */
    if (vtype == EN_PSV && type == EN_PSV) {
      if (vj1 == j1 || vj1 == j2 || vj2 == j1)
        return (0);
    }

    /* Cannot have PSV connected to downstream node of PRV */
    if (vtype == EN_PSV && type == EN_PRV && vj1 == j2)
      return (0);
    if (vtype == EN_PRV && type == EN_PSV && vj2 == j1)
      return (0);

    /*** Updated 3/1/01 ***/
    /* Cannot have PSV connected to downstream node of FCV */
    /* nor have PRV connected to upstream node of FCV */
    if (vtype == EN_FCV && type == EN_PSV && vj2 == j1)
      return (0);
    if (vtype == EN_FCV && type == EN_PRV && vj1 == j2)
      return (0);

    /*** Updated 4/14/05 ***/
    if (vtype == EN_PSV && type == EN_FCV && vj1 == j2)
      return (0);
    if (vtype == EN_PRV && type == EN_FCV && vj2 == j1)
      return (0);
  }
  return (1);
} /* End of valvecheck */

void changestatus(EN_Network *net, int j, StatType status, double y)
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
  Slink *link = &net->Link[j];

  if (link->Type == EN_PIPE || link->Type == EN_GPV) {
    if (status != ACTIVE)
      link->Stat = status;
  } else if (link->Type == EN_PUMP) {
    if (status == ACTIVE) {
      link->Kc = y;
      status = OPEN;
      if (y == 0.0)
        status = CLOSED;
    } else if (status == OPEN) {
      link->Kc = 1.0;
    }
    link->Stat = status;
  } else if (link->Type >= EN_PRV) {
    link->Kc = y;
    link->Stat = status;
    if (status != ACTIVE) {
      link->Kc = MISSING;
    }
  }
} /* end of changestatus */

/********************** END OF INPUT3.C ************************/
