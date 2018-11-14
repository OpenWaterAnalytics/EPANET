/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       input3.c
Description:  parses network data from a line of an INP input file
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 11/10/2018
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

// Defined in ENUMSTXT.H
extern char *MixTxt[];
extern char *Fldname[];
extern char *DemandModelTxt[];

// Exported functions
int powercurve(double, double, double, double, double, double *, double *,
               double *);

// Imported Functions
extern int addnodeID(network_t *n, int, char *);
extern int addlinkID(network_t *n, int, char *);
extern STmplist *getlistitem(char *, STmplist *);

// Local functions
static int  optionchoice(EN_Project pr, int);
static int  optionvalue(EN_Project pr, int);
static int  getpumpcurve(EN_Project pr, int);
static void changestatus(network_t *net, int, StatType, double);

int juncdata(EN_Project pr)
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
    int p = 0;          // time pattern index
    int n;              // number of tokens
    int njuncs;         // number of network junction nodes
    double el,          // elevation
           y = 0.0;     // base demand
    Pdemand demand;     // demand record
    STmplist *patlist;  // list of demands

    network_t      *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    hydraulics_t  *hyd = &pr->hydraulics;
    Snode *node;
  
    // Add new junction to data base
    n = parser->Ntokens;
    if (net->Nnodes == parser->MaxNodes) return 200;
    net->Njuncs++;
    net->Nnodes++;
    njuncs = net->Njuncs;
    if (!addnodeID(net, net->Njuncs, parser->Tok[0])) return 215;

    // Check for valid data
    if (n < 2) return 201;
    if (!getfloat(parser->Tok[1], &el)) return 202;
    if (n >= 3 && !getfloat(parser->Tok[2], &y)) return 202;
    if (n >= 4)
    {
        patlist = getlistitem(parser->Tok[3], parser->Patlist);
        if (patlist == NULL) return 205;
        p = patlist->i;
    }

    // Save junction data
    node = &net->Node[njuncs];
    node->X = MISSING;
    node->Y = MISSING;
    node->El = el;
    node->C0 = 0.0;
    node->S = NULL;
    node->Ke = 0.0;
    node->Rpt = 0;
    node->Type = JUNCTION;
    strcpy(node->Comment, parser->Comment);

  
    // create a demand record, even if no demand is specified here.
    demand = (struct Sdemand *) malloc(sizeof(struct Sdemand));
    if (demand == NULL) return 101;
    demand->Base = y;
    demand->Pat = p;
    strncpy(demand->Name, "", MAXMSG);
    demand->next = NULL;
    node->D = demand;
    hyd->NodeDemand[njuncs] = y;
    return 0;
}

int tankdata(EN_Project pr)
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
      int    i,               // Node index
             n,               // # data items
             pattern = 0,     // Time pattern index
             curve = 0;       // Curve index
      double el = 0.0,        // Elevation
             initlevel = 0.0, // Initial level
             minlevel = 0.0,  // Minimum level
             maxlevel = 0.0,  // Maximum level
             minvol = 0.0,    // Minimum volume
             diam = 0.0,      // Diameter
             area;            // X-sect. area
    STmplist  *tmplist;

    network_t      *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Snode *node;
    Stank *tank;
  
    // Add new tank to data base
    n = parser->Ntokens;
    if (net->Ntanks == parser->MaxTanks ||
        net->Nnodes == parser->MaxNodes) return 200;
    net->Ntanks++;
    net->Nnodes++;

    i = parser->MaxJuncs + net->Ntanks;
    if (!addnodeID(net, i, parser->Tok[0])) return 215;

    // Check for valid data
    if (n < 2) return (201);
    if (!getfloat(parser->Tok[1], &el)) return 202;

    // Tank is reservoir
    if (n <= 3)
    {
        // Head pattern supplied
        if (n == 3)
        {
            tmplist = getlistitem(parser->Tok[2], parser->Patlist);
            if (tmplist == NULL) return 205;
            pattern = tmplist->i;
        }
    }
    else if (n < 6) return 201;
  
    // Tank is a storage tank
    else
    {
        if (!getfloat(parser->Tok[2], &initlevel)) return 202;
        if (!getfloat(parser->Tok[3], &minlevel))  return 202;
        if (!getfloat(parser->Tok[4], &maxlevel))  return 202;
        if (!getfloat(parser->Tok[5], &diam))      return 202;
        if (diam < 0.0) return 202;
        if (n >= 7 && !getfloat(parser->Tok[6], &minvol)) return 202;

        // If volume curve supplied check it exists
        if (n == 8)
        {
            tmplist = getlistitem(parser->Tok[7], parser->Curvelist);
            if (tmplist == NULL) return 202;
            curve = tmplist->i;
            net->Curve[curve].Type = V_CURVE;
        }
    }
    node = &net->Node[i];
    tank = &net->Tank[net->Ntanks];

    node->X = MISSING;
    node->Y = MISSING;
    node->Rpt = 0;
    node->El = el;
    node->C0 = 0.0;
    node->S = NULL;
    node->Ke = 0.0;
    node->Type = (diam == 0) ? RESERVOIR : TANK;
    strcpy(node->Comment, parser->Comment);
    tank->Node = i;
    tank->H0 = initlevel;
    tank->Hmin = minlevel;
    tank->Hmax = maxlevel;
    tank->A = diam;
    tank->Pat = pattern;
    tank->Kb = MISSING;
  
    //*******************************************************************
    // NOTE: The min, max, & initial volumes set here are based on a
    //    nominal tank diameter. They will be modified in INPUT1.C if
    //    a volume curve is supplied for this tank.
    //*******************************************************************
    area = PI * SQR(diam) / 4.0;
    tank->Vmin = area * minlevel;
    if (minvol > 0.0) tank->Vmin = minvol;
    tank->V0 = tank->Vmin + area * (initlevel - minlevel);
    tank->Vmax = tank->Vmin + area * (maxlevel - minlevel);

    tank->Vcurve = curve;
    tank->MixModel = MIX1; // Completely mixed
    tank->V1max = 1.0;     // Mixing compartment size fraction
    return 0;
}

int pipedata(EN_Project pr)
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
    int      j1,               // Start-node index
             j2,               // End-node index
             n;                // # data items
    LinkType type = PIPE;      // Link type
    StatType status = OPEN;    // Link status
    double   length,           // Pipe length
             diam,             // Pipe diameter
             rcoeff,           // Roughness coeff.
             lcoeff = 0.0;     // Minor loss coeff

    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Slink *link;
  
    // Add new pipe to data base
    n = parser->Ntokens;
    if (net->Nlinks == parser->MaxLinks) return 200;
    net->Npipes++;
    net->Nlinks++;
    if (!addlinkID(net, net->Nlinks, parser->Tok[0])) return 215;

    // Check for valid data
    if (n < 6) return 201;
    if ((j1 = findnode(net, parser->Tok[1])) == 0 ||
        (j2 = findnode(net, parser->Tok[2])) == 0) return 203;
    if (j1 == j2) return 222;

    if (!getfloat(parser->Tok[3], &length) ||
        !getfloat(parser->Tok[4], &diam) ||
        !getfloat(parser->Tok[5], &rcoeff)) return 202;
    if (length <= 0.0 || diam <= 0.0 || rcoeff <= 0.0) return 202;

    // Either a loss coeff. or a status is supplied
    if (n == 7)
    {
        if (match(parser->Tok[6], w_CV)) type = CVPIPE;
        else if (match(parser->Tok[6], w_CLOSED)) status = CLOSED;
        else if (match(parser->Tok[6], w_OPEN))   status = OPEN;
        else if (!getfloat(parser->Tok[6], &lcoeff)) return (202);
    }

    // Both a loss coeff. and a status is supplied
    if (n == 8)
    {
        if (!getfloat(parser->Tok[6], &lcoeff)) return 202;
        if (match(parser->Tok[7], w_CV))  type = CVPIPE;
        else if (match(parser->Tok[7], w_CLOSED)) status = CLOSED;
        else if (match(parser->Tok[7], w_OPEN))   status = OPEN;
        else return 202;
    }
    if (lcoeff < 0.0) return 202;

    // Save pipe data
    link = &net->Link[net->Nlinks];
    link->N1 = j1;
    link->N2 = j2;
    link->Len = length;
    link->Diam = diam;
    link->Kc = rcoeff;
    link->Km = lcoeff;
    link->Kb = MISSING;
    link->Kw = MISSING;
    link->Type = type;
    link->Stat = status;
    link->Rpt = 0;
    strcpy(link->Comment, parser->Comment);
    return 0;
}

int pumpdata(EN_Project pr)
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
    int    j, 
           j1,    // Start-node index
           j2,    // End-node index
           m, n;  // # data items 
    double y;
    STmplist *tmplist; // Temporary list

    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Slink *link;
    Spump *pump;

    /* Add new pump to data base */
    n = parser->Ntokens;
    if (net->Nlinks == parser->MaxLinks ||
        net->Npumps == parser->MaxPumps) return 200;
    net->Nlinks++;
    net->Npumps++;
    if (!addlinkID(net, net->Nlinks, parser->Tok[0])) return 215;

    // Check for valid data
    if (n < 4) return 201;
    if ((j1 = findnode(net, parser->Tok[1])) == 0 ||
        (j2 = findnode(net, parser->Tok[2])) == 0)  return 203;
    if (j1 == j2) return 222;

    // Save pump data
    link = &net->Link[net->Nlinks];
    pump = &net->Pump[net->Npumps];
  
    link->N1 = j1;
    link->N2 = j2;
    link->Diam = 0;
    link->Len = 0.0;
    link->Kc = 1.0;
    link->Km = 0.0;
    link->Kb = 0.0;
    link->Kw = 0.0;
    link->Type = PUMP;
    link->Stat = OPEN;
    link->Rpt = 0;
    strcpy(link->Comment, parser->Comment);
    pump->Link = net->Nlinks;
    pump->Ptype = NOCURVE; // NOCURVE is a placeholder
    pump->Hcurve = 0;
    pump->Ecurve = 0;
    pump->Upat = 0;
    pump->Ecost = 0.0;
    pump->Epat = 0;

    // If 4-th token is a number then input follows Version 1.x format
    // so retrieve pump curve parameters
    if (getfloat(parser->Tok[3], &parser->X[0]))
    {
        m = 1;
        for (j = 4; j < n; j++)
        {
            if (!getfloat(parser->Tok[j], &parser->X[m])) return 202;
            m++;
        }
        return (getpumpcurve(pr,m));
    }

    // Otherwise input follows Version 2 format
    // so retrieve keyword/value pairs
    m = 4;
    while (m < n)
    {
        if (match(parser->Tok[m - 1], w_POWER)) // Const. HP curve
        {
            y = atof(parser->Tok[m]);
            if (y <= 0.0) return (202);
            pump->Ptype = CONST_HP;
            link->Km = y;
        } 
        else if (match(parser->Tok[m - 1], w_HEAD))  // Custom pump curve
        {
            tmplist = getlistitem(parser->Tok[m], parser->Curvelist);
            if (tmplist == NULL) return 206;
            pump->Hcurve = tmplist->i;
        }    
        else if (match(parser->Tok[m - 1], w_PATTERN))  // Speed/status pattern
        {
            tmplist = getlistitem(parser->Tok[m], parser->Patlist);
            if (tmplist == NULL) return 205;
            pump->Upat = tmplist->i;
        } 
        else if (match(parser->Tok[m - 1], w_SPEED))   // Speed setting
        {
            if (!getfloat(parser->Tok[m], &y)) return 202;
            if (y < 0.0) return 202;
            link->Kc = y;
        } 
        else return 201;
        m = m + 2;  // Move to next keyword token
    }
    return 0;
}

int valvedata(EN_Project pr)
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
    int j1,                    // Start-node index
        j2,                    // End-node index
        n;                     // # data items
    char  status = ACTIVE,     // Valve status
          type;                // Valve type
    double diam = 0.0,         // Valve diameter
           setting,            // Valve setting
           lcoeff = 0.0;       // Minor loss coeff.
    STmplist *tmplist;         // Temporary list

    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Slink *link;
  
    // Add new valve to data base
    n = parser->Ntokens;
    if (net->Nlinks == parser->MaxLinks ||
        net->Nvalves == parser->MaxValves) return 200;
    net->Nvalves++;
    net->Nlinks++;
    if (!addlinkID(net, net->Nlinks, parser->Tok[0])) return 215;

    // Check for valid data
    if (n < 6) return 201;
    if ((j1 = findnode(net, parser->Tok[1])) == 0 ||
        (j2 = findnode(net, parser->Tok[2])) == 0) return (203);
    if (j1 == j2) return 222;

    if (match(parser->Tok[4], w_PRV))       type = PRV;
    else if (match(parser->Tok[4], w_PSV))  type = PSV;
    else if (match(parser->Tok[4], w_PBV))  type = PBV;
    else if (match(parser->Tok[4], w_FCV))  type = FCV;
    else if (match(parser->Tok[4], w_TCV))  type = TCV;
    else if (match(parser->Tok[4], w_GPV))  type = GPV;
    else return 201;

    if (!getfloat(parser->Tok[3], &diam)) return 202;
    if (diam <= 0.0) return 202;

    // Find headloss curve for GPV 
    if (type == GPV)
    { 
        tmplist = getlistitem(parser->Tok[5], parser->Curvelist);
        if (tmplist == NULL) return 206;
        setting = tmplist->i;
        net->Curve[tmplist->i].Type = H_CURVE;
        status = OPEN;
    }
    else if (!getfloat(parser->Tok[5], &setting)) return 202;
    if (n >= 7 && !getfloat(parser->Tok[6], &lcoeff)) return 202;

    // Check that PRV, PSV, or FCV not connected to a tank & 
    // check for illegal connections between pairs of valves
    if ((j1 > net->Njuncs || j2 > net->Njuncs) &&
        (type == PRV || type == PSV || type == FCV)) return 219;
    if (!valvecheck(pr, type, j1, j2)) return 220;

    // Save valve data
    link = &net->Link[net->Nlinks];
    link->N1 = j1;
    link->N2 = j2;
    link->Diam = diam;
    link->Len = 0.0;
    link->Kc = setting;
    link->Km = lcoeff;
    link->Kb = 0.0;
    link->Kw = 0.0;
    link->Type = type; 
    link->Stat = status;
    link->Rpt = 0;
    strcpy(link->Comment, parser->Comment);
    net->Valve[net->Nvalves].Link = net->Nlinks;
    return 0;
}

int patterndata(EN_Project pr)
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
    int i, n;
    double x;
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    SFloatlist *f;
    STmplist *p;

    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Check for a new pattern
    if (parser->PrevPat != NULL &&
        strcmp(parser->Tok[0], parser->PrevPat->ID) == 0) p = parser->PrevPat;
    else p = getlistitem(parser->Tok[0], parser->Patlist);
    if (p == NULL) return 205;

    // Add parsed multipliers to the pattern
    for (i = 1; i <= n; i++)
    {
        if (!getfloat(parser->Tok[i], &x)) return 202;
        f = (SFloatlist *)malloc(sizeof(SFloatlist));
        if (f == NULL) return 101;
        f->value = x;
        f->next = p->x;
        p->x = f;
    }

    // Save # multipliers for pattern
    net->Pattern[p->i].Length += n;

    // Set previous pattern pointer
    parser->PrevPat = p;
    return (0);
}

int curvedata(EN_Project pr)
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
    double x, y;
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    SFloatlist *fx, *fy;
    STmplist *c;

    // Check for valid curve ID
    if (parser->Ntokens < 3) return 201;
    if (parser->PrevCurve != NULL &&
        strcmp(parser->Tok[0], parser->PrevCurve->ID) == 0) c = parser->PrevCurve;
    else c = getlistitem(parser->Tok[0], parser->Curvelist);
    if (c == NULL) return 205;

    // Check for valid data
    if (!getfloat(parser->Tok[1], &x)) return 202;
    if (!getfloat(parser->Tok[2], &y)) return 202;

    // Add new data point to curve
    fx = (SFloatlist *)malloc(sizeof(SFloatlist));
    if (fx == NULL) return 101;
    fy = (SFloatlist *)malloc(sizeof(SFloatlist));
    if (fy == NULL)
    {
        free(fx);
        return 101;
    }
    fx->value = x;
    fx->next = c->x;
    c->x = fx;
    fy->value = y;
    fy->next = c->y;
    c->y = fy;
    net->Curve[c->i].Npts++;

    // Save the pointer to this curve
    parser->PrevCurve = c;
    return 0;
}

int coordata(EN_Project pr)
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
    int j;
    double x, y;
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Snode *node;
  
    // Check for valid node ID
    if (parser->Ntokens < 3) return 201;
    if ((j = findnode(net, parser->Tok[0])) == 0) return 203;
  
    // Check for valid data
    if (!getfloat(parser->Tok[1], &x)) return 202;
    if (!getfloat(parser->Tok[2], &y)) return 202;

    // Save coord data
    node = &net->Node[j];
    node->X = x;
    node->Y = y;
    return 0;
}

int demanddata(EN_Project pr)
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
    int j, n, p = 0;
    double y;
    network_t     *net = &pr->network;
    hydraulics_t  *hyd = &pr->hydraulics;
    parser_data_t *parser = &pr->parser;
    Pdemand demand;
    Pdemand cur_demand;
    STmplist *patlist;

    // Extract data from tokens
    n = parser->Ntokens;
    if (n < 2) return 201;
    if (!getfloat(parser->Tok[1], &y)) return 202;

    // If MULTIPLY command, save multiplier
    if (match(parser->Tok[0], w_MULTIPLY))
    {
        if (y <= 0.0) return 202;
        else hyd->Dmult = y;
        return 0;
    }

    // Otherwise find node (and pattern) being referenced
    if ((j = findnode(net, parser->Tok[0])) == 0) return 208;
    if (j > net->Njuncs) return 208;
    if (n >= 3)
    {
        patlist = getlistitem(parser->Tok[2], parser->Patlist);
        if (patlist == NULL) return 205;
        p = patlist->i;
    }

    // Replace any demand entered in [JUNCTIONS] section
    demand = net->Node[j].D;
    if (hyd->NodeDemand[j] != MISSING)
    {
        // First category encountered will overwrite "dummy" demand category
        // with what is specified in this section
        demand->Base = y;
        demand->Pat = p;
        strncpy(demand->Name, parser->Comment, MAXMSG);
        hyd->NodeDemand[j] = MISSING; // marker - next iteration will append a new category.
    }

    // Otherwise add new demand to junction
    else
    {
        cur_demand = net->Node[j].D;
        while (cur_demand->next != NULL) cur_demand = cur_demand->next;
        demand = (struct Sdemand *)malloc(sizeof(struct Sdemand));
        if (demand == NULL) return 101;
        demand->Base = y;
        demand->Pat = p;
        strncpy(demand->Name, parser->Comment, MAXMSG);
        demand->next = NULL;
        cur_demand->next = demand;
    }
    return 0;
}

int controldata(EN_Project pr)
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
    int          i = 0,                // Node index
                 k,                    // Link index
                 n;                    // # data items
    StatType     status = ACTIVE;      // Link status
    ControlType  ctltype;              // Control type
    LinkType     linktype;             // Link type
    double       setting = MISSING,    // Link setting
                 time = 0.0,           // Simulation time
                 level = 0.0;          // Pressure or tank level
    Scontrol     *control;
  
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
  
    // Check for sufficient number of input tokens
    n = parser->Ntokens;
    if (n < 6) return 201;

    // Check that controlled link exists
    k = findlink(net, parser->Tok[1]);
    if (k == 0) return 204;

    // Cannot control a check valve
    linktype = net->Link[k].Type;
    if (linktype == CVPIPE)  return 207;

    // Parse control setting into a status level or numerical setting
    if (match(parser->Tok[2], w_OPEN))
    {
        status = OPEN;
        if (linktype == PUMP) setting = 1.0;
        if (linktype == GPV)  setting = net->Link[k].Kc;
    }
    else if (match(parser->Tok[2], w_CLOSED))
    {
        status = CLOSED;
        if (linktype == PUMP) setting = 0.0;
        if (linktype == GPV)  setting = net->Link[k].Kc;
    }
    else if (linktype == GPV) return 206;
    else if (!getfloat(parser->Tok[2], &setting)) return 202;

    // Set status for pump in case speed setting was supplied
    // or for pipe if numerical setting was supplied
    if (linktype == PUMP || linktype == PIPE)
    {
        if (setting != MISSING)
        {
            if (setting < 0.0)       return 202;
            else if (setting == 0.0) status = CLOSED;
            else                     status = OPEN;
        }
    }

    // Determine type of control
    if (match(parser->Tok[4], w_TIME))           ctltype = TIMER;
    else if (match(parser->Tok[4], w_CLOCKTIME)) ctltype = TIMEOFDAY;
    else
    {
        if (n < 8) return 201;
        if ((i = findnode(net, parser->Tok[5])) == 0) return 203;
        if (match(parser->Tok[6], w_BELOW))      ctltype = LOWLEVEL;
        else if (match(parser->Tok[6], w_ABOVE)) ctltype = HILEVEL;
        else return 201;
  }

    // Parse control level or time
    switch (ctltype)
    {
        case TIMER:
        case TIMEOFDAY:
          if (n == 6) time = hour(parser->Tok[5], "");
          if (n == 7) time = hour(parser->Tok[5], parser->Tok[6]);
          if (time < 0.0) return 201;
          break;
        case LOWLEVEL:
        case HILEVEL:
          if (!getfloat(parser->Tok[7], &level)) return 202;
          break;
    }

    // Fill in fields of control data structure
    net->Ncontrols++;
    if (net->Ncontrols > parser->MaxControls) return 200;
    control = &net->Control[net->Ncontrols];
    control->Link = k;
    control->Node = i;
    control->Type = ctltype;
    control->Status = status;
    control->Setting = setting;
    control->Time = (long)(3600.0 * time);
    if (ctltype == TIMEOFDAY) control->Time %= SECperDAY;
    control->Grade = level;
    return 0;
}

int sourcedata(EN_Project pr)
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
    int i,              // Token with quality value
        j,              // Node index
        n,              // # data items
        p = 0;          // Time pattern index
    char type = CONCEN; // Source type
    double c0 = 0;      // Initial quality
    STmplist *patlist;
    Psource source;

    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
  
    // Check for enough tokens & that source node exists
    n = parser->Ntokens;
    if (n < 2) return 201;
    if ((j = findnode(net, parser->Tok[0])) == 0) return 203;

    // Parse source type
    // NOTE: Under old 1.1 format, SourceType not supplied so
    //       let i = index of token that contains quality value
    i = 2;
    if (match(parser->Tok[1], w_CONCEN))         type = CONCEN;
    else if (match(parser->Tok[1], w_MASS))      type = MASS;
    else if (match(parser->Tok[1], w_SETPOINT))  type = SETPOINT;
    else if (match(parser->Tok[1], w_FLOWPACED)) type = FLOWPACED;
    else i = 1;

    // Parse source quality
    if (!getfloat(parser->Tok[i], &c0)) return 202;

    // Parse optional source time pattern
    if (n > i + 1 && strlen(parser->Tok[i + 1]) > 0 &&
        strcmp(parser->Tok[i + 1], "*") != 0) 
    {
        patlist = getlistitem(parser->Tok[i + 1], parser->Patlist);
        if (patlist == NULL) return (205);
        p = patlist->i;
    }

    // Destroy any existing source assigned to node
    if (net->Node[j].S != NULL) free(net->Node[j].S);
    
    // Create a new source & assign it to the node
    source = (struct Ssource *)malloc(sizeof(struct Ssource));
    if (source == NULL) return 101;
    source->C0 = c0;
    source->Pat = p;
    source->Type = type;
    net->Node[j].S = source;
    return 0;
}

int emitterdata(EN_Project pr)
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
    int j,       // Node index
        n;       // # data items
    double k;    // Flow coeff.

    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
  
    // Check that node exists & is a junction
    n = parser->Ntokens;
    if (n < 2) return 201;
    if ((j = findnode(net, parser->Tok[0])) == 0) return 203;
    if (j > net->Njuncs) return 209;

    // Parse emitter flow coeff.
    if (!getfloat(parser->Tok[1], &k)) return 202;
    if (k < 0.0) return 202;
    net->Node[j].Ke = k;
    return 0;
}

int qualdata(EN_Project pr)
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
    int j, n;
    long i, i1, i2;
    double c0;
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Snode *Node = net->Node;
  
    if (net->Nnodes == 0) return 208;  // No nodes defined yet
    n = parser->Ntokens;
    if (n < 2) return 0;

    // Single node name supplied
    if (n == 2)
    {
        if ((j = findnode(net,parser->Tok[0])) == 0) return 0;
        if (!getfloat(parser->Tok[1], &c0)) return 209;
        Node[j].C0 = c0;
    }

    // Range of node names supplied
    else
    {
        // Parse quality value
        if (!getfloat(parser->Tok[2], &c0)) return 209;

        // If numerical node names supplied, then use numerical comparison
        // to find which nodes are assigned the quality value
        if ((i1 = atol(parser->Tok[0])) > 0 &&
            (i2 = atol(parser->Tok[1])) > 0)
        {
            for (j = 1; j <= net->Nnodes; j++)
            {
                i = atol(Node[j].ID);
                if (i >= i1 && i <= i2) Node[j].C0 = c0;
            }
        }
        
        // Otherwise use lexicographic comparison
        else
        {
            for (j = 1; j <= net->Nnodes; j++)
            {
                if ((strcmp(parser->Tok[0], Node[j].ID) <= 0) &&
                    (strcmp(parser->Tok[1], Node[j].ID) >= 0)
                   ) Node[j].C0 = c0;
            }
        }
    }
    return 0;
}

int reactdata(EN_Project pr)
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
    int item, j, n;
    long i, i1, i2;
    double y;

    network_t     *net = &pr->network;
    quality_t     *qual = &pr->quality;
    parser_data_t *parser = &pr->parser;
  
    // Skip line if insufficient data 
    n = parser->Ntokens;
    if (n < 3) return 0;

    // Keyword is ORDER
    if (match(parser->Tok[0], w_ORDER))
    {
        if (!getfloat(parser->Tok[n - 1], &y))  return 213;
        if (match(parser->Tok[1], w_BULK))      qual->BulkOrder = y;
        else if (match(parser->Tok[1], w_TANK)) qual->TankOrder = y;
        else if (match(parser->Tok[1], w_WALL))
        {
            if (y == 0.0)      qual->WallOrder = 0.0;
            else if (y == 1.0) qual->WallOrder = 1.0;
            else return 213;
        }
        else return 213;
        return 0;
    }

    // Keyword is ROUGHNESS
    if (match(parser->Tok[0], w_ROUGHNESS))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return 213;
        qual->Rfactor = y;
        return 0;
    }

    // Keyword is LIMITING
    if (match(parser->Tok[0], w_LIMITING))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return 213;
        qual->Climit = y;
        return 0;
    }
 
    // Keyword is GLOBAL
    if (match(parser->Tok[0], w_GLOBAL))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return 213;
        if (match(parser->Tok[1], w_BULK))      qual->Kbulk = y;
        else if (match(parser->Tok[1], w_WALL)) qual->Kwall = y;
        else return 201;
        return 0;
    }

    // Keyword is BULK, WALL or TANK
    if (match(parser->Tok[0], w_BULK))      item = 1;
    else if (match(parser->Tok[0], w_WALL)) item = 2;
    else if (match(parser->Tok[0], w_TANK)) item = 3;
    else return 201;

    // Save the first link/node ID in the first token
    strcpy(parser->Tok[0], parser->Tok[1]);

    // Case where tank rate coeffs. are being set
    if (item == 3)
    {
        // Get the rate coeff. value
        if (!getfloat(parser->Tok[n - 1], &y)) return 209;

        // Case where just a single tank is specified
        if (n == 3)
        {
            if ((j = findnode(net,parser->Tok[1])) <= net->Njuncs) return 0;
            net->Tank[j - net->Njuncs].Kb = y;
        }
        
        // Case where a numerical range of tank IDs is specified
        else if ((i1 = atol(parser->Tok[1])) > 0 &&
                 (i2 = atol(parser->Tok[2])) > 0)
        {
            for (j = net->Njuncs + 1; j <= net->Nnodes; j++)
            {
                i = atol(net->Node[j].ID);
                if (i >= i1 && i <= i2) net->Tank[j - net->Njuncs].Kb = y;
            }
        }

        // Case where a general range of tank IDs is specified
        else for (j = net->Njuncs + 1; j <= net->Nnodes; j++)
        {
            if ((strcmp(parser->Tok[1], net->Node[j].ID) <= 0) &&
                (strcmp(parser->Tok[2], net->Node[j].ID) >= 0)
                ) net->Tank[j - net->Njuncs].Kb = y;
        }
    }

    // Case where pipe rate coeffs. are being set
    else
    {
        // Get the rate coeff. value
        if (!getfloat(parser->Tok[n - 1], &y)) return 211; /* Rate coeff. */
        if (net->Nlinks == 0) return 0;

        // Case where just a single link is specified
        if (n == 3)
        {
            if ((j = findlink(net, parser->Tok[1])) == 0) return 0;
            if (item == 1) net->Link[j].Kb = y;
            else           net->Link[j].Kw = y;
        }
        
        // Case where a numerical range of link IDs is specified
        else if ((i1 = atol(parser->Tok[1])) > 0 &&
                 (i2 = atol(parser->Tok[2])) > 0)
        {
            for (j = 1; j <= net->Nlinks; j++)
            {
                i = atol(net->Link[j].ID);
                if (i >= i1 && i <= i2) 
                {
                    if (item == 1)  net->Link[j].Kb = y;
                    else            net->Link[j].Kw = y;
                }
            }
        }
                
        // Case where a general range of link IDs is specified
        else for (j = 1; j <= net->Nlinks; j++)
        {
            if ((strcmp(parser->Tok[1], net->Link[j].ID) <= 0) &&
                (strcmp(parser->Tok[2], net->Link[j].ID) >= 0))
            {
                if (item == 1) net->Link[j].Kb = y;
                else           net->Link[j].Kw = y;
            }
        }
    }
    return 0;
}

int mixingdata(EN_Project pr)
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
    int    i,     // Tank index
           j,     // Node index
           m,     // Type of mixing model
           n;     // Number of data items
    double v;     // Mixing zone volume fraction
  
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;

    // Check for valid data
    if (net->Nnodes == 0) return 208;
    n = parser->Ntokens;
    if (n < 2) return 0;
    if ((j = findnode(net, parser->Tok[0])) <= net->Njuncs) return 0;
    if ((m = findmatch(parser->Tok[1], MixTxt)) < 0) return 201;

    // Find mixing zone volume fraction (which can't be 0)
    v = 1.0;
    if ((m == MIX2) && (n == 3) && (!getfloat(parser->Tok[2], &v))) return 209;
    if (v == 0.0) v = 1.0;

    // Assign mixing data to tank (return if tank is a reservoir)
    i = j - net->Njuncs;
    if (net->Tank[i].A == 0.0) return 0;
    net->Tank[i].MixModel = (char)m;
    net->Tank[i].V1max = v;
    return 0;
}

int statusdata(EN_Project pr)
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
    int j, n;
    long i, i1, i2;
    double y = 0.0;
    char status = ACTIVE;
  
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
  
    if (net->Nlinks == 0) return 210;
    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Check for legal status setting
    if (match(parser->Tok[n], w_OPEN))  status = OPEN;
    else if (match(parser->Tok[n], w_CLOSED)) status = CLOSED;
    else if (!getfloat(parser->Tok[n], &y)) return 211;
    if (y < 0.0) return 211;

    // A single link ID was supplied
    if (n == 1)
    {
        if ((j = findlink(net, parser->Tok[0])) == 0) return 0;
    
        // Cannot change status of a Check Valve
        if (net->Link[j].Type == CVPIPE) return 211;

        // Cannot change setting for a GPV
        if (net->Link[j].Type == GPV && status == ACTIVE) return 211;
        changestatus(net, j, status, y);
    }

    // A range of numerical link ID's was supplied
    else if ((i1 = atol(parser->Tok[0])) > 0 &&
             (i2 = atol(parser->Tok[1])) > 0)
    {
        for (j = 1; j <= net->Nlinks; j++)
        {
            i = atol(net->Link[j].ID);
            if (i >= i1 && i <= i2) changestatus(net, j, status, y);
        }
    }
    
    // A range of general link ID's was supplied
    else for (j = 1; j <= net->Nlinks; j++)
    {
        if ((strcmp(parser->Tok[0], net->Link[j].ID) <= 0) &&
            (strcmp(parser->Tok[1], net->Link[j].ID) >= 0)
           ) changestatus(net, j, status, y);
    }
    return 0;
}

int energydata(EN_Project pr)
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
    int j, k, n;
    double y;

    network_t     *net = &pr->network;
    hydraulics_t  *hyd = &pr->hydraulics;
    parser_data_t *parser = &pr->parser;
  
    STmplist *listitem;
    Slink *Link = net->Link;
    Spump *Pump = net->Pump;
  
    // Check for sufficient data
    n = parser->Ntokens;
    if (n < 3) return 201;

    // First keyword is DEMAND
    if (match(parser->Tok[0], w_DMNDCHARGE))
    {
        if (!getfloat(parser->Tok[2], &y)) return 213;
        hyd->Dcost = y;
        return 0;
    }

    // First keyword is GLOBAL (remaining data refer to global options)
    if (match(parser->Tok[0], w_GLOBAL))
    {
        j = 0;
    }
    
    // First keyword is PUMP (remaining data refer to a specific pump)
    else if (match(parser->Tok[0], w_PUMP))
    {
        if (n < 4) return 201;
        k = findlink(net,parser->Tok[1]);
        if (k == 0) return 216;
        if (Link[k].Type != PUMP) return 216;
        j = findpump(net, k);
    }
    else return 201;

    // PRICE parameter being set
    if (match(parser->Tok[n - 2], w_PRICE))
    {
        if (!getfloat(parser->Tok[n - 1], &y))
        {
            if (j == 0) return 213;
            else        return 217;
        }
        if (j == 0) hyd->Ecost = y;
        else        Pump[j].Ecost = y;
        return 0;
    }
    
    // Price PATTERN being set
    else if (match(parser->Tok[n - 2], w_PATTERN))
    {
        listitem = getlistitem(parser->Tok[n - 1], parser->Patlist);
        if (listitem == NULL)
        {
            if (j == 0) return 213;
            else        return 217;
        }
        if (j == 0) hyd->Epat = listitem->i;
        else        Pump[j].Epat = listitem->i;
        return 0;
    }
    
    // Pump EFFIC being set
    else if (match(parser->Tok[n - 2], w_EFFIC))
    {
        if (j == 0)
        {
            if (!getfloat(parser->Tok[n - 1], &y)) return 213;
            if (y <= 0.0) return 213;
            hyd->Epump = y;
        }
        else
        {
            listitem = getlistitem(parser->Tok[n - 1], parser->Curvelist);
            if (listitem == NULL) return 217;
            Pump[j].Ecurve = listitem->i;
            net->Curve[listitem->i].Type = E_CURVE;
        }
        return 0;
    }
    return 201;
}

int reportdata(EN_Project pr)
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
    int i, j, n;
    double y;
  
    network_t        *net = &pr->network;
    report_options_t *rpt = &pr->report;
    parser_data_t    *parser = &pr->parser;
    
    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Value for page size
    if (match(parser->Tok[0], w_PAGE))
    {
        if (!getfloat(parser->Tok[n], &y)) return 213;
        if (y < 0.0 || y > 255.0) return 213;
        rpt->PageSize = (int)y;
        return 0;
    }

    // Request that status reports be written
    if (match(parser->Tok[0], w_STATUS))
    {
        if (match(parser->Tok[n], w_NO))   rpt->Statflag = FALSE;
        if (match(parser->Tok[n], w_YES))  rpt->Statflag = TRUE;
        if (match(parser->Tok[n], w_FULL)) rpt->Statflag = FULL;
        return 0;
    }

    // Request summary report
    if (match(parser->Tok[0], w_SUMMARY))
    {
        if (match(parser->Tok[n], w_NO)) rpt->Summaryflag = FALSE;
        if (match(parser->Tok[n], w_YES))   rpt->Summaryflag = TRUE;
        return 0;
    }

    // Request error/warning message reporting
    if (match(parser->Tok[0], w_MESSAGES))
    {
        if (match(parser->Tok[n], w_NO))  rpt->Messageflag = FALSE;
        if (match(parser->Tok[n], w_YES)) rpt->Messageflag = TRUE;
        return 0;
    }

    // Request an energy usage report
    if (match(parser->Tok[0], w_ENERGY))
    {
        if (match(parser->Tok[n], w_NO))  rpt->Energyflag = FALSE;
        if (match(parser->Tok[n], w_YES)) rpt->Energyflag = TRUE;
        return 0;
    }

    // Particular reporting nodes specified
    if (match(parser->Tok[0], w_NODE))
    {
        if (match(parser->Tok[n], w_NONE))     rpt->Nodeflag = 0; // No nodes
        else if (match(parser->Tok[n], w_ALL)) rpt->Nodeflag = 1; // All nodes
        else
        {
            if (net->Nnodes == 0) return 208;
            for (i = 1; i <= n; i++)
            {
                if ((j = findnode(net, parser->Tok[i])) == 0) return 208;
                net->Node[j].Rpt = 1;
            }
            rpt->Nodeflag = 2;
        }
        return 0;
    }

    // Particular reporting links specified
    if (match(parser->Tok[0], w_LINK))
    {
        if (match(parser->Tok[n], w_NONE))     rpt->Linkflag = 0;
        else if (match(parser->Tok[n], w_ALL)) rpt->Linkflag = 1;
        else
        {
            if (net->Nlinks == 0) return 210;
            for (i = 1; i <= n; i++)
            {
                if ((j = findlink(net, parser->Tok[i])) == 0) return 210;
                net->Link[j].Rpt = 1;
            }
            rpt->Linkflag = 2;
        }
        return 0;
    }

    // Report fields specified
    // Special case needed to distinguish "HEAD" from "HEADLOSS"
    if (strcomp(parser->Tok[0], t_HEADLOSS)) i = HEADLOSS;
    else i = findmatch(parser->Tok[0], Fldname); 
    if (i >= 0)                            
    {
        if (i > FRICTION) return 201;
        if (parser->Ntokens == 1 || match(parser->Tok[1], w_YES))
        {
            rpt->Field[i].Enabled = TRUE;
            return 0;
        }

        if (match(parser->Tok[1], w_NO))
        {
            rpt->Field[i].Enabled = FALSE;
            return 0;
        }

        // Get field qualifier type
        if (parser->Ntokens < 3) return (201);
        if      (match(parser->Tok[1], w_BELOW))     j = LOW;
        else if (match(parser->Tok[1], w_ABOVE))     j = HI;
        else if (match(parser->Tok[1], w_PRECISION)) j = PREC;
        else return 201;

        // Get field qualifier value
        if (!getfloat(parser->Tok[2], &y)) return 201;
        if (j == PREC)
        {
            rpt->Field[i].Enabled = TRUE;
            rpt->Field[i].Precision = ROUND(y);
        }
        else rpt->Field[i].RptLim[j] = y;
        return (0);
    }

    // Name of external report file
    if (match(parser->Tok[0], w_FILE))
    {
        strncpy(rpt->Rpt2Fname, parser->Tok[1], MAXFNAME);
        return 0;
    }

    // If get to here then return error condition 
    return 201;
}

int timedata(EN_Project pr)
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
    int n;
    long t;
    double y;

    report_options_t *rpt = &pr->report;
    quality_t        *qual = &pr->quality;
    parser_data_t    *parser = &pr->parser;
    time_options_t   *time = &pr->time_options;

    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Check if setting report time statistic flag
    if (match(parser->Tok[0], w_STATISTIC))
    {
        if      (match(parser->Tok[n], w_NONE))  rpt->Tstatflag = SERIES;
        else if (match(parser->Tok[n], w_NO))    rpt->Tstatflag = SERIES;
        else if (match(parser->Tok[n], w_AVG))   rpt->Tstatflag = AVG;
        else if (match(parser->Tok[n], w_MIN))   rpt->Tstatflag = MIN;
        else if (match(parser->Tok[n], w_MAX))   rpt->Tstatflag = MAX;
        else if (match(parser->Tok[n], w_RANGE)) rpt->Tstatflag = RANGE;
        else return 201;
        return 0;
    }

    // Convert text time value to numerical value in seconds
    // Examples:
    //    5           = 5 * 3600 sec
    //    5 MINUTES   = 5 * 60   sec
    //    13:50       = 13*3600 + 50*60 sec
    //    1:50 pm     = (12+1)*3600 + 50*60 sec

    if (!getfloat(parser->Tok[n], &y))
    {
        if ((y = hour(parser->Tok[n], "")) < 0.0)
        {
            if ((y = hour(parser->Tok[n - 1], parser->Tok[n])) < 0.0) return 213;
        }
    }
    t = (long)(3600.0 * y + 0.5);

    /// Process the value assigned to the matched parameter
    if      (match(parser->Tok[0], w_DURATION))  time->Dur = t;
    else if (match(parser->Tok[0], w_HYDRAULIC)) time->Hstep = t;
    else if (match(parser->Tok[0], w_QUALITY) )  qual->Qstep = t;
    else if (match(parser->Tok[0], w_RULE))      time->Rulestep = t;
    else if (match(parser->Tok[0], w_MINIMUM))   return 0; // Not used anymore
    else if (match(parser->Tok[0], w_PATTERN))
    {
        if      (match(parser->Tok[1], w_TIME))  time->Pstep = t;
        else if (match(parser->Tok[1], w_START)) time->Pstart = t;
        else return 201;
    }
    else if (match(parser->Tok[0], w_REPORT))
    {
        if      (match(parser->Tok[1], w_TIME))  time->Rstep = t;
        else if (match(parser->Tok[1], w_START)) time->Rstart = t;
        else return 201;
    }
    else if (match(parser->Tok[0], w_START)) time->Tstart = t % SECperDAY;
    else return 201;
    return 0;
}

int optiondata(EN_Project pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: processes [OPTIONS] data
**--------------------------------------------------------------
*/
{
    int i, n;
    parser_data_t *parser = &pr->parser;

    // Option is a named choice
    n = parser->Ntokens - 1;
    i = optionchoice(pr, n);
    if (i >= 0) return i;

    // Option is a numerical value
    return (optionvalue(pr, n));
}

int optionchoice(EN_Project pr, int n)
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
    int choice;
    network_t     *net = &pr->network;
    hydraulics_t  *hyd = &pr->hydraulics;
    quality_t     *qual = &pr->quality;
    parser_data_t *parser = &pr->parser;
    out_file_t    *out = &pr->out_files;
  
    // Check if 1st token matches a parameter name and 
    // process the input for the matched parameter
    if (n < 0) return 201;

    // Flow UNITS
    if (match(parser->Tok[0], w_UNITS))
    {
        if (n < 1) return 0;
        else if (match(parser->Tok[1], w_CFS))  parser->Flowflag = CFS;
        else if (match(parser->Tok[1], w_GPM))  parser->Flowflag = GPM;
        else if (match(parser->Tok[1], w_AFD))  parser->Flowflag = AFD;
        else if (match(parser->Tok[1], w_MGD))  parser->Flowflag = MGD;
        else if (match(parser->Tok[1], w_IMGD)) parser->Flowflag = IMGD;
        else if (match(parser->Tok[1], w_LPS))  parser->Flowflag = LPS;
        else if (match(parser->Tok[1], w_LPM))  parser->Flowflag = LPM;
        else if (match(parser->Tok[1], w_CMH))  parser->Flowflag = CMH;
        else if (match(parser->Tok[1], w_CMD))  parser->Flowflag = CMD;
        else if (match(parser->Tok[1], w_MLD))  parser->Flowflag = MLD;
        else if (match(parser->Tok[1], w_SI))   parser->Flowflag = LPS;
        else return 201;
    }

    // PRESSURE units
    else if (match(parser->Tok[0], w_PRESSURE))
    {
        if (n < 1) return 0;
        else if (match(parser->Tok[1], w_EXPONENT)) return -1;
        else if (match(parser->Tok[1], w_PSI))    parser->Pressflag = PSI;
        else if (match(parser->Tok[1], w_KPA))    parser->Pressflag = KPA;
        else if (match(parser->Tok[1], w_METERS)) parser->Pressflag = METERS;
        else return 201;
    }
  
    // HEADLOSS formula
    else if (match(parser->Tok[0], w_HEADLOSS))
    {
        if (n < 1)  return 0;
        else if (match(parser->Tok[1], w_HW))   hyd->Formflag = HW;
        else if (match(parser->Tok[1], w_DW))   hyd->Formflag = DW;
        else if (match(parser->Tok[1], w_CM))   hyd->Formflag = CM;
        else return 201;
    }
  
    // HYDRUALICS USE/SAVE file option
    else if (match(parser->Tok[0], w_HYDRAULIC))
    {
        if (n < 2) return 0;
        else if (match(parser->Tok[1], w_USE))  out->Hydflag = USE;
        else if (match(parser->Tok[1], w_SAVE)) out->Hydflag = SAVE;
        else return 201;
        strncpy(out->HydFname, parser->Tok[2], MAXFNAME);
    }

    // Water QUALITY option
    else if (match(parser->Tok[0], w_QUALITY))
    {
        if (n < 1) return (0);
        else if (match(parser->Tok[1], w_NONE))  qual->Qualflag = NONE;
        else if (match(parser->Tok[1], w_CHEM))  qual->Qualflag = CHEM;
        else if (match(parser->Tok[1], w_AGE))   qual->Qualflag = AGE;
        else if (match(parser->Tok[1], w_TRACE)) qual->Qualflag = TRACE;
        else
        {
            qual->Qualflag = CHEM;
            strncpy(qual->ChemName, parser->Tok[1], MAXID);
            if (n >= 2) strncpy(qual->ChemUnits, parser->Tok[2], MAXID);
        }
        if (qual->Qualflag == TRACE)
        {
            // Copy Trace Node ID to parser->Tok[0] for error reporting
            strcpy(parser->Tok[0], "");
            if (n < 2) return 212;
            strcpy(parser->Tok[0], parser->Tok[2]);
            qual->TraceNode = findnode(net, parser->Tok[2]);
            if (qual->TraceNode == 0) return 212;
            strncpy(qual->ChemUnits, u_PERCENT, MAXID);
            strncpy(qual->ChemName, parser->Tok[2], MAXID);
        }
        if (qual->Qualflag == AGE)
        {
            strncpy(qual->ChemName, w_AGE, MAXID);
            strncpy(qual->ChemUnits, u_HOURS, MAXID);
        }
    }
  
    // MAP file name
    else if (match(parser->Tok[0], w_MAP))
    {
        if (n < 1) return 0;
        strncpy(pr->MapFname, parser->Tok[1], MAXFNAME);
    }
  
    else if (match(parser->Tok[0], w_VERIFY))
    {
        // Deprecated
    }
  
    // Hydraulics UNBALANCED option
    else if (match(parser->Tok[0], w_UNBALANCED))
    {
        if (n < 1) return 0;
        if (match(parser->Tok[1], w_STOP)) hyd->ExtraIter = -1;
        else if (match(parser->Tok[1], w_CONTINUE))
        {
            if (n >= 2)  hyd->ExtraIter = atoi(parser->Tok[2]);
            else         hyd->ExtraIter = 0;
        }
        else return 201;
    }
  
    // Default demand PATTERN
    else if (match(parser->Tok[0], w_PATTERN))
    {
        if (n < 1) return 0;
        strncpy(parser->DefPatID, parser->Tok[1], MAXID);
    }

    // DEMAND model
    else if (match(parser->Tok[0], w_DEMAND))
    {
        if (n < 2) return 0;
        if (!match(parser->Tok[1], w_MODEL)) return -1;
        choice = findmatch(parser->Tok[2], DemandModelTxt);
        if (choice < 0) return 201;
        hyd->DemandModel = choice;
    }

    // Return -1 if keyword did not match any option
    else return -1;
    return 0;
}

int optionvalue(EN_Project pr, int n)
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
    int    nvalue = 1;   // Index of token with numerical value
    double y;

    hydraulics_t  *hyd = &pr->hydraulics;
    quality_t     *qual = &pr->quality;
    parser_data_t *parser = &pr->parser;
    char* tok0 = parser->Tok[0];

    // Check for deprecated SEGMENTS keyword
    if (match(tok0, w_SEGMENTS)) return 0;

    // Check for missing value (which is permissible)
    if (match(tok0, w_SPECGRAV) || match(tok0, w_EMITTER) ||
        match(tok0, w_DEMAND)   || match(tok0, w_MINIMUM) ||
        match(tok0, w_REQUIRED) || match(tok0, w_PRESSURE) ||
        match(tok0, w_PRECISION)
       ) nvalue = 2;
    if (n < nvalue) return 0;

    // Check for valid numerical input
    if (!getfloat(parser->Tok[nvalue], &y)) return 213;

    // Quality tolerance option (which can be 0)
    if (match(tok0, w_TOLERANCE))
    {
        if (y < 0.0) return 213;
        qual->Ctol = y;
        return 0;
    }

    // Diffusivity
    if (match(tok0, w_DIFFUSIVITY))
    { 
        if (y < 0.0) return 213;
        qual->Diffus = y;
        return 0;
    }

    // Hydraulic damping limit option */ 
    if (match(tok0, w_DAMPLIMIT))
    {
        hyd->DampLimit = y;
        return 0;
    }
  
    // Flow change limit
    else if (match(tok0, w_FLOWCHANGE))
    {
          if (y < 0.0) return 213;
          hyd->FlowChangeLimit = y;
          return 0;
    }

    // Head loss error limit
    else if (match(tok0, w_HEADERROR))
    {
        if (y < 0.0) return 213;
        hyd->HeadErrorLimit = y;
        return 0;
    }

    // Pressure dependent demand parameters
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

    // All other options must be > 0
    if (y <= 0.0) return 213;

    // Assign value to all other options
    if (match(tok0, w_VISCOSITY))     hyd->Viscos = y;
    else if (match(tok0, w_SPECGRAV)) hyd->SpGrav = y;
    else if (match(tok0, w_TRIALS))   hyd->MaxIter = (int)y;
    else if (match(tok0, w_ACCURACY))
    {
        y = MAX(y, 1.e-5);
        y = MIN(y, 1.e-1);
        hyd->Hacc = y;
    }
    else if (match(tok0, w_HTOL))  hyd->Htol = y;
    else if (match(tok0, w_QTOL))  hyd->Qtol = y;
    else if (match(tok0, w_RQTOL))
    {
        if (y >= 1.0) return 213;
        hyd->RQtol = y;
    }
    else if (match(tok0, w_CHECKFREQ)) hyd->CheckFreq = (int)y;
    else if (match(tok0, w_MAXCHECK))  hyd->MaxCheck = (int)y;
    else if (match(tok0, w_EMITTER))   hyd->Qexp = 1.0 / y;
    else if (match(tok0, w_DEMAND))    hyd->Dmult = y;
    else return 201;
    return 0;
}

int getpumpcurve(EN_Project pr, int n)
/*
**--------------------------------------------------------
**  Input:   n = number of parameters for pump curve
**  Output:  returns error code
**  Purpose: processes pump curve data for Version 1.1-
**           style input data
**  Notes:
**    1. Called by pumpdata() in INPUT3.C
**    2. Current link index & pump index of pump being
**       processed is found in network variables Nlinks
**       and Npumps, respectively
**    3. Curve data read from input line is found in
**       parser's array X[0],...X[n-1]
**---------------------------------------------------------
*/
{
    double a, b, c, h0, h1, h2, q1, q2;
    network_t     *net = &pr->network;
    parser_data_t *parser = &pr->parser;
    Spump *pump = &net->Pump[net->Npumps];
  
    // Constant HP curve
    if (n == 1)
    {
        if (parser->X[0] <= 0.0) return 202;
        pump->Ptype = CONST_HP;
        net->Link[net->Nlinks].Km = parser->X[0];
    }
    
    // Power function curve
    else
    {
        // Single point power curve
        if (n == 2)
        {
            q1 = parser->X[1];
            h1 = parser->X[0];
            h0 = 1.33334 * h1;
            q2 = 2.0 * q1;
            h2 = 0.0;
        }

        // 3-point power curve
        else if (n >= 5)
        {
            h0 = parser->X[0];
            h1 = parser->X[1];
            q1 = parser->X[2];
            h2 = parser->X[3];
            q2 = parser->X[4];
        }
        else return 202;
        pump->Ptype = POWER_FUNC;
        if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c)) return 206;
        pump->H0 = -a;
        pump->R = -b;
        pump->N = c;
        pump->Q0 = q1;
        pump->Qmax = pow((-a / b), (1.0 / c));
        pump->Hmax = h0;
    }
    return 0;
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

    if (h0 < TINY || h0 - h1 < TINY || h1 - h2 < TINY ||
        q1 < TINY || q2 - q1 < TINY
       ) return 0;
    *a = h0;
    h4 = h0 - h1;
    h5 = h0 - h2;
    *c = log(h5 / h4) / log(q2 / q1);
    if (*c <= 0.0 || *c > 20.0) return 0;
    *b = -h4 / pow(q1, *c);
    if (*b >= 0.0) return 0;
    return 1;
}

int valvecheck(EN_Project pr, int type, int j1, int j2)
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
    int k, vj1, vj2;
    LinkType vtype;
    network_t *net = &pr->network;
    Slink *link;
    Svalve *valve;

    // Examine each existing valve
    for (k = 1; k <= net->Nvalves; k++)
    {
        valve = &net->Valve[k];
        link = &net->Link[valve->Link];
        vj1 = link->N1;
        vj2 = link->N2;
        vtype = link->Type;

        // Cannot have two PRVs sharing downstream nodes or in series
        if (vtype == PRV && type == PRV)
        {
            if (vj2 == j2 || vj2 == j1 || vj1 == j2) return 0;
        }

        // Cannot have two PSVs sharing upstream nodes or in series
        if (vtype == PSV && type == PSV)
        {
            if (vj1 == j1 || vj1 == j2 || vj2 == j1) return 0;
        }

        // Cannot have PSV connected to downstream node of PRV
        if (vtype == PSV && type == PRV && vj1 == j2) return 0;
        if (vtype == PRV && type == PSV && vj2 == j1) return 0;

        // Cannot have PSV connected to downstream node of FCV
        // nor have PRV connected to upstream node of FCV
        if (vtype == FCV && type == PSV && vj2 == j1) return 0;
        if (vtype == FCV && type == PRV && vj1 == j2) return 0;
        if (vtype == PSV && type == FCV && vj1 == j2) return 0;
        if (vtype == PRV && type == FCV && vj2 == j1) return 0;
    }
    return 1;
}

void changestatus(network_t *net, int j, StatType status, double y)
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
  
    if (link->Type == PIPE || link->Type == GPV)
    {
        if (status != ACTIVE) link->Stat = status;
    }
    else if (link->Type == PUMP)
    {
        if (status == ACTIVE)
        {
            link->Kc = y;
            status = OPEN;
            if (y == 0.0) status = CLOSED;
        }
        else if (status == OPEN) link->Kc = 1.0;
        link->Stat = status;
    }
    else if (link->Type >= PRV)
    {
        link->Kc = y;
        link->Stat = status;
        if (status != ACTIVE) link->Kc = MISSING;
    }
}
