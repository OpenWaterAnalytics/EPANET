/*
******************************************************************************
Project:      OWA EPANET
Version:      2.3
Module:       input2.c
Description:  reads and interprets network data from an EPANET input file
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
Last Updated: 09/28/2023
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

extern char *SectTxt[]; // Input section keywords (see ENUMSTXT.H)

// Exported functions
int addnodeID(Network *, int, char *);
int addlinkID(Network *, int, char *);
int getunitsoption(Project *, char *);
int getheadlossoption(Project *, char *);

// Local functions
static int  newline(Project *, int, char *);
static int  addpattern(Network *, char *);
static int  addcurve(Network *, char *);
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
    Spattern *pattern;

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


    // Add a "dummy" time pattern with index of 0 and a single multiplier
    // of 1.0 to be used by all demands not assigned a pattern
    pr->network.Npats = -1;
    errcode = addpattern(&pr->network, "");
    if (errcode) return errcode;
    pattern = &pr->network.Pattern[0];
    pattern->Length = 1;
    pattern[0].F = (double *)calloc(1, sizeof(double));
    pattern[0].F[0] = 1.0;
    parser->MaxPats = pr->network.Npats;

    // Make a pass through input file counting number of each object
    if (parser->InFile == NULL) return 0;
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
            else
            {
                sect = -1;
                continue;
            }
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
            case _PATTERNS:
                errcode = addpattern(&pr->network, tok);
                parser->MaxPats = pr->network.Npats;
                break;
            case _CURVES:
                errcode = addcurve(&pr->network, tok);
                parser->MaxCurves = pr->network.Ncurves;
                break;
            case _OPTIONS:
                if (match(tok, w_UNITS))
                    getunitsoption(pr, strtok(line, SEPSTR));
                else if (match(tok, w_HEADLOSS))
                    getheadlossoption(pr, strtok(line, SEPSTR));
                break;
        }
        if (errcode) break;
    }

    parser->MaxNodes = parser->MaxJuncs + parser->MaxTanks;
    parser->MaxLinks = parser->MaxPipes + parser->MaxPumps + parser->MaxValves;
    if (parser->MaxPats < 1) parser->MaxPats = 1;
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
	char errmsg[MAXMSG + 1] = "";		 
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

    // Patterns & Curves were created previously in netsize()
    parser->MaxPats = net->Npats;
    parser->MaxCurves = net->Ncurves;
    parser->PrevPat = NULL;
    parser->PrevCurve = NULL;

    // Initialize full line comment, input data section and error count
    parser->LineComment[0] = '\0';
    sect = -1;
    errsum = 0;

    // Read each line from input file
    while (fgets(line, MAXLINE, parser->InFile) != NULL)
    {
        // Make copy of line and scan for tokens
        strcpy(wline, line);
        parser->Ntokens = gettokens(wline, parser->Tok, MAXTOKS, parser->Comment);

        // Skip blank lines and those filled with a comment
        parser->ErrTok = -1;
        if (parser->Ntokens == 0)
        {
            // Store full line comment for Patterns and Curves
            if (sect == _PATTERNS || sect == _CURVES)
            {
                strncpy(parser->LineComment, parser->Comment, MAXMSG);
            }
            continue;
        }

        // Apply full line comment for Patterns and Curves
        if (sect == _PATTERNS || sect == _CURVES)
        {
            strcpy(parser->Comment, parser->LineComment);
        }
        parser->LineComment[0] = '\0';

        // Check if max. line length exceeded
        if (strlen(line) >= MAXLINE)
        {
            sprintf(pr->Msg, "%s section: %s", geterrmsg(214, errmsg), SectTxt[sect]);
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
                sect = -1;
                parser->ErrTok = 0;
                errsum++;
                inperrmsg(pr, 299, sect, line);
                continue;
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
            else continue;
        }
    }

    // Check for errors
    if (errsum > 0) errcode = 200;

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
              line[n - 1] = '\0';
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
              deleterule(pr, pr->network.Nrules);
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
        case _VERTICES:    return (vertexdata(pr));

        // Data in these sections are not used for any computations
        case _LABELS:
        case _TAGS:
        case _BACKDROP:
          return (0);
    }
    return 201;
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
    if (findnode(net,id))
      return 215;  // duplicate id
    if (strlen(id) > MAXID)
      return 252;  // invalid format (too long)
    strncpy(net->Node[n].ID, id, MAXID);
    hashtable_insert(net->NodeHashTable, net->Node[n].ID, n);
    return 0;
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
    if (findlink(net,id))
      return 215;  // duplicate id
    if (strlen(id) > MAXID)
      return 252; // invalid formt (too long);
    strncpy(net->Link[n].ID, id, MAXID);
    hashtable_insert(net->LinkHashTable, net->Link[n].ID, n);
    return 0;
}

int addpattern(Network *network, char *id)
/*
**-------------------------------------------------------------
**  Input:   id = pattern ID label
**  Output:  returns error code
**  Purpose: adds a new pattern to the database
**--------------------------------------------------------------
*/
{
    int n = network->Npats;
    Spattern *pattern;

    // Check if pattern was already created
    if (n > 0)
    {
        if (strcmp(id, network->Pattern[n].ID) == 0) return 0;
        if (findpattern(network, id) > 0) return 0;
    }
    if (strlen(id) > MAXID) return 252;

    // Update pattern count & add a new pattern to the database
    n = n + 2;
    network->Pattern = (Spattern *)realloc(network->Pattern, n * sizeof(Spattern));
    if (network->Pattern == NULL) return 101;
    (network->Npats)++;

    // Initialize the pattern
    pattern = &network->Pattern[network->Npats];
    strncpy(pattern->ID, id, MAXID);
    pattern->Comment = NULL;
    pattern->Length = 0;
    pattern->F = NULL;
    return 0;
}

int addcurve(Network *network, char *id)
/*
**-------------------------------------------------------------
**  Input:   id = curve ID label
**  Output:  returns error code
**  Purpose: adds a new curve to the database
**--------------------------------------------------------------
*/
{
    int n = network->Ncurves;
    Scurve *curve;

    // Check if was already created
    if (n > 0)
    {
        if (strcmp(id, network->Curve[n].ID) == 0) return 0;
        if (findcurve(network, id) > 0) return 0;
    }
    if (strlen(id) > MAXID) return 252;

    n = n + 2;
    network->Curve = (Scurve *)realloc(network->Curve, n * sizeof(Scurve));
    if (network->Curve == NULL) return 101;
    (network->Ncurves)++;

    // Initialize the curve
    curve = &network->Curve[network->Ncurves];
    strncpy(curve->ID, id, MAXID);
    curve->Type = GENERIC_CURVE;
    curve->Comment = NULL;
    curve->Capacity = 0;
    curve->Npts = 0;
    curve->X = NULL;
    curve->Y = NULL;
    return 0;
}

int getunitsoption(Project *pr, char *units)
/*
**-------------------------------------------------------------
**  Input:   units = name of flow units to be used
**  Output:  returns 1 if successful, 0 if not
**  Purpose: sets the flows units to be used by a project.
**--------------------------------------------------------------
*/
{
    Parser *parser = &pr->parser;
    if      (match(units, w_CFS))  parser->Flowflag = CFS;
    else if (match(units, w_GPM))  parser->Flowflag = GPM;
    else if (match(units, w_AFD))  parser->Flowflag = AFD;
    else if (match(units, w_MGD))  parser->Flowflag = MGD;
    else if (match(units, w_IMGD)) parser->Flowflag = IMGD;
    else if (match(units, w_LPS))  parser->Flowflag = LPS;
    else if (match(units, w_LPM))  parser->Flowflag = LPM;
    else if (match(units, w_CMH))  parser->Flowflag = CMH;
    else if (match(units, w_CMD))  parser->Flowflag = CMD;
    else if (match(units, w_MLD))  parser->Flowflag = MLD;
    else if (match(units, w_CMS))  parser->Flowflag = CMS;
    else if (match(units, w_SI))   parser->Flowflag = LPS;
    else return 0;
    if (parser->Flowflag >= LPS) parser->Unitsflag = SI;
    else parser->Unitsflag = US;
    return 1;
}

int getheadlossoption(Project *pr, char *formula)
/*
**-------------------------------------------------------------
**  Input:   formula = name of head loss formula to be used
**  Output:  returns 1 if successful, 0 if not
**  Purpose: sets the head loss formula to be used by a project.
**--------------------------------------------------------------
*/
{
    Hydraul *hyd = &pr->hydraul;
    if      (match(formula, w_HW))   hyd->Formflag = HW;
    else if (match(formula, w_DW))   hyd->Formflag = DW;
    else if (match(formula, w_CM))   hyd->Formflag = CM;
    else return 0;
    return 1;
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
    int  n;
    int len, m;
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
            len = (int)strlen(c2);
            if (len > 0)
            {
                len = (int)strcspn(c2, "\n\r");
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
        if (m == len)                   // s is last token
        {
            Tok[n] = s;
            n++;
            break;
        }
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
    if (err == 299)
        sprintf(pr->Msg, "Error %d: %s %s: section contents ignored.",
            err, geterrmsg(err, errStr), tok);
    else        
        sprintf(pr->Msg, "Error %d: %s %s in %s section:",
            err, geterrmsg(err, errStr), tok, SectTxt[sect]);
    writeline(pr, pr->Msg);

    // Echo input line
    writeline(pr, line);
}
