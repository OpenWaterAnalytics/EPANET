/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.h
 Description:  declarations for the legacy EPANET 2 API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/29/2018
 ******************************************************************************
 */


#ifndef EPANET2_H
#define EPANET2_H


// the toolkit can be compiled with support for double-precision as well.
// just make sure that you use the correct #define in your client code.
#ifndef EN_API_FLOAT_TYPE
  #define EN_API_FLOAT_TYPE float
#endif

#ifdef WITH_GENX
   #include "epanet2_export.h"
#else
  // --- define WINDOWS
  #undef WINDOWS
  #ifdef _WIN32
    #define WINDOWS
  #endif
  #ifdef __WIN32__
    #define WINDOWS
  #endif

  // --- define DLLEXPORT
  #ifndef DLLEXPORT
    #ifdef WINDOWS
      #ifdef __cplusplus
        #define DLLEXPORT __declspec(dllexport)
      #else
        #define DLLEXPORT __declspec(dllexport) __stdcall
      #endif // __cplusplus
    #elif defined(CYGWIN)
      #define DLLEXPORT __stdcall
    #elif defined(__APPLE__)
      #ifdef __cplusplus
        #define DLLEXPORT
      #else
        #define DLLEXPORT
      #endif
    #else
      #define DLLEXPORT
    #endif
  #endif
#endif

#include "epanet2_enums.h"


// --- Declare the EPANET toolkit functions
#if defined(__cplusplus)
extern "C" {
#endif


/********************************************************************

    System Functions

********************************************************************/

  /**
  brief runs a complete EPANET simulation
  param inpFile pointer to name of input file (must exist)
  param rptFile pointer to name of report file (to be created)
  param binOutFile pointer to name of binary output file (to be created)
  param callback a callback function that takes a character string (char *) as its only parameter.
  return error code

  The callback function should reside in and be used by the calling
  code to display the progress messages that EPANET generates
  as it carries out its computations. If this feature is not
  needed then the argument should be NULL.
  */
  int  DLLEXPORT ENepanet(const char *inpFile, const char *rptFile,
                 const char *binOutFile, void (*callback) (char *));

  /**
   @brief Initializes an EPANET session
   @param rptFile pointer to name of report file (to be created)
   @param binOutFile pointer to name of binary output file (to be created)
   @param UnitsType flow units flag
   @param HeadlossFormula headloss formula flag
   @return error code
   */
  int  DLLEXPORT ENinit(const char *rptFile, const char *binOutFile,
                 int UnitsType, int HeadlossFormula);

  /**
   @brief Opens EPANET input file & reads in network data
   @param inpFile pointer to name of input file (must exist)
   @param rptFile pointer to name of report file (to be created)
   @param binOutFile pointer to name of binary output file (to be created)
   @return error code
   */
  int  DLLEXPORT ENopen(const char *inpFile, const char *rptFile,
                 const char *binOutFile);

  /**
   @brief Saves current data to "INP" formatted text file.
   @param filename The file path to create
   @return Error code
   */
  int  DLLEXPORT ENsaveinpfile(const char *filename);

  /**
   @brief Frees all memory and files used by EPANET
   @return Error code
   */
  int  DLLEXPORT ENclose();

/********************************************************************

    Hydraulic Analysis Functions

********************************************************************/

  /**
   @brief Solves the network hydraulics for all time periods
   @return Error code
   */
  int  DLLEXPORT ENsolveH();

  /**
   @brief Saves hydraulic results to binary file
   @return Error code

   Must be called before ENreport() if no WQ simulation has been made.
   Should not be called if ENsolveQ() will be used.
   */
  int  DLLEXPORT ENsaveH();

  /**
   @brief Sets up data structures for hydraulic analysis
   @return Error code
   */
  int  DLLEXPORT ENopenH();

  /**
   @brief Initializes hydraulic analysis
   @param initFlag 2-digit initialization flag
   @return Error code

   The initialization flag is a two digit number where the 1st (left) digit
   indicates if link flows should be re-initialized (1) or not (0), and the
   2nd digit indicates if hydraulic results should be saved to file (1) or not (0).
   */
  int  DLLEXPORT ENinitH(int initFlag);

  /**
   @brief Run a hydraulic solution period
   @param[out] currentTime The current simulation time in seconds
   @return Error or warning code
   @see ENsolveH

   This function is used in a loop with ENnextH() to run
   an extended period hydraulic simulation.
   See ENsolveH() for an example.
   */
  int  DLLEXPORT ENrunH(long *currentTime);

  /**
   @brief Determine time (in seconds) until next hydraulic event
   @param[out] tStep Time (seconds) until next hydraulic event. 0 marks end of simulation period.
   @return Error code

   This function is used in a loop with ENrunH() to run an extended period hydraulic simulation.
   See ENsolveH() for an example.
   */
  int  DLLEXPORT ENnextH(long *tStep);


  /**
   @brief Frees data allocated by hydraulics solver
   @return Error code
   */
  int  DLLEXPORT ENcloseH();

  /**
   @brief Copies binary hydraulics file to disk
   @param filename Name of file to be created
   @return Error code
   */
  int  DLLEXPORT ENsavehydfile(char *filename);

  /**
   @brief Opens previously saved binary hydraulics file
   @param filename Name of file to be used
   @return Error code
   */
  int  DLLEXPORT ENusehydfile(char *filename);

/********************************************************************

    Water Quality Analysis Functions

********************************************************************/

  /**
   @brief Solves for network water quality in all time periods
   @return Error code
   */
  int  DLLEXPORT ENsolveQ();

  /**
   @brief Sets up data structures for WQ analysis
   @return Error code
   */
  int  DLLEXPORT ENopenQ();

  /**
   @brief Initializes water quality analysis
   @param saveFlag EN_SAVE (1) if results saved to file, EN_NOSAVE (0) if not
   @return Error code
   */
  int  DLLEXPORT ENinitQ(int saveFlag);

  /**
   @brief Retrieves hydraulic & WQ results at time t.
   @param[out] currentTime Current simulation time, in seconds.
   @return Error code

   This function is used in a loop with ENnextQ() to run
   an extended period WQ simulation. See ENsolveQ() for
   an example.
   */
  int  DLLEXPORT ENrunQ(long *currentTime);

  /**
   @brief Advances WQ simulation to next hydraulic event.
   @param[out] tStep Time in seconds until next hydraulic event. 0 marks end of simulation period.
   @return Error code

   This function is used in a loop with ENrunQ() to run
   an extended period WQ simulation. See ENsolveQ() for
   an example.
   */
  int  DLLEXPORT ENnextQ(long *tStep);

  /**
   @brief Advances WQ simulation by a single WQ time step
   @param[out] timeLeft Time left in overall simulation (in seconds)
   @return Error code

   This function is used in a loop with ENrunQ() to run
   an extended period WQ simulation.
   */
  int  DLLEXPORT ENstepQ(long *timeLeft);

  /**
   @brief Frees data allocated by water quality solver.
   @return Error code.
   */
  int  DLLEXPORT ENcloseQ();

/********************************************************************

    Reporting Functions

********************************************************************/

  /**
   @brief Writes line of text to the report file.
   @param line Text string to write
   @return Error code.
   */
  int  DLLEXPORT ENwriteline(char *line);

  /**
   @brief Writes simulation report to the report file
   @return Error code
   */
  int  DLLEXPORT ENreport();

  /**
   @brief Resets report options to default values
   @return Error code
   */
  int  DLLEXPORT ENresetreport();

  /**
   @brief Processes a reporting format command
   @return Error code
   */
  int  DLLEXPORT ENsetreport(char *reportFormat);

  /**
  @brief Set the level of hydraulic status reporting.
  @param code Status reporting code (see EN_StatusReport).
  @return Error code.
  */
  int  DLLEXPORT ENsetstatusreport(int code);

  /**
  @brief Get the API version number.
  @param[out] version The version of EPANET
  @return Error code.

  The version number is to be interpreted with implied decimals, i.e.,
  "20100" == "2(.)01(.)00"
  */
  int  DLLEXPORT ENgetversion(int *version);

  /**
  @brief Retrieves the number of components of a given type in the network.
  @param code Component code (see EPANET2.H)
  @param[out] count Number of components in network
  @return Error code
  */
  int  DLLEXPORT ENgetcount(int code, int *count);

  /**
  @brief Get the text of an error code.
  @param errcode The error code
  @param[out] errmsg The error string represented by the code
  @param maxLen The maximum number of characters to copy into the char pointer errmsg
  @return Error code
  */
  int  DLLEXPORT ENgeterror(int errcode, char *errmsg, int maxLen);

  /**
  @brief Get hydraulic simulation statistic
  @param code The type of statistic to get
  @param[out] value The value of the statistic
  @return Error code
  */
  int  DLLEXPORT ENgetstatistic(int code, EN_API_FLOAT_TYPE* value);

/********************************************************************

    Analysis Options Functions

********************************************************************/

  /**
  @brief Gets value for an analysis option
  @param code Option code (see EPANET2.H)
  @param[out] value Option value
  @return Error code
  */
  int  DLLEXPORT ENgetoption(int code, EN_API_FLOAT_TYPE *value);

  /**
  @brief Set a value for an anlysis option.
  @param code The code for the desired option.
  @param v The desired value for the option specified.
  @return Error code.
  @see EN_Option
  */
  int  DLLEXPORT ENsetoption(int code, EN_API_FLOAT_TYPE v);

  /**
  @brief Retrieves the flow units code
  @param[out] code Code of flow units in use
  @return Error code
  */
  int  DLLEXPORT ENgetflowunits(int *code);

  /**
  @brief Sets the flow units
  @param code Code of flow units to use
  @return Error code
  */
  int  DLLEXPORT ENsetflowunits(int code);

  /**
  @brief Retrieves value of specific time parameter.
  @param code Time parameter code
  @param[out] value Value of time parameter.
  @return Error code
  */
  int  DLLEXPORT ENgettimeparam(int code, long *value);

  /**
  @brief Set the value for a time parameter.
  @param code The code for the parameter to set.
  @param value The desired value of the parameter.
  @return Error code.
  @see EN_TimeProperty
  */
  int  DLLEXPORT ENsettimeparam(int code, long value);

  /**
  @brief Get information about the type of water quality analysis requested.
  @param[out] qualcode Type of analysis to be made (see EN_QualityType).
  @param[out] chemname Name of the quality constituent.
  @param[out] chemunits Concentration units of the constituent.
  @param[out] tracenode ID of node being traced (if applicable).
  @return Error code.
  */
  int  DLLEXPORT ENgetqualinfo(int *qualcode, char *chemname, char *chemunits,
      int *tracenode);

  /**
  @brief Retrieve the type of quality analytis to be run.
  @param[out] qualcode The quality analysis code number.
  @param[out] tracenode The index of node being traced, if qualcode == trace
  @return Error code
  @see ENsetqualtype
  */
  int  DLLEXPORT ENgetqualtype(int *qualcode, int *tracenode);

  /**
  @brief Set the type of quality analysis called for.
  @param qualcode Type of analysis to be made (see EN_QualityType).
  @param chemname Name of the quality constituent.
  @param chemunits Concentration units of the constituent.
  @param tracenode ID of node being traced (if applicable).
  @return Error code.

  Note: "chemname" and "chemunits" only apply when "qualcode" is EN_CHEM.
  "tracenode" only applies when 'qualcode" is EN_TRACE.
  */
  int  DLLEXPORT ENsetqualtype(int qualcode, char *chemname, char *chemunits,
      char *tracenode);

/********************************************************************

    Node Functions

********************************************************************/

  /**
  @brief Add a new node to the project.
  @param id The name of the node to be added.
  @param nodeType The type of node being added (see EN_NodeType)
  @return Error code.
  */
  int DLLEXPORT ENaddnode(char *id, EN_NodeType nodeType);

  /**
  @brief Delete a node from the project.
  @param index The index of the node to be deleted.
  @param actionCode The action taken if any control contains the node and its links.
  @return Error code.

  If 'actionCode' is EN_UNCONDITIONAL then the node, its incident links and all
  simple and rule-based controls that contain them are deleted. If set to
  EN_CONDITIONAL then the node is not deleted if it or its incident links appear
  in any controls and an error code is returned.

  */
  int DLLEXPORT ENdeletenode(int index, int actionCode);

  /**
  @brief Get index of node with specified ID
  @param id The string ID of the node
  @param[out] index The node's index (first node is index 1)
  @return Error code
  @see ENgetnodeid
  */
  int  DLLEXPORT ENgetnodeindex(char *id, int *index);

  /**
  @brief Get the string ID of the specified node.
  @param index The index of the node (first node is index 1)
  @param[out] id The string ID of the specified node.
  @return Error code
  @see ENgetnodeindex

  The ID string must be sized to hold at least MAXID characters.
  */
  int  DLLEXPORT ENgetnodeid(int index, char *id);

  /**
  @brief Change the ID name for a node.
  @param index The index of a node. First node is index 1.
  @param newid A string containing the node's new ID name.
  @return Error code.
  */
  int DLLEXPORT ENsetnodeid(int index, char *newid);

  /**
  @brief Get the type of node with specified index.
  @param index The index of a node (first node is index 1)
  @param[out] code The type code for the node (see the EN_NodeType enumeration)
  @return Error code
  */
  int  DLLEXPORT ENgetnodetype(int index, int *code);

  /**
  @brief Get a property value for specified node
  @param index The index of a node (first node is index 1).
  @param code The property type code
  @param[out] value The value of the node's property.
  @return Error code
  @see EN_NodeProperty
  */
  int  DLLEXPORT ENgetnodevalue(int index, int code, EN_API_FLOAT_TYPE *value);

  /**
  @brief Set a property value for a node.
  @param index The index of a node. First node is index 1.
  @param code The code for the proprty to set.
  @param v The value to set for this node and property.
  @return Error code.
  @see EN_NodeProperty
  */
  int  DLLEXPORT ENsetnodevalue(int index, int code, EN_API_FLOAT_TYPE v);

  /**
  @brief Get coordinates (x,y) for a node.
  @param index The index of a node (first node is index 1).
  @param[out] x X-value of node's coordinate
  @param[out] y Y-value of node's coordinate
  @return Error code
  @see ENsetcoord
  */
  int  DLLEXPORT ENgetcoord(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);

  /**
  @brief Set coordinates (x,y) for a node.
  @param index The index of a node (first node is index 1)
  @param x X-value of node's coordinate
  @param y Y-value of node's coordinate
  @return Error code
  @see ENgetcoord
  */
  int  DLLEXPORT ENsetcoord(int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);

/********************************************************************

    Nodal Demand Functions

********************************************************************/

  /**
  @brief Retrieves the type of demand model in use and its parameters
  @param[out] type  Type of demand model (EN_DDA or EN_PDA)
  @param[out] pmin  Pressure below which there is no demand
  @param[out] preq  Pressure required to deliver full demand
  @param[out] pexp  Pressure exponent in demand function
  @return Error code
  */
  int DLLEXPORT ENgetdemandmodel(int *type, EN_API_FLOAT_TYPE *pmin,
      EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp);

  /**
  @brief Sets the type of demand model to use and its parameters
  @param type  Type of demand model (EN_DDA or EN_PDA)
  @param pmin  Pressure below which there is no demand
  @param preq  Pressure required to deliver full demand
  @param pexp  Pressure exponent in demand function
  @return Error code
  */
  int DLLEXPORT ENsetdemandmodel(int type, EN_API_FLOAT_TYPE pmin,
      EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp);

  /**
  @brief Get the number of demand categories for a node.
  @param nodeIndex The index of a node (first node is index 1)
  @param[out] numDemands The number of demand categories
  @return Error code
  */
  int  DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands);

  /**
  @brief Get a node's base demand for a specified category.
  @param nodeIndex The index of a node (first node is index 1)
  @param demandIndex The index of the demand category (starting at 1)
  @return Error code
  */
  int  DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIndex,
      EN_API_FLOAT_TYPE *baseDemand);

  /**
  @brief Set a node's base demand for a demand category.
  @param nodeIndex The node's index.
  @param demandIndex The index of one of the node's demand categories.
  @param baseDemand The base demand for the selected category.
  @return Error code.
  @see ENgetbasedemand
  */
  int  DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIndex,
      EN_API_FLOAT_TYPE baseDemand);

  /**
  @brief Get the index of the demand pattern assigned to a node for a category index.
  @param nodeIndex The index of a node (first node is index 1).
  @param demandIndex The index of a category (first category is index 1).
  @param[out] pattIndex The index of the pattern for this node and category.
  @return Error code
  */
  int  DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIndex, int *pattIndex);

  /**
  @brief Set the time pattern assigned to a node's demand category.
  @param nodeIndex The node's index.
  @param demandIndex The index of one of the node's demand categories.
  @param pattIndex The index of a time pattern applied to this demand category.
  @return Error code
  */
  int  DLLEXPORT ENsetdemandpattern(int nodeIndex, int demandIndex, int patIndex);

  /**
  @brief Retrieve the name of a node's demand category.
  @param nodeIndex The node's index.
  @param demandIdx Index of the node's demand.
  @param demandName[out] Name of the category the demand belongs to.
  @return Error code.
  */
  int DLLEXPORT ENgetdemandname(int nodeIndex, int demandIdx, char *demandName);

  /**
  @brief Set the name of a node's demand category.
  @param nodeIndex The node's index.
  @param demandIdx Index of the node's demand.
  @param demandName Name for the category the demand belongs to.
  @return Error code.
  */
  int DLLEXPORT ENsetdemandname(int nodeIndex, int demandIdx, char *demandName);

/********************************************************************

    Link Functions

********************************************************************/

  /**
  @brief Add a new link to the project.
  @param id The name of the link to be added.
  @param linkType The type of link being added (see EN_LinkType)
  @param fromNode The id of the link's starting node
  @param toNode The id of the link's ending node
  @return Error code.
  */
  int DLLEXPORT ENaddlink(char *id, EN_LinkType linkType, char *fromNode, char *toNode);

  /**
  @brief Delete a link from the project.
  @param index The index of the link to be deleted.
  @param ctrlsCode The action taken if any control contains the link.
  @return Error code.

  If 'actionCode' is EN_UNCONDITIONAL then the link an all simple and rule-based
  controls that contain it are deleted. If set to EN_CONDITIONAL then the link
  is not deleted if it appears in any control and an error code is returned.

  */
  int DLLEXPORT ENdeletelink(int index, int actionCode);

  /**
  @brief Get the index of a Link with specified ID.
  @param id The string ID of a link.
  @param[out] index The index of the named link (first link is index 1)
  @return Error code
  @see ENgetlinkid
  */
  int  DLLEXPORT ENgetlinkindex(char *id, int *index);

  /**
  @brief Get the string ID of a link with specified index
  @param index The index of a link (first link is index 1)
  @param[out] id The ID of the link.
  @return Error code
  @see ENgetlinkindex

  The ID string must be sized to hold at least MAXID characters.
  */
  int  DLLEXPORT ENgetlinkid(int index, char *id);

  /**
  @brief Change the ID name for a link.
  @param index The index of a link. First link is index 1.
  @param newid A string containing the link's new ID name.
  @return Error code.
  */
  int DLLEXPORT ENsetlinkid(int index, char *newid);

  /**
  @brief Get the link type code for a specified link
  @param index The index of a link (first link is index 1)
  @param[out] code The type code of the link (see the EN_LinkType enumeration)
  @return Error code
  @see EN_LinkType
  */
  int  DLLEXPORT ENgetlinktype(int index, EN_LinkType *code);

  /**
  @brief Set the link type code for a specified link
  @param[in,out] index The index of a link before [in] and after [out] the type change
  @param code The new type code of the link (see EN_LinkType).
  @param actionCode Action taken if any controls contain the link.
  @return Error code
  @see the EN_LinkType enumeration

  If 'actionCode' is EN_UNCONDITIONAL then all simple and rule-based controls that
  contain the link are deleted when the link's type is changed. If set to
  EN_CONDITIONAL then the type change is cancelled if the link appears in any
  control and an error code is returned.
  */
  int  DLLEXPORT ENsetlinktype(int *index, EN_LinkType Code, int actionCode);

  /**
  @brief Get the indexes of a link's start- and end-nodes.
  @param index The index of a link (first link is index 1)
  @param[out] node1 The index of the link's start node (first node is index 1).
  @param[out] node2 The index of the link's end node (first node is index 1).
  @return Error code
  @see ENgetnodeid, ENgetlinkid
  */
  int  DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2);

  /**
  @brief Set the indexes of a link's start- and end-nodes.
  @param index The index of a link (first link is index 1)
  @param node1 The index of the link's start node (first node is index 1).
  @param node2 The index of the link's end node (first node is index 1).
  @return Error code
  @see ENsetnodeid, ENsetlinkid
  */
  int  DLLEXPORT ENsetlinknodes(int index, int node1, int node2);

  /**
  @brief Get a property value for specified link.
  @param index The index of a node (first node is index 1).
  @param code The parameter desired.
  @param[out] value The value of the link's specified property.
  @return Error code
  @see ENgetnodevalue, EN_LinkProperty
  */
  int  DLLEXPORT ENgetlinkvalue(int index, int code, EN_API_FLOAT_TYPE *value);

  /**
  @brief Set a property value for a link.
  @param index The index of a link. First link is index 1.
  @param code The code for the property to set.
  @param v The value to set for this link and property.
  @return Error code.
  @see EN_LinkProperty
  */
  int  DLLEXPORT ENsetlinkvalue(int index, int code, EN_API_FLOAT_TYPE v);

/********************************************************************

    Pump Functions

********************************************************************/

  /**
  @brief Get the type of pump
  @param linkIndex The index of the pump element
  @param[out] outType The integer-typed pump curve type signifier (output parameter)
  @return Error code
  @see EN_PumpType
  */
  int  DLLEXPORT ENgetpumptype(int linkIndex, int *outType);

  /**
  @brief Retrieves the curve index for a specified pump index.
  @param pumpIndex The index of a pump
  @param[out] curveIndex The index of the curve used by the pump.
  @return Error code.
  */
  int  DLLEXPORT ENgetheadcurveindex(int pumpIndex, int *curveIndex);

  /**
  @brief Sets the curve id for a specified pump index.
  @param pumpIndex The index of the pump
  @param curveIndex The index of the curve used by the pump
  @return Error code.
  */
  int  DLLEXPORT ENsetheadcurveindex(int pumpIndex, int curveIndex);

/********************************************************************

    Time Pattern Functions

********************************************************************/

  /**
  @brief Add a new time pattern.
  @param id The string ID of the pattern to add.
  @return Error code.
  @see ENgetpatternindex
  */
  int  DLLEXPORT ENaddpattern(char *id);

  /**
  @brief Retrieves the index of the time pattern with specified ID
  @param id String ID of the time pattern
  @param[out] index Index of the specified time pattern
  @return Error code
  @see ENgetpatternid
  */
  int  DLLEXPORT ENgetpatternindex(char *id, int *index);

  /**
  @brief Retrieves ID of a time pattern with specific index.
  @param index The index of a time pattern.
  @param[out] id The string ID of the time pattern.
  @return Error code
  @see ENgetpatternindex
  */
  int  DLLEXPORT ENgetpatternid(int index, char *id);

  /**
  @brief Retrieves the number of multipliers in a time pattern.
  @param index The index of a time pattern.
  @param[out] len The length of the time pattern.
  @return Error code
  */
  int  DLLEXPORT ENgetpatternlen(int index, int *len);

  /**
  @brief Retrive a multiplier from a pattern for a specific time period.
  @param index The index of a time pattern
  @param period The pattern time period. First time period is 1.
  @param[out] value Pattern multiplier
  @return Error code
  */
  int  DLLEXPORT ENgetpatternvalue(int index, int period, EN_API_FLOAT_TYPE *value);

  /**
  @brief Set the multiplier for a specific pattern at a specific period.
  @param index The index of a pattern. First pattern is index 1.
  @param period The period of the pattern to set.
  @param value The value of the multiplier to set.
  @return Error code.
  */
  int  DLLEXPORT ENsetpatternvalue(int index, int period, EN_API_FLOAT_TYPE value);

  /**
  @brief Retrieve the average multiplier value in a time pattern
  @param index The index of a time pattern
  @param[out] value The average of all of this time pattern's values
  @return Error code
  */
  int  DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value);

  /**
  @brief Set multipliers for a specific pattern
  @param index The index of a pattern. First pattern is index 1.
  @param f An array of multipliers
  @param len The length of array f.
  @return Error code.
  @see ENgetpatternindex
  */
  int  DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *f, int len);

/********************************************************************

    Data Curve Functions

********************************************************************/

  /**
  @brief Add a new curve to the project.
  @param id The name of the curve to be added.
  @return Error code.
  @see ENgetcurveindex ENsetcurve
  */
  int  DLLEXPORT ENaddcurve(char *id);

  /**
  @brief Retrieve the index of a curve given its name.
  @param id The ID name of a curve.
  @param[out] index The index of the named curve.
  @return Error code.
  @see ENgetcurveid
  */
  int  DLLEXPORT ENgetcurveindex(char *id, int *index);

  /**
  @brief Retrieve the ID name of a curve given its index.
  @param index The index of a curve.
  @param[out] id The ID of the specified curve.
  @return Error code.
  @see ENsetcurveindex

  NOTE: 'id' must be sized to hold MAXID characters.
  */
  int  DLLEXPORT ENgetcurveid(int index, char *id);

  /**
  @brief Retrieve the number of points in a curve.
  @param index The index of a curve.
  @param[out] len The number of data points assigned to the curve.
  @return Error code.
  @see ENgetcurvevalue
  */
  int  DLLEXPORT ENgetcurvelen(int index, int *len);

  /**
  @brief Get the type of a curve
  @param curveIndex The index of the curve element
  @param[out] outType The integer-typed curve curve type signifier (output parameter)
  @return Error code
  @see EN_CurveType
  */
  int  DLLEXPORT ENgetcurvetype(int curveIndex, int *outType);

  /**
  @brief Retrieve an x,y data point for a curve.
  @param curveIndex The index of a curve.
  @param pointIndex The index of a point in the curve.
  @param[out] x The x-value of the specified point.
  @param[out] y The y-value of the specified point.
  @return Error code.
  @see ENgetcurvelen ENsetcurvevalue
  */
  int  DLLEXPORT ENgetcurvevalue(int curveIndex, int pointIndex,
                 EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);

  /**
  @brief Set the x and y values for a curve's data point.
  @param curveIndex The index of a curve.
  @param pointIndex The index of a point in the curve.
  @param x The x-value of the point.
  @param y The y-value of the point.
  @return Error code.
  */
  int  DLLEXPORT ENsetcurvevalue(int curveIndex, int pointIndex,
                 EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);

  /**
  @brief Get a curve's properties.
  @param curveIndex The index of a curve (first curve is index 1).
  @param[out] id The curve's string ID. Client code must preallocate at least MAXID characters.
  @param[out] nValues The number of values in the curve's (x,y) list.
  @param[out] xValues The curve's x-values. Pointer must be freed by client.
  @param[out] yValues The curve's y-values. Pointer must be freed by client.
  @return Error code.

  The calling program is responsible for making xValues and yValues large enough
  to hold nValues number of data points.
  */
  int  DLLEXPORT ENgetcurve(int curveIndex, char* id, int *nValues,
                 EN_API_FLOAT_TYPE **xValues, EN_API_FLOAT_TYPE **yValues);

  /**
  @brief Set the x,y values for all points on a curve.
  @param index The index of a curve.
  @param x An array of x-values for the curve.
  @param y An array of y-values for the curve.
  @param len The length of the arrays for x and y.
  @return Error code.
  */
  int  DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *x,
                 EN_API_FLOAT_TYPE *y, int len);

/********************************************************************

    Simple Controls Functions

********************************************************************/

  /**
   @brief Add a new simple control to the project.
   @param[out] index The index of the new control. First control is index 1.
   @param ctype The type of control to add (see EN_ControlType).
   @param lindex The index of a link to control.
   @param setting The control setting applied to the link.
   @param nindex The index of a node used to control the link, or 0 for TIMER / TIMEOFDAY control.
   @param level control point (tank level, junction pressure, or time in seconds).
   @return Error code.
   */
  int  DLLEXPORT ENaddcontrol(int *index, int ctype, int lindex, EN_API_FLOAT_TYPE setting,
                              int nindex, EN_API_FLOAT_TYPE level);

  /**
   @brief Delete an existing simple control
   @param index The index of the control. First control is index 1.
   @return Error code.
  */
  int  DLLEXPORT ENdeletecontrol(int index);

  /**
   @brief Retrieves properties that define a simple control
   @param index Position of control in list of controls added to the project
   @param[out] ctype Control type code (see EN_ControlType enumeration)
   @param[out] lindex Index of controlled link
   @param[out] setting Control setting on link
   @param[out] nindex Index of controlling node (0 for TIMER or TIMEOFDAY control)
   @param[out] level Control level (tank level, junction pressure, or time (seconds))
   @return Error code
   */
  int  DLLEXPORT ENgetcontrol(int index, int *ctype, int *lindex, EN_API_FLOAT_TYPE *setting,
                              int *nindex, EN_API_FLOAT_TYPE *level);

  /**
   @brief Set the properties of an existing simple control.
   @param cindex The index of the control. First control is index 1.
   @param ctype The type of control to use (see EN_ControlType).
   @param lindex The index of a link to control.
   @param setting The control setting applied to the link.
   @param nindex The index of a node used to control the link, or 0 for TIMER / TIMEOFDAY control.
   @param level control point (tank level, junction pressure, or time in seconds).
   @return Error code.
   */
  int  DLLEXPORT ENsetcontrol(int cindex, int ctype, int lindex,
                 EN_API_FLOAT_TYPE setting, int nindex, EN_API_FLOAT_TYPE level);


/********************************************************************

    Rule-Based Controls Functions

********************************************************************/

  /**
  @brief Add a new control rule to the project.
  @param rule Text of the rule following the format used in an EPANET input file.
  @return Error code.
  */
  int DLLEXPORT ENaddrule(char *rule);

  /**
  @brief Delete a rule-based control.
  @param index The rule's index.
  @return Error code.
  */
  int  DLLEXPORT ENdeleterule(int index);

  /**
   @brief Get summary information for a rule-based control.
   @param index The rule's index.
   @param[out] nPremises Number of premises in the rule's IF section.
   @param[out] nThenActions Number of actions in the rule's THEN section.
   @param nElseActions[out] Number of actions in the rule's ELSE section.
   @param priority[out] Rule's priority.
   @return Error code.
   */
  int  DLLEXPORT ENgetrule(int index, int *nPremises, int *nTrueActions,
                 int *nFalseActions, EN_API_FLOAT_TYPE *priority);

  /**
  @brief Get the ID name of a rule-based control.
  @param index The rule's index.
  @param id[out] The rule's ID name.
  @return Error code.
  */
  int  DLLEXPORT ENgetruleID(int index, char* id);

  /**
   @brief Get the properties of a premise in a rule-based control.
   @param ruleIndex The rule's index.
   @param premiseIndex The premise's index.
   @param logop[out] Logical operator (IF = 1, AND = 2, OR = 3) of the premise.
   @param object[out] Type of object the premise is looking at (see EN_RuleObject).
   @param objIndex[out] Index of the object (e.g. the index of the tank).
   @param variable[out] Index of the variable to be checked (see EN_RuleVariable).
   @param relop[out] Relationship operator in the premise (see EN_RuleOperator).
   @param status[out] Status of the object being checked (see EN_RuleStatus).
   @param value[out] Setting of the variable being checked (e.g. 5.5)
   @return Error code.
   */
  int  DLLEXPORT ENgetpremise(int ruleIndex, int premiseIndex, int *logop,
                 int *object, int *objIndex, int *variable,
                 int *relop, int *status, EN_API_FLOAT_TYPE *value);

  /**
   @brief Set the properties of a premise in a rule-based control.
   @param ruleIndex The rule's index.
   @param premiseIndex The premise's index.
   @param logop Logical operator (IF = 1, AND = 2, OR = 3) of the premise.
   @param object Type of object the premise is looking at (see EN_RuleObject).
   @param objIndex Index of the object (e.g. the index of the tank).
   @param variable Index of the variable to be checked (see EN_RuleVariable).
   @param relop Relationship operator in the premise (see EN_RuleOperator).
   @param status Status of the object being checked (see EN_RuleStatus).
   @param value Setting of the variable being checked (e.g. 5.5)
   @return Error code.
   */
  int  DLLEXPORT ENsetpremise(int ruleIndex, int premiseIndex, int logop,
                 int object, int objIndex, int variable, int relop,
                 int status, EN_API_FLOAT_TYPE value);

  /**
   @brief Set the index of an object in a premise of a rule-based control.
   @param ruleIndex The rule's index.
   @param premiseIndex The premise's index.
   @param objIndex The index of the premise's object (e.g. the index of the tank).
   @return Error code.
   */
  int  DLLEXPORT ENsetpremiseindex(int ruleIndex, int premiseIndex, int objIndex);

  /**
   @brief Set the status in a premise of a rule-based control.
   @param ruleIndex The rule's index.
   @param premiseIndex The premise's index.
   @param status The status of the object being checked (see EN_RuleStatus)
   @return Error code.
   */
  int  DLLEXPORT ENsetpremisestatus(int ruleIndex, int premiseIndex, int status);

  /**
   @brief Set the value in a premise of a rule-based control.
   @param ruleIndex The rule's index.
   @param premiseIndex The premise's index.
   @param value The value of the premise's variable being checked (e.g. 5.5)
   @return Error code.
   */
  int  DLLEXPORT ENsetpremisevalue(int ruleIndex, int premiseIndex,
                 EN_API_FLOAT_TYPE value);

  /**
   @brief Get the properties of a THEN action in a rule-based control.
   @param ruleIndex The rule's index.
   @param actionIndex Index of the THEN action to retrieve.
   @param linkIndex[out] Index of the link in the action (e.g. index of Pump 1)
   @param status[out] Status of the link (see EN_RuleStatus)
   @param setting[out] Value of the link's setting (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENgetthenaction(int ruleIndex, int actionIndex, int *linkIndex,
                 int *status, EN_API_FLOAT_TYPE *setting);

  /**
   @brief Set the properties of a THEN action in a rule-based control.
   @param ruleIndex The rule's index.
   @param actionIndex Index of the THEN action to modify.
   @param linkIndex Index of the link in the action (e.g. index of Pump 1)
   @param status Status assigned to the link (e.g. CLOSED)
   @param setting Setting value assigned to the link (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENsetthenaction(int ruleIndex, int actionIndex, int linkIndex,
                 int status, EN_API_FLOAT_TYPE setting);

  /**
   @brief Get the properties of an ELSE action in a rule-based control.
   @param ruleIndex The rule's index.
   @param actionIndex Index of the ELSE action to retrieve.
   @param linkIndex[out] Index of the link in the action (e.g. index of Pump 1).
   @param status[out] Status of the link (see EN_RuleStatus).
   @param setting[out] Value of the link's setting (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENgetelseaction(int ruleIndex, int actionIndex, int *linkIndex,
                 int *status, EN_API_FLOAT_TYPE *setting);

  /**
   @brief Set the properties of an ELSE action in a rule-based control.
   @param ruleIndex The rule's index.
   @param actionIndex Index of the ELSE action being modified.
   @param linkIndex Index of the link in the action (e.g. index of Pump 1)
   @param status Status assigned to the link (see EN_RuleStatus)
   @param setting Setting value assigned to the link (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENsetelseaction(int ruleIndex, int actionIndex, int linkIndex,
                 int status, EN_API_FLOAT_TYPE setting);

  /**
  @brief Set the priority of a rule-based control.
  @param index The rule's index.
  @param priority The priority assigned to the rule.
  @return Error code.
  */
  int  DLLEXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority);


  #if defined(__cplusplus)
  }
  #endif

#endif //EPANET2_H
