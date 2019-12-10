/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       text.h
 Description:  string constants used throughout EPANET
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 07/15/2019
 ******************************************************************************
*/

#ifndef TEXT_H
#define TEXT_H

//------- Keyword Dictionary ------------------------------

#define   w_USE         "USE"
#define   w_SAVE        "SAVE"

#define   w_NONE        "NONE"
#define   w_ALL         "ALL"

#define   w_CHEM        "CHEM"
#define   w_AGE         "AGE"
#define   w_TRACE       "TRACE"

#define   w_SYSTEM      "SYSTEM"
#define   w_JUNC        "Junc"
#define   w_RESERV      "Reser"
#define   w_TANK        "Tank"
#define   w_CV          "CV"
#define   w_PIPE        "Pipe"
#define   w_PUMP        "Pump"
#define   w_VALVE       "Valve"
#define   w_PRV         "PRV"
#define   w_PSV         "PSV"
#define   w_PBV         "PBV"
#define   w_FCV         "FCV"
#define   w_TCV         "TCV"
#define   w_GPV         "GPV"

#define   w_OPEN        "OPEN"
#define   w_CLOSED      "CLOSED"
#define   w_ACTIVE      "ACTIVE"
#define   w_TIME        "TIME"
#define   w_ABOVE       "ABOVE"
#define   w_BELOW       "BELOW"
#define   w_PRECISION   "PREC"
#define   w_IS          "IS"
#define   w_NOT         "NOT"

#define   w_ADD         "ADD"
#define   w_MULTIPLY    "MULT"

#define   w_LIMITING    "LIMIT"
#define   w_ORDER       "ORDER"
#define   w_GLOBAL      "GLOB"
#define   w_BULK        "BULK"
#define   w_WALL        "WALL"

#define   w_PAGE        "PAGE"
#define   w_STATUS      "STATUS"
#define   w_SUMMARY     "SUMM"
#define   w_MESSAGES    "MESS"
#define   w_ENERGY      "ENER"
#define   w_NODE        "NODE"
#define   w_LINK        "LINK"
#define   w_FILE        "FILE"
#define   w_YES         "YES"
#define   w_NO          "NO"
#define   w_FULL        "FULL"

#define   w_HW          "H-W"
#define   w_DW          "D-W"
#define   w_CM          "C-M"

#define   w_CFS         "CFS"
#define   w_GPM         "GPM"
#define   w_MGD         "MGD"
#define   w_IMGD        "IMGD"
#define   w_AFD         "AFD"
#define   w_LPS         "LPS"
#define   w_LPM         "LPM"
#define   w_MLD         "MLD"
#define   w_CMH         "CMH"
#define   w_CMD         "CMD"
#define   w_SI          "SI"

#define   w_PSI         "PSI"
#define   w_KPA         "KPA"
#define   w_METERS      "METERS"

#define   w_ELEV        "ELEV"
#define   w_DEMAND      "DEMAND"
#define   w_HEAD        "HEAD"
#define   w_PRESSURE    "PRESSURE"
#define   w_QUALITY     "QUAL"

#define   w_DIAM        "DIAM"
#define   w_FLOW        "FLOW"
#define   w_ROUGHNESS   "ROUG"
#define   w_VELOCITY    "VELO"
#define   w_HEADLOSS    "HEADL"
#define   w_SETTING     "SETTING"
#define   w_POWER       "POWER"
#define   w_VOLUME      "VOLU"
#define   w_CLOCKTIME   "CLOCKTIME"
#define   w_FILLTIME    "FILLTIME"
#define   w_DRAINTIME   "DRAINTIME"
#define   w_GRADE       "GRADE"
#define   w_LEVEL       "LEVEL"

#define   w_DURATION    "DURA"
#define   w_HYDRAULIC   "HYDR"
#define   w_MINIMUM     "MINI"
#define   w_PATTERN     "PATT"
#define   w_REPORT      "REPO"
#define   w_START       "STAR"

#define   w_UNITS       "UNIT"
#define   w_MAP         "MAP"
#define   w_VERIFY      "VERI"
#define   w_VISCOSITY   "VISC"
#define   w_DIFFUSIVITY "DIFF"
#define   w_SPECGRAV    "SPEC"
#define   w_TRIALS      "TRIAL"
#define   w_ACCURACY    "ACCU"
#define   w_SEGMENTS    "SEGM"
#define   w_TOLERANCE   "TOLER"
#define   w_EMITTER     "EMIT"

#define   w_PRICE       "PRICE"
#define   w_DMNDCHARGE  "DEMAN"

#define   w_HTOL        "HTOL"
#define   w_QTOL        "QTOL"
#define   w_RQTOL       "RQTOL"
#define   w_CHECKFREQ   "CHECKFREQ"
#define   w_MAXCHECK    "MAXCHECK"
#define   w_DAMPLIMIT   "DAMPLIMIT"

#define   w_FLOWCHANGE  "FLOWCHANGE"
#define   w_HEADERROR   "HEADERROR"

#define   w_MODEL       "MODEL"
#define   w_DDA         "DDA"
#define   w_PDA         "PDA"
#define   w_REQUIRED    "REQ"
#define   w_EXPONENT    "EXP"

#define   w_SECONDS     "SEC"
#define   w_MINUTES     "MIN"
#define   w_HOURS       "HOU"
#define   w_DAYS        "DAY"
#define   w_AM          "AM"
#define   w_PM          "PM"

#define   w_CONCEN      "CONCEN"
#define   w_MASS        "MASS"
#define   w_SETPOINT    "SETPOINT"
#define   w_FLOWPACED   "FLOWPACED"

#define   w_PATTERN     "PATT"
#define   w_CURVE       "CURV"

#define   w_EFFIC       "EFFI"
#define   w_HEAD        "HEAD"
#define   w_POWER       "POWER"
#define   w_SPEED       "SPEE"

#define   w_MIXED       "MIXED"
#define   w_2COMP       "2COMP"
#define   w_FIFO        "FIFO"
#define   w_LIFO        "LIFO"

#define   w_STATISTIC   "STAT"
#define   w_AVG         "AVERAGE"
#define   w_MIN         "MINIMUM"
#define   w_MAX         "MAXIMUM"
#define   w_RANGE       "RANGE"

#define   w_UNBALANCED  "UNBA"
#define   w_STOP        "STOP"
#define   w_CONTINUE    "CONT"

#define   w_RULE        "RULE"
#define   w_IF          "IF"
#define   w_AND         "AND"
#define   w_OR          "OR"
#define   w_THEN        "THEN"
#define   w_ELSE        "ELSE"
#define   w_PRIORITY    "PRIO"

// ------ Input File Section Names ------------------------

#define   s_TITLE       "[TITLE]"
#define   s_JUNCTIONS   "[JUNCTIONS]"
#define   s_RESERVOIRS  "[RESERVOIRS]"
#define   s_TANKS       "[TANKS]"
#define   s_PIPES       "[PIPES]"
#define   s_PUMPS       "[PUMPS]"
#define   s_VALVES      "[VALVES]"
#define   s_CONTROLS    "[CONTROLS]"
#define   s_RULES       "[RULES]"
#define   s_DEMANDS     "[DEMANDS]"
#define   s_SOURCES     "[SOURCES]"
#define   s_EMITTERS    "[EMITTERS]"
#define   s_PATTERNS    "[PATTERNS]"
#define   s_CURVES      "[CURVES]"
#define   s_QUALITY     "[QUALITY]"
#define   s_STATUS      "[STATUS]"
#define   s_ROUGHNESS   "[ROUGHNESS]"
#define   s_ENERGY      "[ENERGY]"
#define   s_REACTIONS   "[REACTIONS]"
#define   s_MIXING      "[MIXING]"
#define   s_REPORT      "[REPORT]"
#define   s_TIMES       "[TIMES]"
#define   s_OPTIONS     "[OPTIONS]"
#define   s_COORDS      "[COORDINATES]"
#define   s_VERTICES    "[VERTICES]"
#define   s_LABELS      "[LABELS]"
#define   s_BACKDROP    "[BACKDROP]"
#define   s_TAGS        "[TAGS]"
#define   s_END         "[END]"

//------- Units -------------------------------------------

#define   u_CFS         "cfs"
#define   u_GPM         "gpm"
#define   u_AFD         "a-f/d"
#define   u_MGD         "mgd"
#define   u_IMGD        "Imgd"
#define   u_LPS         "L/s"
#define   u_LPM         "Lpm"
#define   u_CMH         "m3/h"
#define   u_CMD         "m3/d"
#define   u_MLD         "ML/d"
#define   u_MGperL      "mg/L"
#define   u_UGperL      "ug/L"
#define   u_HOURS       "hrs"
#define   u_MINUTES     "min"
#define   u_PERCENT     "% from"
#define   u_METERS      "m"
#define   u_MMETERS     "mm"
#define   u_MperSEC     "m/s"
#define   u_SQMperSEC   "sq m/sec"
#define   u_per1000M    "/1000m"
#define   u_KW          "kw"
#define   u_FEET        "ft"
#define   u_INCHES      "in"
#define   u_PSI         "psi"
#define   u_KPA         "kPa"
#define   u_FTperSEC    "fps"
#define   u_SQFTperSEC  "sq ft/sec"
#define   u_per1000FT   "/1000ft"
#define   u_HP          "hp"

//------- Curve Types ------------------------------------- 

#define   c_HEADLOSS    "HEADLOSS"
#define   c_PUMP        "PUMP"
#define   c_EFFIC       "EFFIC"
#define   c_VOLUME      "VOLUME"

//------- Text Phrases ------------------------------------

#define   t_ABOVE       "above"
#define   t_BELOW       "below"
#define   t_HW          "Hazen-Williams"
#define   t_DW          "Darcy-Weisbach"
#define   t_CM          "Chezy-Manning"
#define   t_CHEMICAL    "Chemical"
#define   t_XHEAD       "closed because cannot deliver head"
#define   t_TEMPCLOSED  "temporarily closed"
#define   t_CLOSED      "closed"
#define   t_OPEN        "open"
#define   t_ACTIVE      "active"
#define   t_XFLOW       "open but exceeds maximum flow"
#define   t_XFCV        "open but cannot deliver flow"
#define   t_XPRESSURE   "open but cannot deliver pressure"
#define   t_FILLING     "filling"
#define   t_EMPTYING    "emptying"
#define   t_OVERFLOWING "overflowing"

#define   t_ELEV        "Elevation"
#define   t_DEMAND      "Demand"
#define   t_HEAD        "Head"
#define   t_PRESSURE    "Pressure"
#define   t_QUALITY     "Quality"
#define   t_LENGTH      "Length"
#define   t_DIAM        "Diameter"
#define   t_FLOW        "Flow"
#define   t_VELOCITY    "Velocity"
#define   t_HEADLOSS    "Headloss"
#define   t_LINKQUAL    "Quality"
#define   t_LINKSTATUS  "State"
#define   t_SETTING     "Setting"
#define   t_REACTRATE   "Reaction"
#define   t_FRICTION    "F-Factor"

#define   t_NODEID      "Node"
#define   t_LINKID      "Link"
#define   t_PERDAY      "/day"

#define   t_JUNCTION    "Junction"
#define   t_RESERVOIR   "Reservoir"
#define   t_TANK        "Tank"
#define   t_PIPE        "Pipe"
#define   t_PUMP        "Pump"
#define   t_VALVE       "Valve"
#define   t_CONTROL     "Control"
#define   t_RULE        "Rule"
#define   t_DEMANDFOR   "Demand for Node"
#define   t_SOURCE      "Source"
#define   t_EMITTER     "Emitter"
#define   t_PATTERN     "Pattern"
#define   t_CURVE       "Curve"
#define   t_STATUS      "Status"
#define   t_ROUGHNESS   "Roughness"
#define   t_ENERGY      "Energy"
#define   t_REACTION    "Reaction"
#define   t_MIXING      "Mixing"
#define   t_REPORT      "Report"
#define   t_TIME        "Times"
#define   t_OPTION      "Options"
#define   t_RULES_SECT  "[RULES] section"
#define   t_HALTED      " EXECUTION HALTED."
#define   t_FUNCCALL    "function call"
#define   t_CONTINUED   " (continued)"
#define   t_perM3       "  /m3"
#define   t_perMGAL     "/Mgal"
#define   t_DIFFER      "DIFFERENTIAL"
#define   t_FIXED       "Fixed Demands"
#define   t_POWER       "Power Function"
#define   t_ORIFICE     "Orifice Flow"


//----- Summary Report Format Strings ---------------------

#define LOGO1  \
"******************************************************************"
#define LOGO2  \
"*                           E P A N E T                          *"
#define LOGO3  \
"*                   Hydraulic and Water Quality                  *"
#define LOGO4  \
"*                   Analysis for Pipe Networks                   *"
#define LOGO5  \
"*                         Version %d.%d                            *"
#define LOGO6  \
"******************************************************************"
#define FMT02  "\n  o Retrieving network data"
#define FMT04  "\n    Cannot use duplicate file names."
#define FMT05  "\n    Cannot open input file "
#define FMT06  "\n    Cannot open report file "
#define FMT07  "\n    Cannot open output file "
#define FMT08  "\n    Cannot open temporary output file"
#define FMT14  "\n  o Computing hydraulics at hour "
#define FMT15  "\n  o Computing water quality at hour "
#define FMT16  "\n  o Transferring results to file"
#define FMT17  "\n  o Writing output report to "
#define FMT18  "  Page 1                                    "
#define FMT19  "    Input Data File ................... %s"
#define FMT20  "    Number of Junctions................ %-d"
#define FMT21a "    Number of Reservoirs............... %-d"
#define FMT21b "    Number of Tanks ................... %-d"
#define FMT22  "    Number of Pipes ................... %-d"
#define FMT23  "    Number of Pumps ................... %-d"
#define FMT24  "    Number of Valves .................. %-d"
#define FMT25  "    Headloss Formula .................. %s"
#define FMT25a "    Nodal Demand Model ................ %s"
#define FMT26  "    Hydraulic Timestep ................ %-.2f %s"
#define FMT27  "    Hydraulic Accuracy ................ %-.6f"

#define FMT27a "    Status Check Frequency ............ %-d"                   
#define FMT27b "    Maximum Trials Checked ............ %-d"                   
#define FMT27c "    Damping Limit Threshold ........... %-.6f"

#define FMT27d "    Headloss Error Limit .............. %-.6f %s"
#define FMT27e "    Flow Change Limit ................. %-.6f %s"

#define FMT28  "    Maximum Trials .................... %-d"
#define FMT29  "    Quality Analysis .................. None"
#define FMT30  "    Quality Analysis .................. %s"
#define FMT31  "    Quality Analysis .................. Trace From Node %s"
#define FMT32  "    Quality Analysis .................. Age"
#define FMT33  "    Water Quality Time Step ........... %-.2f min"
#define FMT34  "    Water Quality Tolerance ........... %-.2f %s"
#define FMT36  "    Specific Gravity .................. %-.2f"
#define FMT37a "    Relative Kinematic Viscosity ...... %-.2f"
#define FMT37b "    Relative Chemical Diffusivity ..... %-.2f"
#define FMT38  "    Demand Multiplier ................. %-.2f"
#define FMT39  "    Total Duration .................... %-.2f %s"
#define FMT40  "    Reporting Criteria:"
#define FMT41  "       No Nodes"
#define FMT42  "       All Nodes"
#define FMT43  "       Selected Nodes"
#define FMT44  "       No Links"
#define FMT45  "       All Links"
#define FMT46  "       Selected Links"
#define FMT47  "       with %s below %-.2f %s"
#define FMT48  "       with %s above %-.2f %s"

//----- Status Report Format Strings ----------------------

#define FMT49  "Hydraulic Status:"
#define FMT50  "%10s: Tank %s is %s at %-.2f %s"
#define FMT51  "%10s: Reservoir %s is %s"
#define FMT52  "%10s: %s %s %s"
#define FMT53  "%10s: %s %s changed from %s to %s"
#define FMT54  "%10s: %s %s changed by %s %s control"
#define FMT55  "%10s: %s %s changed by timer control"
#define FMT56  "            %s %s setting changed to %-.2f"
#define FMT57  "            %s %s switched from %s to %s"
#define FMT58  "%10s: Balanced after %-d trials"
#define FMT59  "%10s: Unbalanced after %-d trials (flow change = %-.6f)"

#define FMT60a "            Max. flow imbalance is %.4f %s at Node %s"         
#define FMT60b "            Max. head imbalance is %.4f %s at Link %s"         

#define FMT61  "%10s: Valve %s caused ill-conditioning"
#define FMT62  "%10s: System ill-conditioned at node %s"
#define FMT63  "%10s: %s %s changed by rule %s"
#define FMT64  "%10s: Balancing the network:\n"
#define FMT65  "            Trial %2d: relative flow change = %-.6f"
#define FMT66  "                      maximum  flow change = %.4f for Link %s"
#define FMT67  "                      maximum  flow change = %.4f for Node %s"
#define FMT68  "                      maximum  head error  = %.4f for Link %s\n"
#define FMT69a "            1 node had its demand reduced by a total of %.2f%%"
#define FMT69b "            %-d nodes had demands reduced by a total of %.2f%%"

//----- Energy Report Table -------------------------------

#define FMT71  "Energy Usage:"
#define FMT72  \
        "           Usage   Avg.     Kw-hr      Avg.      Peak      Cost"
#define FMT73  \
        "Pump      Factor Effic.     %s        Kw        Kw      /day"
#define FMT74  "%38s Demand Charge: %9.2f"
#define FMT75  "%38s Total Cost:    %9.2f"

//----- Node Report Table ---------------------------------

#define FMT76  "%s Node Results:"
#define FMT77  "Node Results:"
#define FMT78  "Node Results at %s hrs:"

//----- Link Report Table ---------------------------------

#define FMT79  "%s Link Results:"
#define FMT80  "Link Results:"
#define FMT81  "Link Results at %s hrs:"
#define FMT82  "\n\f\n  Page %-d    %60.60s\n"

//----- Progress Messages ---------------------------------

#define FMT100 "    Retrieving network data ...                   "
#define FMT101 "    Computing hydraulics at hour %-10s       "
#define FMT102 "    Computing water quality at hour %-10s    "
#define FMT103 "    Writing output report ...                     "
#define FMT106 "    Transferring results to file ...              "
#define FMT104 "Analysis begun %s"
#define FMT105 "Analysis ended %s"

//----- Rule Error Messages -------------------------------

#define R_ERR201 "Input Error 201: syntax error in following line of "
#define R_ERR202 "Input Error 202: illegal numeric value in following line of "
#define R_ERR203 "Input Error 203: undefined node in following line of "
#define R_ERR204 "Input Error 204: undefined link in following line of "
#define R_ERR207 "Input Error 207: attempt to control a CV in following line of "
#define R_ERR221 "Input Error 221: mis-placed clause in following line of "

//----- Specific Warning Messages -------------------------

#define WARN01 "WARNING: System unbalanced at %s hrs."
#define WARN02 \
"WARNING: Maximum trials exceeded at %s hrs. System may be unstable."
#define WARN03a "WARNING: Node %s disconnected at %s hrs"
#define WARN03b "WARNING: %d additional nodes disconnected at %s hrs"
#define WARN03c "WARNING: System disconnected because of Link %s" 
#define WARN04  "WARNING: Pump %s %s at %s hrs."
#define WARN05  "WARNING: %s %s %s at %s hrs."
#define WARN06  "WARNING: Negative pressures at %s hrs."

//----- General Warning Messages --------------------------

#define WARN1 "WARNING: System hydraulically unbalanced."
#define WARN2 "WARNING: System may be hydraulically unstable."
#define WARN3 "WARNING: System disconnected."
#define WARN4 "WARNING: Pumps cannot deliver enough flow or head."
#define WARN5 "WARNING: Valves cannot deliver enough flow."
#define WARN6 "WARNING: System has negative pressures."

#endif
