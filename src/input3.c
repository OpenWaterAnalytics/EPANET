/*
******************************************************************************
Project:      OWA EPANET
Version:      2.2
Module:       input3.c
Description:  parses network data from a line of an EPANET input file
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 11/29/2019
******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
extern int addnodeID(Network *, int, char *);
extern int addlinkID(Network *, int, char *);

// Local functions
static int  optionchoice(Project *, int);
static int  optionvalue(Project *, int);
static int  getpumpcurve(Project *, int);
static void changestatus(Network *, int, StatusType, double);
static int  setError(Parser *, int, int);


int setError(Parser *parser, int tokindex, int errcode)
/*
**--------------------------------------------------------------
**  Input:   tokindex = index of input line token
**           errcode = an error code
**  Output:  returns error code
**  Purpose: records index of token from line of input associated
**           with an error
**--------------------------------------------------------------
*/
{
    parser->ErrTok = tokindex;
    return errcode;
}

int juncdata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;
    Hydraul *hyd = &pr->hydraul;

    int p = 0;                  // time pattern index
    int n;                      // number of tokens
    int njuncs;                 // number of network junction nodes
    double el,                  // elevation
           y = 0.0;             // base demand
    Snode *node;
    int err = 0;

    // Add new junction to data base
    n = parser->Ntokens;
    if (net->Nnodes == parser->MaxNodes) return 200;
    net->Njuncs++;
    net->Nnodes++;
    njuncs = net->Njuncs;
    err = addnodeID(net, net->Njuncs, parser->Tok[0]);
    if (err) return setError(parser, 0, err);

    // Check for valid data
    if (n < 2) return 201;
    if (!getfloat(parser->Tok[1], &el)) return setError(parser, 1, 202);
    if (n >= 3 && !getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);
    if (n >= 4)
    {
        p = findpattern(net, parser->Tok[3]);
        if (p < 0) return setError(parser, 3, 205);
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
    node->ResultIndex = 0;
    node->Type = JUNCTION;
    node->Comment = xstrcpy(&node->Comment, parser->Comment, MAXMSG);

    // Create a demand for the junction and use NodeDemand as an indicator
    // to be used when processing demands from the [DEMANDS] section
    if (!adddemand(node, y, p, NULL)) return 101;
    hyd->NodeDemand[njuncs] = y;
    return 0;
}

int tankdata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int    i,               // Node index
           n,               // # data items
           pattern = 0,     // Time pattern index
           curve = 0,       // Curve index
           overflow = FALSE;// Overflow indicator

    double el = 0.0,        // Elevation
           initlevel = 0.0, // Initial level
           minlevel = 0.0,  // Minimum level
           maxlevel = 0.0,  // Maximum level
           minvol = 0.0,    // Minimum volume
           diam = 0.0,      // Diameter
           area;            // X-sect. area
    Snode *node;
    Stank *tank;

    int err = 0;

    // Add new tank to data base
    n = parser->Ntokens;
    if (net->Ntanks == parser->MaxTanks ||
        net->Nnodes == parser->MaxNodes) return 200;
    net->Ntanks++;
    net->Nnodes++;

    i = parser->MaxJuncs + net->Ntanks;
    err = addnodeID(net, i, parser->Tok[0]);
    if (err) return setError(parser, 0, err);

    // Check for valid data
    if (n < 2) return 201;
    if (!getfloat(parser->Tok[1], &el)) return setError(parser, 1, 202);

    // Tank is reservoir
    if (n <= 3)
    {
        // Head pattern supplied
        if (n == 3)
        {
            pattern = findpattern(net, parser->Tok[2]);
            if (pattern < 0) return setError(parser, 2, 205);
        }
    }
    else if (n < 6) return 201;

    // Tank is a storage tank
    else
    {
        if (!getfloat(parser->Tok[2], &initlevel)) return setError(parser, 2, 202);
        if (!getfloat(parser->Tok[3], &minlevel))  return setError(parser, 3, 202);
        if (!getfloat(parser->Tok[4], &maxlevel))  return setError(parser, 4, 202);
        if (!getfloat(parser->Tok[5], &diam))      return setError(parser, 5, 202);
        if (n >= 7 && !getfloat(parser->Tok[6], &minvol)) return setError(parser, 6, 202);

        // If volume curve supplied check it exists
        if (n >= 8)
        {
            if (strlen(parser->Tok[7]) > 0 && *(parser->Tok[7]) != '*')
            {
                curve = findcurve(net, parser->Tok[7]);
                if (curve == 0) return setError(parser, 7, 206);
                net->Curve[curve].Type = VOLUME_CURVE;
            }
        }

        // Parse overflow indicator if present
        if (n >= 9)
        {
            if (match(parser->Tok[8], w_YES)) overflow = TRUE;
            else if (match(parser->Tok[8], w_NO)) overflow = FALSE;
            else return setError(parser, 8, 213);
        }

        if (initlevel < 0.0) return setError(parser, 2, 209);
        if (minlevel  < 0.0) return setError(parser, 3, 209);
        if (maxlevel  < 0.0) return setError(parser, 4, 209);
        if (diam      < 0.0) return setError(parser, 5, 209);
        if (minvol    < 0.0) return setError(parser, 6, 209);
    }
    node = &net->Node[i];
    tank = &net->Tank[net->Ntanks];

    node->X = MISSING;
    node->Y = MISSING;
    node->Rpt = 0;
    node->ResultIndex = 0;
    node->El = el;
    node->C0 = 0.0;
    node->S = NULL;
    node->Ke = 0.0;
    node->Type = (diam == 0) ? RESERVOIR : TANK;
    node->Comment = xstrcpy(&node->Comment, parser->Comment, MAXMSG);
    tank->Node = i;
    tank->H0 = initlevel;
    tank->Hmin = minlevel;
    tank->Hmax = maxlevel;
    tank->A = diam;
    tank->Pat = pattern;
    tank->Kb = MISSING;
    tank->CanOverflow = overflow;

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

int pipedata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int      j1,               // Start-node index
             j2,               // End-node index
             n;                // # data items
    double   length,           // Pipe length
             diam,             // Pipe diameter
             rcoeff,           // Roughness coeff.
             lcoeff = 0.0;     // Minor loss coeff
    LinkType type = PIPE;      // Link type
    StatusType status = OPEN;  // Link status
    Slink *link;
    int err = 0;

    // Add new pipe to data base
    n = parser->Ntokens;
    if (net->Nlinks == parser->MaxLinks) return 200;
    net->Npipes++;
    net->Nlinks++;
    err = addlinkID(net, net->Nlinks, parser->Tok[0]);
    if (err) return setError(parser, 0, err);

    // Check for valid data
    if (n < 6) return 201;
    if ((j1 = findnode(net, parser->Tok[1])) == 0) return setError(parser, 1, 203);
    if ((j2 = findnode(net, parser->Tok[2])) == 0) return setError(parser, 2, 203);
    if (j1 == j2) return setError(parser, 0, 222);

    if (!getfloat(parser->Tok[3], &length)) return setError(parser, 3, 202);
    if (length <= 0.0) return setError(parser, 3, 211);
    if (!getfloat(parser->Tok[4], &diam)) return  setError(parser, 4, 202);
    if (diam <= 0.0) return setError(parser, 4, 211);
    if (!getfloat(parser->Tok[5], &rcoeff)) return setError(parser, 5, 202);
    if (rcoeff <= 0.0) setError(parser, 5, 211);

    // Either a loss coeff. or a status is supplied
    if (n == 7)
    {
        if (match(parser->Tok[6], w_CV)) type = CVPIPE;
        else if (match(parser->Tok[6], w_CLOSED)) status = CLOSED;
        else if (match(parser->Tok[6], w_OPEN))   status = OPEN;
        else if (!getfloat(parser->Tok[6], &lcoeff)) return setError(parser, 6, 202);
    }

    // Both a loss coeff. and a status is supplied
    if (n == 8)
    {
        if (!getfloat(parser->Tok[6], &lcoeff)) return setError(parser, 6, 202);
        if (match(parser->Tok[7], w_CV))  type = CVPIPE;
        else if (match(parser->Tok[7], w_CLOSED)) status = CLOSED;
        else if (match(parser->Tok[7], w_OPEN))   status = OPEN;
        else return setError(parser, 7, 213);
    }
    if (lcoeff < 0.0) return setError(parser, 6, 211);

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
    link->Status = status;
    link->Rpt = 0;
    link->ResultIndex = 0;
    link->Comment = xstrcpy(&link->Comment, parser->Comment, MAXMSG);
    return 0;
}

int pumpdata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int    j, m,  // Token array indexes
           j1,    // Start-node index
           j2,    // End-node index
           n,     // # data items
           c, p;  // Curve & Pattern indexes
    double y;
    Slink *link;
    Spump *pump;
    int err = 0;

    /* Add new pump to data base */
    n = parser->Ntokens;
    if (net->Nlinks == parser->MaxLinks ||
        net->Npumps == parser->MaxPumps) return 200;
    net->Nlinks++;
    net->Npumps++;
    err = addlinkID(net, net->Nlinks, parser->Tok[0]);
    if (err) return setError(parser, 0, err);

    // Check for valid data
    if (n < 3) return 201;
    if ((j1 = findnode(net, parser->Tok[1])) == 0) return setError(parser, 1, 203);
    if ((j2 = findnode(net, parser->Tok[2])) == 0) return setError(parser, 2, 203);
    if (j1 == j2) return setError(parser, 0, 222);

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
    link->Status = OPEN;
    link->Rpt = 0;
    link->ResultIndex = 0;
    link->Comment = xstrcpy(&link->Comment, parser->Comment, MAXMSG);
    pump->Link = net->Nlinks;
    pump->Ptype = NOCURVE; // NOCURVE is a placeholder
    pump->Hcurve = 0;
    pump->Ecurve = 0;
    pump->Upat = 0;
    pump->Ecost = 0.0;
    pump->Epat = 0;
    if (n < 4) return 0;

    // If 4-th token is a number then input follows Version 1.x format
    // so retrieve pump curve parameters
    if (getfloat(parser->Tok[3], &parser->X[0]))
    {
        m = 1;
        for (j = 4; j < n; j++)
        {
            if (!getfloat(parser->Tok[j], &parser->X[m])) return setError(parser, j, 202);
            m++;
        }
        return (getpumpcurve(pr, m));
    }

    // Otherwise input follows Version 2 format
    // so retrieve keyword/value pairs
    m = 4;
    while (m < n)
    {
        if (match(parser->Tok[m - 1], w_POWER)) // Const. HP curve
        {
            y = atof(parser->Tok[m]);
            if (y <= 0.0) return setError(parser, m, 202);
            pump->Ptype = CONST_HP;
            link->Km = y;
        }
        else if (match(parser->Tok[m - 1], w_HEAD))  // Custom pump curve
        {
            c = findcurve(net, parser->Tok[m]);
            if (c == 0) return setError(parser, m, 206);
            pump->Hcurve = c;
        }
        else if (match(parser->Tok[m - 1], w_PATTERN))  // Speed/status pattern
        {
            p = findpattern(net, parser->Tok[m]);
            if (p < 0) return setError(parser, m, 205);
            pump->Upat = p;
        }
        else if (match(parser->Tok[m - 1], w_SPEED))   // Speed setting
        {
            if (!getfloat(parser->Tok[m], &y)) return setError(parser, m, 202);
            if (y < 0.0) return setError(parser, m, 211);
            link->Kc = y;
        }
        else return 201;
        m = m + 2;  // Move to next keyword token
    }
    return 0;
}

int valvedata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int c,                     // Curve index
        j1,                    // Start-node index
        j2,                    // End-node index
        n;                     // # data items
    char  status = ACTIVE,     // Valve status
          type;                // Valve type
    double diam = 0.0,         // Valve diameter
           setting,            // Valve setting
           lcoeff = 0.0;       // Minor loss coeff.
    Slink *link;
    int err = 0;

    // Add new valve to data base
    n = parser->Ntokens;
    if (net->Nlinks == parser->MaxLinks ||
        net->Nvalves == parser->MaxValves) return 200;
    net->Nvalves++;
    net->Nlinks++;
    err = addlinkID(net, net->Nlinks, parser->Tok[0]);
    if (err) return setError(parser, 0, err);

    // Check for valid data
    if (n < 6)
      return 201;
    if ((j1 = findnode(net, parser->Tok[1])) == 0)
      return setError(parser, 1, 203);
    if ((j2 = findnode(net, parser->Tok[2])) == 0)
      return setError(parser, 2, 203);
    if (j1 == j2)
      return setError(parser, 0, 222);

    if (match(parser->Tok[4], w_PRV))
      type = PRV;
    else if (match(parser->Tok[4], w_PSV))
      type = PSV;
    else if (match(parser->Tok[4], w_PBV))
      type = PBV;
    else if (match(parser->Tok[4], w_FCV))
      type = FCV;
    else if (match(parser->Tok[4], w_TCV))
      type = TCV;
    else if (match(parser->Tok[4], w_GPV))
      type = GPV;
    else
      return setError(parser, 4, 213);

    if (!getfloat(parser->Tok[3], &diam)) return setError(parser, 3, 202);
    if (diam <= 0.0) return setError(parser, 3, 211);

    // Find headloss curve for GPV
    if (type == GPV)
    {
        c = findcurve(net, parser->Tok[5]);
        if (c == 0) return setError(parser, 5, 206);
        setting = c;
        net->Curve[c].Type = HLOSS_CURVE;
        status = OPEN;
    }
    else if (!getfloat(parser->Tok[5], &setting)) return setError(parser, 5, 202);
    if (n >= 7 && !getfloat(parser->Tok[6], &lcoeff)) return setError(parser, 6, 202);

    // Check for illegal connections
    if (valvecheck(pr, net->Nlinks, type, j1, j2))
    {
        if      (j1 > net->Njuncs) return setError(parser, 1, 219);
        else if (j2 > net->Njuncs) return setError(parser, 2, 219);
        else                       return setError(parser, -1, 220);
    }

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
    link->Status = status;
    link->Rpt = 0;
    link->ResultIndex = 0;
    link->Comment = xstrcpy(&link->Comment, parser->Comment, MAXMSG);
    net->Valve[net->Nvalves].Link = net->Nlinks;
    return 0;
}

int patterndata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int i, j, n, n1;
    double x;
    Spattern *pattern;

    // "n" is the number of pattern factors contained in the line
    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Check if previous input line was for the same pattern
    if (parser->PrevPat && strcmp(parser->Tok[0], parser->PrevPat->ID) == 0)
    {
        pattern = parser->PrevPat;
    }

    // Otherwise retrieve pattern from the network's Pattern array
    else
    {
        i = findpattern(net, parser->Tok[0]);
        if (i <= 0) return setError(parser, 0, 205);
        pattern = &(net->Pattern[i]);
        if (pattern->Comment == NULL && parser->Comment[0])
        {
            pattern->Comment = xstrcpy(&pattern->Comment, parser->Comment, MAXMSG);
        }
    }

    // Expand size of the pattern's factors array
    n1 = pattern->Length;
    pattern->Length += n;
    pattern->F = realloc(pattern->F, pattern->Length * sizeof(double));

    // Add parsed multipliers to the pattern
    for (j = 1; j <= n; j++)
    {
        if (!getfloat(parser->Tok[j], &x)) return setError(parser, j, 202);
        pattern->F[n1 + j - 1] = x;
    }

    // Save a reference to this pattern for processing additional pattern data
    parser->PrevPat = pattern;
    return 0;
}

int curvedata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int i;
    double x, y;
    Scurve *curve;

    // Check for valid data
    if (parser->Ntokens < 3) return 201;
    if (!getfloat(parser->Tok[1], &x)) return setError(parser, 1, 202);
    if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);

    // Check if previous input line was for the same curve
    if (parser->PrevCurve && strcmp(parser->Tok[0], parser->PrevCurve->ID) == 0)
    {
        curve = parser->PrevCurve;
    }

    // Otherwise retrieve curve from the network's Curve array
    else
    {
        i = findcurve(net, parser->Tok[0]);
        if (i == 0) return setError(parser, 0, 206);
        curve = &(net->Curve[i]);
        if (curve->Comment == NULL && parser->Comment[0])
        {
            curve->Comment = xstrcpy(&curve->Comment, parser->Comment, MAXMSG);
        }
    }

    // Expand size of data arrays if need be
    if (curve->Capacity == curve->Npts)
    {
        if (resizecurve(curve, curve->Capacity + 10) > 0) return 101;
    }

    // Add new data point to curve
    curve->X[curve->Npts] = x;
    curve->Y[curve->Npts] = y;
    curve->Npts++;

    // Save a reference to this curve for processing additional curve data
    parser->PrevCurve = curve;
    return 0;
}

int coordata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int j;
    double x, y;
    Snode *node;

    // Check for valid node ID
    if (parser->Ntokens < 3) return 201;
    if ((j = findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);

    // Check for valid data
    if (!getfloat(parser->Tok[1], &x)) return setError(parser, 1, 202);
    if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);

    // Save coord data
    node = &net->Node[j];
    node->X = x;
    node->Y = y;
    return 0;
}

int vertexdata(Project *pr)
/*
 **--------------------------------------------------------------
 **  Input:   none
 **  Output:  returns error code
 **  Purpose: processes link vertex data
 **  Format:
 **    [VERTICES]
 **      id  x  y
 **--------------------------------------------------------------
 */
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int j;
    double x, y;
    
    // Check for valid link ID
    if (parser->Ntokens < 3) return 201;
    if ((j = findlink(net, parser->Tok[0])) == 0) return setError(parser, 0, 204);

    // Check for valid coordinate data
    if (!getfloat(parser->Tok[1], &x)) return setError(parser, 1, 202);
    if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);

    // Add to link's list of vertex points
    return addlinkvertex(&net->Link[j], x, y);
}


int demanddata(Project *pr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Parser  *parser = &pr->parser;

    int j, n, p = 0;
    double y;

    Pdemand demand;

    // Extract data from tokens
    n = parser->Ntokens;
    if (n < 2) return 201;
    if (!getfloat(parser->Tok[1], &y)) return setError(parser, 1, 202);

    // If MULTIPLY command, save multiplier
    if (match(parser->Tok[0], w_MULTIPLY))
    {
        if (y <= 0.0) return setError(parser, 1, 213);
        else hyd->Dmult = y;
        return 0;
    }

    // Otherwise find node (and pattern) being referenced
    if ((j = findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);
    if (j > net->Njuncs) return 0;
    if (n >= 3)
    {
        p = findpattern(net, parser->Tok[2]);
        if (p < 0) return setError(parser, 2, 205);
    }

    // Replace any demand entered in [JUNCTIONS] section
    demand = net->Node[j].D;
    if (demand && hyd->NodeDemand[j] != MISSING)
    {
        // First category encountered will overwrite demand category
        // created when junction was read from [JUNCTIONS] section
        demand->Base = y;
        demand->Pat = p;
        if (parser->Comment[0])
        {
            demand->Name = xstrcpy(&demand->Name, parser->Comment, MAXID);
        }
        hyd->NodeDemand[j] = MISSING; // marker - next iteration will append a new category.
    }

    // Otherwise add new demand to junction
    else if (!adddemand(&net->Node[j], y, p, parser->Comment) > 0) return 101;
    return 0;
}

int controldata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int          i = 0,                // Node index
                 k,                    // Link index
                 n;                    // # data items
    double       setting = MISSING,    // Link setting
                 time = 0.0,           // Simulation time
                 level = 0.0;          // Pressure or tank level
    StatusType   status = ACTIVE;      // Link status
    ControlType  ctltype;              // Control type
    LinkType     linktype;             // Link type
    Scontrol     *control;

    // Check for sufficient number of input tokens
    n = parser->Ntokens;
    if (n < 6) return 201;

    // Check that controlled link exists
    k = findlink(net, parser->Tok[1]);
    if (k == 0) return setError(parser, 1, 204);

    // Cannot control a check valve
    linktype = net->Link[k].Type;
    if (linktype == CVPIPE)  return setError(parser, 1, 207);

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
    else if (linktype == GPV) return setError(parser, 1, 207);
    else if (!getfloat(parser->Tok[2], &setting)) return setError(parser, 2, 202);

    // Set status for pump in case speed setting was supplied
    // or for pipe if numerical setting was supplied
    if (linktype == PUMP || linktype == PIPE)
    {
        if (setting != MISSING)
        {
            if (setting < 0.0)       return setError(parser, 2, 211);
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
        if ((i = findnode(net, parser->Tok[5])) == 0) return setError(parser, 5, 203);
        if (match(parser->Tok[6], w_BELOW))      ctltype = LOWLEVEL;
        else if (match(parser->Tok[6], w_ABOVE)) ctltype = HILEVEL;
        else return setError(parser, 6, 213);
  }

    // Parse control level or time
    switch (ctltype)
    {
        case TIMER:
        case TIMEOFDAY:
          if (n == 6) time = hour(parser->Tok[5], "");
          if (n == 7) time = hour(parser->Tok[5], parser->Tok[6]);
          if (time < 0.0) return setError(parser, 5, 213);
          break;
        case LOWLEVEL:
        case HILEVEL:
          if (!getfloat(parser->Tok[7], &level)) return setError(parser, 7, 202);
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

int sourcedata(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: processes water quality source data
**  Formats:
**     [SOURCE]
**        node  sourcetype  quality  (pattern)
**
**  NOTE: units of mass-based source are mass/min
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int i,              // Token with quality value
        j,              // Node index
        n,              // # data items
        p = 0;          // Time pattern index
    char type = CONCEN; // Source type
    double c0 = 0;      // Initial quality
    Psource source;

    // Check for enough tokens & that source node exists
    n = parser->Ntokens;
    if (n < 2) return 201;
    if ((j = findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);

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
    if (!getfloat(parser->Tok[i], &c0))
    {
        if (i == 1) return setError(parser, i, 213);
        else        return setError(parser, i, 202);
    }

    // Parse optional source time pattern
    if (n > i + 1 && strlen(parser->Tok[i + 1]) > 0 &&
        strcmp(parser->Tok[i + 1], "*") != 0)
    {
        p = findpattern(net, parser->Tok[i + 1]);
        if (p < 0) return setError(parser, i + 1, 205);
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

int emitterdata(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: processes junction emitter data
**  Format:
**     [EMITTER]
**        node   Ke
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int j,       // Node index
        n;       // # data items
    double k;    // Flow coeff.

    // Check that node exists & is a junction
    n = parser->Ntokens;
    if (n < 2) return 201;
    if ((j = findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);
    if (j > net->Njuncs) return 0;

    // Parse emitter flow coeff.
    if (!getfloat(parser->Tok[1], &k)) return setError(parser, 1, 202);
    if (k < 0.0) return setError(parser, 1, 209);
    net->Node[j].Ke = k;
    return 0;
}

int qualdata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int j, n;
    long i, i1, i2;
    double c0;
    Snode *Node = net->Node;

    if (net->Nnodes == 0) return setError(parser, 0, 203);  // No nodes defined yet
    n = parser->Ntokens;
    if (n < 2) return 0;

    // Single node name supplied
    if (n == 2)
    {
        if ((j = findnode(net,parser->Tok[0])) == 0) return setError(parser, 0, 203);
        if (!getfloat(parser->Tok[1], &c0)) return setError(parser, 1, 202);
        if (c0 < 0.0) return setError(parser, 1, 209);
        Node[j].C0 = c0;
    }

    // Range of node names supplied
    else
    {
        // Parse quality value
        if (!getfloat(parser->Tok[2], &c0)) return setError(parser, 2, 202);
        if (c0 < 0.0) return setError(parser, 2, 209);

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

int reactdata(Project *pr)
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
    Network *net = &pr->network;
    Quality *qual = &pr->quality;
    Parser  *parser = &pr->parser;

    int item, j, n;
    long i, i1, i2;
    double y;

    // Skip line if insufficient data
    n = parser->Ntokens;
    if (n < 3) return 0;

    // Keyword is ORDER
    if (match(parser->Tok[0], w_ORDER))
    {
        if (!getfloat(parser->Tok[n - 1], &y))  return setError(parser, n-1, 202);
        if (match(parser->Tok[1], w_BULK))      qual->BulkOrder = y;
        else if (match(parser->Tok[1], w_TANK)) qual->TankOrder = y;
        else if (match(parser->Tok[1], w_WALL))
        {
            if (y == 0.0)      qual->WallOrder = 0.0;
            else if (y == 1.0) qual->WallOrder = 1.0;
            else return setError(parser, n-1, 213);
        }
        else return setError(parser, 1, 213);
        return 0;
    }

    // Keyword is ROUGHNESS
    if (match(parser->Tok[0], w_ROUGHNESS))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n-1, 202);
        qual->Rfactor = y;
        return 0;
    }

    // Keyword is LIMITING
    if (match(parser->Tok[0], w_LIMITING))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n-1, 202);
        qual->Climit = y;
        return 0;
    }

    // Keyword is GLOBAL
    if (match(parser->Tok[0], w_GLOBAL))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n-1, 202);
        if (match(parser->Tok[1], w_BULK))      qual->Kbulk = y;
        else if (match(parser->Tok[1], w_WALL)) qual->Kwall = y;
        else return setError(parser, 1, 213);
        return 0;
    }

    // Keyword is BULK, WALL or TANK
    if (match(parser->Tok[0], w_BULK))      item = 1;
    else if (match(parser->Tok[0], w_WALL)) item = 2;
    else if (match(parser->Tok[0], w_TANK)) item = 3;
    else return setError(parser, 0, 213);

    // Case where tank rate coeffs. are being set
    if (item == 3)
    {
        // Get the rate coeff. value
        if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n-1, 202);

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
        if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n-1, 202);
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

int mixingdata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int    i,     // Tank index
           j,     // Node index
           m,     // Type of mixing model
           n;     // Number of data items
    double v;     // Mixing zone volume fraction

    // Check for valid data
    if (net->Nnodes == 0) return setError(parser, 0, 203);
    n = parser->Ntokens;
    if (n < 2) return 0;
    j = findnode(net, parser->Tok[0]);
    if (j == 0) return setError(parser, 0, 203);
    if (j <= net->Njuncs) return 0;
    if ((m = findmatch(parser->Tok[1], MixTxt)) < 0) return setError(parser, 1, 213);

    // Find mixing zone volume fraction (which can't be 0)
    v = 1.0;
    if ((m == MIX2) && (n == 3) &&
        (!getfloat(parser->Tok[2], &v))) return setError(parser, 2, 202);
    if (v == 0.0) v = 1.0;

    // Assign mixing data to tank (return if tank is a reservoir)
    i = j - net->Njuncs;
    if (net->Tank[i].A == 0.0) return 0;
    net->Tank[i].MixModel = (char)m;
    net->Tank[i].V1max = v;
    return 0;
}

int statusdata(Project *pr)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    int j, n;
    long i, i1, i2;
    double y = 0.0;
    char status = ACTIVE;

    if (net->Nlinks == 0) return setError(parser, 0, 204);
    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Check for legal status setting
    if (match(parser->Tok[n], w_OPEN))  status = OPEN;
    else if (match(parser->Tok[n], w_CLOSED)) status = CLOSED;
    else
    {
        if (!getfloat(parser->Tok[n], &y)) return setError(parser, n, 202);
        if (y < 0.0) return setError(parser, n, 211);
    }

    // A single link ID was supplied
    if (n == 1)
    {
        if ((j = findlink(net, parser->Tok[0])) == 0) return setError(parser, 0, 204);

        // Cannot change status of a Check Valve
        if (net->Link[j].Type == CVPIPE) return setError(parser, 0, 207);

        // Cannot change setting for a GPV
        if (net->Link[j].Type == GPV && status == ACTIVE) return setError(parser, 0, 207);
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

int energydata(Project *pr)
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
    Network *net = &pr->network;
    Hydraul *hyd = &pr->hydraul;
    Parser  *parser = &pr->parser;

    int j, k, n, p, c;
    double y;

    Slink *Link = net->Link;
    Spump *Pump = net->Pump;

    // Check for sufficient data
    n = parser->Ntokens;
    if (n < 3) return 201;

    // First keyword is DEMAND
    if (match(parser->Tok[0], w_DMNDCHARGE))
    {
        if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);
        if (y < 0.0) return setError(parser, 2, 213);
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
        if (k == 0) return setError(parser, 1, 216);
        if (Link[k].Type != PUMP) return setError(parser, 1, 216);
        j = findpump(net, k);
    }
    else return setError(parser, 0, 213);

    // PRICE parameter being set
    if (match(parser->Tok[n - 2], w_PRICE))
    {
        if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n-1, 202);
        if (y < 0.0) return setError(parser, n-1, 217);
        if (j == 0) hyd->Ecost = y;
        else        Pump[j].Ecost = y;
        return 0;
    }

    // Price PATTERN being set
    else if (match(parser->Tok[n - 2], w_PATTERN))
    {
        p = findpattern(net, parser->Tok[n - 1]);
        if (p < 0) return setError(parser, n - 1, 205);
        if (j == 0) hyd->Epat = p;
        else        Pump[j].Epat = p;
        return 0;
    }

    // Pump EFFIC being set
    else if (match(parser->Tok[n - 2], w_EFFIC))
    {
        if (j == 0)
        {
            if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
            if (y <= 0.0) return setError(parser, n - 1, 217);
            hyd->Epump = y;
        }
        else
        {
            c = findcurve(net, parser->Tok[n - 1]);
            if (c == 0) return setError(parser, n - 1, 206);
            Pump[j].Ecurve = c;
            net->Curve[c].Type = EFFIC_CURVE;
        }
        return 0;
    }
    return 201;
}

int reportdata(Project *pr)
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
    Network *net = &pr->network;
    Report  *rpt = &pr->report;
    Parser  *parser = &pr->parser;

    int i, j, n;
    double y;

    n = parser->Ntokens - 1;
    if (n < 1) return 201;

    // Value for page size
    if (match(parser->Tok[0], w_PAGE))
    {
        if (!getfloat(parser->Tok[n], &y)) return setError(parser, n, 202);
        if (y < 0.0 || y > 255.0) return setError(parser, n, 213);
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
            if (net->Nnodes == 0) return setError(parser, 1, 203);
            for (i = 1; i <= n; i++)
            {
                if ((j = findnode(net, parser->Tok[i])) == 0) return setError(parser, i, 203);
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
            if (net->Nlinks == 0) return setError(parser, 1, 204);
            for (i = 1; i <= n; i++)
            {
                if ((j = findlink(net, parser->Tok[i])) == 0) return setError(parser, i, 204);
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
        if (i > FRICTION) return setError(parser, 0, 213);
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
        if (parser->Ntokens < 3) return 201;
        if      (match(parser->Tok[1], w_BELOW))     j = LOW;
        else if (match(parser->Tok[1], w_ABOVE))     j = HI;
        else if (match(parser->Tok[1], w_PRECISION)) j = PREC;
        else return setError(parser, 1, 213);

        // Get field qualifier value
        if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);
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

int timedata(Project *pr)
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
    Report *rpt = &pr->report;
    Parser *parser = &pr->parser;
    Times  *time = &pr->times;

    int n;
    long t;
    double y;

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
        else return setError(parser, n, 213);
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
            if ((y = hour(parser->Tok[n - 1], parser->Tok[n])) < 0.0)
            {
                return setError(parser, n-1, 213);
            }
        }
    }
    t = (long)(3600.0 * y + 0.5);

    /// Process the value assigned to the matched parameter
    if      (match(parser->Tok[0], w_DURATION))  time->Dur = t;
    else if (match(parser->Tok[0], w_HYDRAULIC)) time->Hstep = t;
    else if (match(parser->Tok[0], w_QUALITY) )  time->Qstep = t;
    else if (match(parser->Tok[0], w_RULE))      time->Rulestep = t;
    else if (match(parser->Tok[0], w_MINIMUM))   return 0; // Not used anymore
    else if (match(parser->Tok[0], w_PATTERN))
    {
        if      (match(parser->Tok[1], w_TIME))  time->Pstep = t;
        else if (match(parser->Tok[1], w_START)) time->Pstart = t;
        else return setError(parser, 1, 213);
    }
    else if (match(parser->Tok[0], w_REPORT))
    {
        if      (match(parser->Tok[1], w_TIME))  time->Rstep = t;
        else if (match(parser->Tok[1], w_START)) time->Rstart = t;
        else return setError(parser, 1, 213);
    }
    else if (match(parser->Tok[0], w_START)) time->Tstart = t % SECperDAY;
    else return setError(parser, 0, 213);
    return 0;
}

int optiondata(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  returns error code
**  Purpose: processes [OPTIONS] data
**--------------------------------------------------------------
*/
{
    int i, n;
    Parser *parser = &pr->parser;

    // Option is a named choice
    n = parser->Ntokens - 1;
    i = optionchoice(pr, n);
    if (i >= 0) return i;

    // Option is a numerical value
    return (optionvalue(pr, n));
}

int optionchoice(Project *pr, int n)
/*
**--------------------------------------------------------------
**  Input:   n = index of last input token
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
**    DEMAND MODEL        DDA/PDA
**--------------------------------------------------------------
*/
{
    Network *net    = &pr->network;
    Hydraul *hyd    = &pr->hydraul;
    Quality *qual   = &pr->quality;
    Parser  *parser = &pr->parser;
    Outfile *out    = &pr->outfile;

    int choice;

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
        else return setError(parser, 1, 213);
    }

    // PRESSURE units
    else if (match(parser->Tok[0], w_PRESSURE))
    {
        if (n < 1) return 0;
        else if (match(parser->Tok[1], w_EXPONENT)) return -1;
        else if (match(parser->Tok[1], w_PSI))    parser->Pressflag = PSI;
        else if (match(parser->Tok[1], w_KPA))    parser->Pressflag = KPA;
        else if (match(parser->Tok[1], w_METERS)) parser->Pressflag = METERS;
        else return setError(parser, 1, 213);
    }

    // HEADLOSS formula
    else if (match(parser->Tok[0], w_HEADLOSS))
    {
        if (n < 1)  return 0;
        else if (match(parser->Tok[1], w_HW))   hyd->Formflag = HW;
        else if (match(parser->Tok[1], w_DW))   hyd->Formflag = DW;
        else if (match(parser->Tok[1], w_CM))   hyd->Formflag = CM;
        else return setError(parser, 1, 213);
    }

    // HYDRUALICS USE/SAVE file option
    else if (match(parser->Tok[0], w_HYDRAULIC))
    {
        if (n < 2) return 0;
        else if (match(parser->Tok[1], w_USE))  out->Hydflag = USE;
        else if (match(parser->Tok[1], w_SAVE)) out->Hydflag = SAVE;
        else return setError(parser, 1, 213);
        strncpy(out->HydFname, parser->Tok[2], MAXFNAME);
    }

    // Water QUALITY option
    else if (match(parser->Tok[0], w_QUALITY))
    {
        if (n < 1) return 0;
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
            if (n < 2) return 201;
            strcpy(parser->Tok[0], parser->Tok[2]);
            qual->TraceNode = findnode(net, parser->Tok[2]);
            if (qual->TraceNode == 0) return setError(parser, 2, 212);
            strncpy(qual->ChemName, u_PERCENT, MAXID);
            strncpy(qual->ChemUnits, parser->Tok[2], MAXID);
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
        else return setError(parser, 1, 213);
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
        if (choice < 0) return setError(parser, 2, 213);
        hyd->DemandModel = choice;
    }

    // Return -1 if keyword did not match any option
    else return -1;
    return 0;
}

int optionvalue(Project *pr, int n)
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

**    HEADERROR           value
**    FLOWCHANGE          value
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
    Hydraul *hyd = &pr->hydraul;
    Quality *qual = &pr->quality;
    Parser  *parser = &pr->parser;

    int    nvalue = 1;   // Index of token with numerical value
    double y;
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
    if (!getfloat(parser->Tok[nvalue], &y)) return setError(parser, nvalue, 202);

    // Quality tolerance option (which can be 0)
    if (match(tok0, w_TOLERANCE))
    {
        if (y < 0.0) return setError(parser, nvalue, 213);
        qual->Ctol = y;
        return 0;
    }

    // Diffusivity
    if (match(tok0, w_DIFFUSIVITY))
    {
        if (y < 0.0) return setError(parser, nvalue, 213);
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
          if (y < 0.0) return setError(parser, nvalue, 213);
          hyd->FlowChangeLimit = y;
          return 0;
    }

    // Head loss error limit
    else if (match(tok0, w_HEADERROR))
    {
        if (y < 0.0) return setError(parser, nvalue, 213);
        hyd->HeadErrorLimit = y;
        return 0;
    }

    // Pressure dependent demand parameters
    else if (match(tok0, w_MINIMUM))
    {
        if (y < 0.0) return setError(parser, nvalue, 213);
        // Required pressure still at default value
        if (hyd->Preq == MINPDIFF)
            hyd->Preq = y + MINPDIFF;
        // Required pressure already entered
        else if (hyd->Preq - y < MINPDIFF)
            return setError(parser, nvalue, 208);
        hyd->Pmin = y;
        return 0;
    }
    else if (match(tok0, w_REQUIRED))
    {
        if (y < 0.0) return setError(parser, nvalue, 213);
        if (y - hyd->Pmin < MINPDIFF)
            return setError(parser, nvalue, 208);
        hyd->Preq = y;
        return 0;
    }
    else if (match(tok0, w_PRESSURE))
    {
        if (y < 0.0) return setError(parser, nvalue, 213);
        hyd->Pexp = y;
        return 0;
    }

    // All other options must be > 0
    if (y <= 0.0) return setError(parser, nvalue, 213);

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

int getpumpcurve(Project *pr, int n)
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
    Network *net = &pr->network;
    Parser  *parser = &pr->parser;

    double a, b, c, h0, h1, h2, q1, q2;
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

void changestatus(Network *net, int j, StatusType status, double y)
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
        if (status != ACTIVE) link->Status = status;
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
        link->Status = status;
    }
    else if (link->Type >= PRV)
    {
        link->Kc = y;
        link->Status = status;
        if (status != ACTIVE) link->Kc = MISSING;
    }
}
