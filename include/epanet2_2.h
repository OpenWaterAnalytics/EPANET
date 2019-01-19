/** @file epanet2_2.h
 @see http://github.com/openwateranalytics/epanet
 */

/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2_2.h
 Description:  declarations of the new thread-safe EPANET 2 API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 01/09/2019
 ******************************************************************************
 */

#ifndef EPANET2_2_H
#define EPANET2_2_H

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

/**
 @brief The EPANET Project wrapper object
*/
typedef struct Project *EN_Project;

/********************************************************************

    System Functions

********************************************************************/

/**
  @brief Creates an EPANET project.
  @param[out] ph an EPANET project handle that is passed into all other API functions.
  @return an error code.
  */
  int DLLEXPORT EN_createproject(EN_Project *ph);


  /**
  @brief Deletes a currently opened EPANET project.
  @param[in,out] ph an EPANET project handle which is returned as NULL.
  @return an error code.

  EN_deleteproject should be called after all network analysis has been completed.
  */
  int DLLEXPORT EN_deleteproject(EN_Project *ph);

  /**
  @brief Runs a complete EPANET simulation.
  @param ph an EPANET project handle.
  @param inpFile the name of an existing EPANET-formatted input file.
  @param rptFile the name of a report file to be created (or "" if not needed)
  @param outFile the name of a binary output file to be created (or "" if not needed)
  @param pviewprog a callback function that takes a character string (char *) as its only parameter.
  @return an error code

  The callback function should reside in and be used by the calling code to display
  the progress messages that EPANET generates as it carries out its computations. Here is
  an example of a such a function that displays progress messages to stdout:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void  writeConsole(char *s)
  {
      fprintf(stdout, "\n%s", s);
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  It would be passed into EN_runproject as &writeConsole. If this feature is not needed then
  the pviewprog argument should be NULL.
  */
  int DLLEXPORT EN_runproject(EN_Project ph, const char *inpFile, const char *rptFile,
                const char *outFile, void (*pviewprog)(char *));

  /**
  @brief Initializes an EPANET project.
  @param ph an EPANET project handle.
  @param rptFile the name of a report file to be created (or "" if not needed).
  @param outFile the name of a binary output file to be created (or "" if not needed).
  @param unitsType the choice of flow units (see ::EN_FlowUnits).
  @param headLossType the choice of head loss formula (see ::EN_HeadLossType).
  @return an error code.

  This function should be called immediately after EN_open if an EPANET-formatted input
  file will not be used to supply network data. If the project receives it's network data
  from an input file then there is no need to call this function.
  */
  int DLLEXPORT EN_init(EN_Project ph, const char *rptFile, const char *outFile,
                int unitsType, int headLossType);

  /**
  @brief Opens an EPANET input file & reads in network data.
  @param ph an EPANET project handle.
  @param inpFile the name of an existing EPANET-formatted input file.
  @param rptFile the name of a report file to be created (or "" if not needed).
  @param outFile the name of a binary output file to be created (or "" if not needed).
  @return an error code.
  */
  int DLLEXPORT EN_open(EN_Project ph, const char *inpFile, const char *rptFile,
                const char *outFile);


  /**
  @brief Saves a project's data to an EPANET-formatted text file.
  @param ph an EPANET project handle.
  @param filename the name of the file to create.
  @return Error code
  */
  int DLLEXPORT EN_saveinpfile(EN_Project ph, const char *filename);


  /**
  @brief Closes a project and frees all of its memory.
  @param ph an EPANET project handle.
  @return Error code

  This function clears all existing data from a project but does not delete the
  project, so it can be re-used with another set of network data. Use \ref EN_deleteProject
  to actually delete a project from memory.
  */
  int DLLEXPORT EN_close(EN_Project ph);

  /********************************************************************

  Hydraulic Analysis Functions

  ********************************************************************/

  /**
  @brief Runs a complete hydraulic simulation with results for all time periods
  written to a temporary hydraulics file.  
  @param ph an EPANET project handle.
  @return an error code.

  Use EN_solveH to generate a complete hydraulic solution which can stand alone
  or be used as input to a water quality analysis. This function will not allow one to 
  examine intermediate hydraulic results as they are generated. It can also be followed by calls
  to ::EN_saveH and ::EN_report to write hydraulic results to the report file.
  
  The sequence ::EN_openH - ::EN_initH - ::EN_runH - ::EN_nextH - ::EN_closeH 
  can be used instead to gain access to results at intermediate time periods and
  directly adjust link status and control settings as a simulation proceeds.
  */
  int DLLEXPORT EN_solveH(EN_Project ph);


  /**
  @brief Uses a previously saved binary hydraulics file to supply a project's hydraulics.
  @param ph an EPANET project handle.
  @param filename the name of the binary file containing hydraulic results.
  @return an error code.

  Call this function to re-use a set of hydraulic analysis results saved previously. This
  can save computational time if water quality analyses are being made under the same set
  of hydraulic conditions.

  Do not call this function while the hydraulics solver is open.
  */
  int DLLEXPORT EN_usehydfile(EN_Project ph, char *filename);


  /**
  @brief Opens a project's hydraulic solver.
  @param ph an EPANET project handle.
  @return an error code.

  Call EN_openH prior to running the first hydraulic analysis using the EN_initH - EN_runH - EN_nextH
  sequence. Multiple analyses can be made before calling ::EN_closeH to close the hydraulic solver.
  
  Do not call this function if ::EN_solveH is being used to run a complete hydraulic analysis or if
  hydraulics are being supplied by a previously saved hydraulics file using ::EN_usehydfile.
  */
  int DLLEXPORT EN_openH(EN_Project ph);


  /**
  @brief Initializes a network prior to running a hydraulic analysis.  
  @param ph an EPANET project handle.
  @param initFlag a 2-digit initialization flag (see ::EN_SaveOption).
  @return an error code.

  The initialization flag is a two digit number where the 1st (left) digit
  indicates if link flows should be re-initialized (1) or not (0), and the
  2nd digit indicates if hydraulic results should be saved to a temporary
  binary hydraulics file (1) or not (0).

  Be sure to call EN_initH prior to running a hydraulic analysis using a ::EN_runH - ::EN_nextH loop.

  Choose to save hydraulics results if you will be:
  - making a subsequent water quality run,
  - using ::EN_report to generate a report
  - or using ::EN_savehydfile to save the binary hydraulics file.
  There is no need to save hydraulics if you will be writing custom code to process hydraulic
  results as they are generated, using functions like ::EN_getnodevalue and ::EN_getlinkvalue.
  */
  int DLLEXPORT EN_initH(EN_Project ph, int initFlag);


  /**
  @brief Computes a hydraulic solution for the current point in time.
  @param ph an EPANET project handle.
  @param[out] currentTime the current simulation time in seconds.
  @return an error or warning code.

  This function is used in a loop with ::EN_nextH() to run an extended period hydraulic simulation.
  This process automatically updates the simulation clock time so currentTime should be treated as a
  read-only variable.

  ::EN_initH must have been called prior to running the EN_runH - EN_nextH loop.
  */
  int DLLEXPORT EN_runH(EN_Project ph, long *currentTime);


  /**
  @brief Determines the length of time until the next hydraulic event occurs in an extended period
  simulation.
  @param ph an EPANET project handle.
  @param[out] tStep the time (in seconds) until the next hydraulic event or 0 if at the end of the
  full simulation duration.
  @return an error code.

  This function is used in a loop with ::EN_runH() to run an extended period hydraulic simulation.

  The value of tstep should be treated as a read-only variable. It is automatically computed as
  the smaller of:
    - the time interval until the next hydraulic time step begins
    - the time interval until the next reporting time step begins
    - the time interval until the next change in demands occurs
    - the time interval until a tank becomes full or empty
    - the time interval until a control or rule fires.
  */
  int DLLEXPORT EN_nextH(EN_Project ph, long *tStep);

  /**
  @brief Transfers a project's hydraulics results from its temporary hydraulics file to its
  binary output file, where results are only reported at uniform reporting intervals. 
  @param ph an EPANET project handle.
  @return an error code.

  EN_saveH is used when only a hydraulic analysis is run and results at uniform reporting intervals
  need to be transferred to EPANET's binary output file. Such would be the case when an output
  report to EPANET's report file will be written using ::EN_report.
  */
  int DLLEXPORT EN_saveH(EN_Project ph);


  /**
  @brief Saves a project's temporary hydraulics file to disk.
  @param ph an EPANET project handle.
  @param filename the name of the file to be created.
  @return an error code.

  Use this function to save the current set of hydraulics results to a file, either for
  post-processing or to be used at a later time by calling the ::EN_usehydfile function.
  
  The hydraulics file contains nodal demands and heads and link flows, status, and settings
  for all hydraulic time steps, even intermediate ones.
  */
  int DLLEXPORT EN_savehydfile(EN_Project ph, char *filename);


  /**
  @brief Closes the hydraulic solver freeing all of its allocated memory.
  @return an error code.
  */
  int DLLEXPORT EN_closeH(EN_Project ph);

  /********************************************************************

  Water Quality Analysis Functions

  ********************************************************************/

  /**
  @brief Runs a complete water quality simulation with results at uniform
  reporting intervals written to the project's binary output file.  
  @param ph an EPANET project handle.
  @return an error code.
  
  A hydraulic analysis must have been run and saved to a hydraulics file before
  calling EN_solveQ. This function will not allow one to examine intermediate water
  quality results as they are generated. It can be followed by a call to ::EN_report
  to write all hydraulic and water quality results to a formatted report file.

  One can instead use the ::EN_openQ - ::EN_initQ - ::EN_runQ - ::EN_nextQ - ::EN_closeQ
  sequence to gain access to gain access to water quality results at intermediate time
  periods.
  */
  int DLLEXPORT EN_solveQ(EN_Project ph);


  /**
  @brief Opens a project's water quality solver.
  @param ph n EPANET project handle.
  @return an error code.

  Call EN_openQ prior to running the first water quality analysis using an
  ::EN_initQ - ::EN_runQ - ::EN_nextQ (or ::EN_stepQ) sequence. Multiple water
  quality analyses can be made before calling ::EN_closeQ to close the water
  quality solver.

  Do not call this function if a complete water quality analysis will be made
  using ::EN_solveQ.
  */
  int DLLEXPORT EN_openQ(EN_Project ph);


  /**
  @brief Initializes a network prior to running a water quality analysis. 
  @param ph n EPANET project handle.
  @param saveFlag set to EN_SAVE (1) if results are to be saved to the project's
  binary output file, or to EN_NOSAVE (0) if not.
  @return an error code.

  Call EN_initQ prior to running a water quality analysis using ::EN_runQ in conjunction with
  either ::EN_nextQ or ::EN_stepQ.
  
  ::EN_openQ must have been called prior to calling EN_initQ.
  
  Do not call EN_initQ if a complete water quality analysis will be made using ::EN_solveQ.
  */
  int DLLEXPORT EN_initQ(EN_Project ph, int saveFlag);


  /**
  @brief Makes hydraulic and water quality results at the start of the current time
  period available to a project's water quality solver. 
  @param ph an EPANET project handle.
  @param[out] currentTime current simulation time in seconds.
  @return an error code.

  Use EN_runQ along with ::EN_nextQ in a loop to access water quality results at the start
  of each hydraulic period in an extended period simulation. Or use it in a loop with ::EN_stepQ
  to access results at the start of each water quality time step. See each of these functions
  for examples of how to code such loops.
  
  ::EN_initQ must have been called prior to running an EN_runQ - EN_nextQ (or EN_stepQ) loop.
  
  The current time of the simulation is determined from information saved with the hydraulic
  analysis that preceded the water quality analysis. Treat it as a read-only variable.
  */
  int DLLEXPORT EN_runQ(EN_Project ph, long *currentTime);


  /**
  @brief Advances a water quality simulation over the time until the next hydraulic event.
  @param ph an EPANET project handle.
  @param[out] tStep time (in seconds) until the next hydraulic event or 0 if at the end of the
  full simulation duration.
  @return an error code.

  This function is used in a loop with ::EN_runQ to perform an extended period water quality analysis.
  It reacts and routes a project's water quality constituent over a time step determined by when
  the next hydraulic event occurs. Use ::EN_stepQ instead if you wish to generate results over each
  water quality time step.

  The value of tstep is determined from information produced by the hydraulic analysis that
  preceded the water quality analysis. Treat it as a read-only variable.
  */
  int DLLEXPORT EN_nextQ(EN_Project ph, long *tStep);


  /**
  @brief Advances a water quality simulation by a single water quality time step.
  @param ph an EPANET project handle.
  @param[out] timeLeft time left (in seconds) to the overall simulation duration.
  @return an error code.

  This function is used in a loop with ::EN_runQ to perform an extended period water quality
  simulation. It allows one to generate water quality results at each water quality time step
  of the simulation, rather than over each hydraulic event period as with ::EN_nextQ.
  
  Use the argument timeLeft to determine when no more calls to EN_runQ are needed because
  the end of the simulation period has been reached (i.e., when tleft = 0).
  */
  int DLLEXPORT EN_stepQ(EN_Project ph, long *timeLeft);


  /**
  @brief Closes the water quality solver, freeing all of its allocated memory.
  @param ph an EPANET project handle.
  @return an error code.
  */
  int DLLEXPORT EN_closeQ(EN_Project ph);

  /********************************************************************

  Reporting Functions

  ********************************************************************/

  /**
  @brief Writes a line of text to a project's report file.
  @param ph an EPANET project handle.
  @param line a text string to write.
  @return an error code.
  */
  int  DLLEXPORT EN_writeline(EN_Project ph, char *line);

  /**
  @brief Writes simulation results in a tabular format to a project's report file.
  @param ph an EPANET project handle.
  @return an error code
  */
  int  DLLEXPORT EN_report(EN_Project ph);

  /**
  @brief Resets a project's report options to their default values.
  @param ph an EPANET project handle.
  @return an error code
  */
  int  DLLEXPORT EN_resetreport(EN_Project ph);

  /**
  @brief Processes a reporting format command.
  @param ph an EPANET project handle.
  @param format a report formatting command.
  @return an error code
  
  Acceptable report formatting commands are described in Appendix C of the
  <a href="https://nepis.epa.gov/Adobe/PDF/P1007WWU.pdf">EPANET 2 Users Manual</a>.
  */
  int  DLLEXPORT EN_setreport(EN_Project ph, char *format);

  /**
  @brief Sets the level of hydraulic status reporting.
  @param ph an EPANET project handle.
  @param level a status reporting level code (see ::EN_StatusReport).
  @return an error code.
  */
  int  DLLEXPORT EN_setstatusreport(EN_Project ph, int level);

  /**
  @brief Retrieves the toolkit API version number.
  @param[out] version the version of the OWA-EPANET toolkit
  @return an error code.

  The version number is to be interpreted with implied decimals, i.e.,
  "20100" == "2(.)01(.)00"
  */
  int  DLLEXPORT EN_getversion(int *version);

  /**
  @brief Retrieves the number of objects of a given type in the network.
  @param ph an EPANET project handle.
  @param object a type of object to count (see ::EN_CountType)
  @param[out] count number of objects of the specified type
  @return an error code
  */
  int  DLLEXPORT EN_getcount(EN_Project ph, int object, int *count);

  /**
  @brief Returns the text of an error message generated by an error code.
  @param errcode an error code.
  @param[out] errmsg the error message generated by the error code
  @param maxLen maximum number of characters that errmsg can hold
  @return an error code
  */
  int  DLLEXPORT EN_geterror(int errcode, char *errmsg, int maxLen);

  /**
  @brief Retrieves a particular simulation statistic
  @param ph an EPANET project handle.
  @param type the type of statistic to retrieve (see ::EN_AnalysisStatistic).
  @param[out] value the value of the statistic.
  @return an error code
  */
  int  DLLEXPORT EN_getstatistic(EN_Project ph, int type, double* value);

  /********************************************************************

  Analysis Options Functions

  ********************************************************************/

  /**
  @brief Retrieves the value of an analysis option.
  @param ph an EPANET project handle.
  @param option a type of analysis option (see ::EN_Option).
  @param[out] value the current value of the option.
  @return an error code
  */
  int  DLLEXPORT EN_getoption(EN_Project ph, int option, double *value);

  /**
  @brief Sets the value for an anlysis option.
  @param ph an EPANET project handle.
  @param option a type of analysis option (see ::EN_Option).
  @param value the new value assigned to the option.
  @return an error code.
  @see EN_Option
  */
  int  DLLEXPORT EN_setoption(EN_Project ph, int option, double value);

  /**
  @brief Retrieves a project's flow units.
  @param ph an EPANET project handle.
  @param[out] units a flow units code (see ::EN_FlowUnits)
  @return an error code.
  */
  int  DLLEXPORT EN_getflowunits(EN_Project ph, int *units);

  /**
  @brief Sets a project's flow units.
  @param ph an EPANET project handle.
  @param units a flow units code (see ::EN_FlowUnits)
  @return an error code.
  */
  int  DLLEXPORT EN_setflowunits(EN_Project ph, int units);

  /**
  @brief Retrieves the value of a time parameter.
  @param ph an EPANET project handle.
  @param param a time parameter code (see ::EN_TimeProperty).
  @param[out] value the current value of the time parameter (in seconds).
  @return an error code.
  */
  int  DLLEXPORT EN_gettimeparam(EN_Project ph, int param, long *value);

  /**
  @brief Sets the value of a time parameter.
  @param ph an EPANET project handle.
  @param param a time parameter code (see ::EN_TimeProperty).
  @param value the new value of the time parameter (in seconds)
  @return an error code.
  */
  int  DLLEXPORT EN_settimeparam(EN_Project ph, int param, long value);

  /**
  @brief Gets information about the type of water quality analysis requested.
  @param ph an EPANET project handle.
  @param[out] qualType type of analysis to run (see ::EN_QualityType).
  @param[out] chemName name of chemical constituent.
  @param[out] chemUnits concentration units of the constituent.
  @param[out] traceNode index of the node being traced (if applicable).
  @return an error code.
  */
  int  DLLEXPORT EN_getqualinfo(EN_Project ph, int *qualType, char *chemName,
                 char *chemUnits, int *traceNode);

  /**
  @brief Retrieves the type of water quality analysis to be run.
  @param ph an EPANET project handle.
  @param[out] qualType the type of analysis to run (see ::EN_QualityType).
  @param[out] traceNode the index of node being traced, if qualType is EN_TRACE.
  @return an error code.
  */
  int  DLLEXPORT EN_getqualtype(EN_Project ph, int *qualType, int *traceNode);

  /**
  @brief Sets the type of water quality analysis to run.
  @param ph an EPANET project handle.
  @param qualType the type of analysis to run (see ::EN_QualityType).
  @param chemName the name of the quality constituent.
  @param chemUnits the concentration units of the constituent.
  @param traceNode the index of the node being traced if qualType is EN_TRACE.
  @return an error code.

  _chemName_ and _chemUnits_ only apply when _qualType_ is EN_CHEM.
  _traceNode_ only applies when _qualType_ is EN_TRACE. 
  */
  int  DLLEXPORT EN_setqualtype(EN_Project ph, int qualType, char *chemName,
                 char *chemUnits, char *traceNode);

  /********************************************************************

  Node Functions

  ********************************************************************/

  /**
  @brief Adds a new node to a project.
  @param ph an EPANET project handle.
  @param id the ID name of the node to be added.
  @param nodeType the type of node being added (see ::EN_NodeType)
  @return an error code.
  */
  int DLLEXPORT EN_addnode(EN_Project ph, char *id, int nodeType);

  /**
  @brief Deletes a node from a project.
  @param ph an EPANET project handle.
  @param index the index of the node to be deleted.
  @param actionCode the action taken if any control contains the node and its links.
  @return an error code.

  If _actionCode_ is EN_UNCONDITIONAL then the node, its incident links and all
  simple and rule-based controls that contain them are deleted. If set to
  EN_CONDITIONAL then the node is not deleted if it or its incident links appear
  in any controls and error code 261 is returned.

  */
  int DLLEXPORT EN_deletenode(EN_Project ph, int index, int actionCode);

  /**
  @brief Gets the index of node given its ID name.
  @param ph an EPANET project handle.
  @param id a node ID name.
  @param[out] index the node's index.
  @return an error code
  */
  int  DLLEXPORT EN_getnodeindex(EN_Project ph, char *id, int *index);

  /**
  @brief Gets the ID name of a node given its index.
  @param ph an EPANET project handle.
  @param index a node's index.
  @param[out] id the node's ID name.
  @return an error code

  The ID name must be sized to hold at least ::MAXID characters.
  */
  int  DLLEXPORT EN_getnodeid(EN_Project ph, int index, char *id);

  /**
  @brief Changes the ID name of a node.
  @param ph an EPANET project handle.
  @param index a node's index.
  @param newid the new ID name for the node.
  @return an error code.
  */
  int DLLEXPORT EN_setnodeid(EN_Project ph, int index, char *newid);

  /**
  @brief Retrieves a node's type given its index.
  @param ph an EPANET project handle.
  @param index a node's index.
  @param[out] nodeType the node's type (see ::EN_NodeType).
  @return an error code.
  */
  int  DLLEXPORT EN_getnodetype(EN_Project ph, int index, int *nodeType);

  /**
  @brief Retrieves a property value for a node.
  @param ph an EPANET project handle.
  @param index a node's index.
  @param property the property to retrieve (see ::EN_NodeProperty). 
  @param[out] value the current value of the property.
  @return an error code.
  */
  int  DLLEXPORT EN_getnodevalue(EN_Project ph, int index, int property, double *value);

  /**
  @brief Sets a property value for a node.
  @param ph an EPANET project handle.
  @param index a node's index.
  @param property the property to set (see ::EN_NodeProperty).
  @param value the new value for the property.
  @return an error code.
  */
  int  DLLEXPORT EN_setnodevalue(EN_Project ph, int index, int property, double value);

  /**
  @brief Sets a group of properties for a junction node.
  @param ph an EPANET project handle.
  @param index a junction node's index.
  @param elev the value of the junction's elevation.
  @param dmnd the value of the junction's primary base demand.
  @param dmndpat the name of the demand's time pattern ("" for no pattern)
  @return an error code.
  */
  int  DLLEXPORT EN_setjuncdata(EN_Project ph, int index, double elev, double dmnd,
                 char *dmndpat);

  /**
  @brief Sets a group of properties for a tank node.
  @param ph an EPANET project handle.
  @param index a tank node's index.
  @param elev the tank's bottom elevation.
  @param initlvl the initial water level in the tank.
  @param minlvl the minimum water level for the tank.
  @param maxlvl the maximum water level for the tank.
  @param diam the tank's diameter (0 if a volume curve is supplied).
  @param minvol the volume of the tank at its minimum water level.
  @param volcurve the name of the tank's volume curve ("" for no curve)
  @return an error code.
  */
  int  DLLEXPORT EN_settankdata(EN_Project ph, int index, double elev, double initlvl,
                 double minlvl, double maxlvl, double diam, double minvol, char *volcurve);

  /**
  @brief Gets the (x,y) coordinates of a node.
  @param ph an EPANET project handle.
  @param index a node index.
  @param[out] x the node's X-coordinate value.
  @param[out] y the node's Y-coordinate value.
  @return an error code.
  */
  int  DLLEXPORT EN_getcoord(EN_Project ph, int index, double *x, double *y);

  /**
  @brief Sets the (x,y) coordinates of a node.
  @param ph an EPANET project handle.
  @param index a node index.
  @param x the node's X-coordinate value.
  @param y the node's Y-coordinate value.
  @return an error code.
  */
  int  DLLEXPORT EN_setcoord(EN_Project ph, int index, double x, double y);

  /********************************************************************

  Nodal Demand Functions

  ********************************************************************/

  /**
  @brief Retrieves the type of demand model in use and its parameters.
  @param ph an EPANET project handle.
  @param[out] type  Type of demand model (see ::EN_DemandModel).
  @param[out] pmin  Pressure below which there is no demand.
  @param[out] preq  Pressure required to deliver full demand.
  @param[out] pexp  Pressure exponent in demand function.
  @return an error code.
  */
  int DLLEXPORT EN_getdemandmodel(EN_Project ph, int *type, double *pmin,
                double *preq, double *pexp);

  /**
  @brief Sets the type of demand model to use and its parameters.
  @param ph an EPANET project handle.
  @param type  Type of demand model (see ::EN_DemandModel).
  @param pmin  Pressure below which there is no demand.
  @param preq  Pressure required to deliver full demand.
  @param pexp  Pressure exponent in demand function.
  @return an error code.
  
  Set _type_ to __EN_DDA__ for a traditional demand driven analysis (in which case the
  remaining three parameter values are ignored) or to __EN_PDA__ for a pressure driven
  analysis. In the latter case a node's demand is computed as:
  >  Dfull * [ (P - pmin) / (preq - pmin) ] ^ pexp
  where Dfull is the full demand and P is the current pressure.
  
  Setting _preq_ equal to _pmin_ will result in a solution with the smallest amount of demand reductions needed to insure that no node delivers positive demand at a pressure below _pmin_.
  */
  int DLLEXPORT EN_setdemandmodel(EN_Project ph, int type, double pmin,
                double preq, double pexp);

  /**
  @brief Retrieves the number of demand categories for a node.
  @param ph an EPANET project handle.
  @param nodeIndex the index of a node.
  @param[out] numDemands the number of demand categories assigned to the node.
  @return an error code.
  */
  int  DLLEXPORT EN_getnumdemands(EN_Project ph, int nodeIndex, int *numDemands);

  /**
  @brief Gets the base demand for one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index.
  @param demandIndex the index of a demand category for the node.
  @param[out] baseDemand the category's base demand.
  @return an error code.
  */
  int  DLLEXPORT EN_getbasedemand(EN_Project ph, int nodeIndex, int demandIndex,
                 double *baseDemand);

  /**
  @brief Sets the base demand for one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index.
  @param demandIndex the index of a demand category for the node.
  @param baseDemand the new base demand for the category.
  @return an error code.
  */
  int  DLLEXPORT EN_setbasedemand(EN_Project ph, int nodeIndex, int demandIndex,
                 double baseDemand);

  /**
  @brief Retrieves the index of a time pattern assigned to one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex the node's index.
  @param demandIndex the index of a demand category for the node.
  @param[out] patIndex the index of the category's time pattern.
  @return an error code.
  */
  int  DLLEXPORT EN_getdemandpattern(EN_Project ph, int nodeIndex, int demandIndex,
       int *patIndex);

  /**
  @brief Sets the index of a time pattern used for one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index.
  @param demandIndex the index of one of the node's demand categories.
  @param patIndex the index of the time pattern assigned to the category.
  @return an error code.
  */
  int  DLLEXPORT EN_setdemandpattern(EN_Project ph, int nodeIndex, int demandIndex, int patIndex);

  /**
  @brief Retrieves the name of a node's demand category.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index.
  @param demandIndex the index of one of the node's demand categories.
  @param[out] demandName The name of the selected category.
  @return an error code.
  */
  int DLLEXPORT EN_getdemandname(EN_Project ph, int nodeIndex, int demandIndex, char *demandName);

  /**
  @brief Sets the name of a node's demand category.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index.
  @param demandIdx the index of one of the node's demand categories.
  @param demandName the new name assigned to the category.
  @return Error code.
  */
  int DLLEXPORT EN_setdemandname(EN_Project ph, int nodeIndex, int demandIdx, char *demandName);

  /********************************************************************

  Link Functions

  ********************************************************************/

  /**
  @brief Adds a new link to a project.
  @param ph an EPANET project handle.
  @param id the ID name of the link to be added.
  @param linkType The type of link being added (see ::EN_LinkType)
  @param fromNode The ID name of the link's starting node.
  @param toNode The ID name of the link's ending node.
  @return an error code.
  */
  int DLLEXPORT EN_addlink(EN_Project ph, char *id, int linkType, char *fromNode, char *toNode);

  /**
  @brief Deletes a link from the project.
  @param ph an EPANET project handle.
  @param index the index of the link to be deleted.
  @param actionCode The action taken if any control contains the link.
  @return an error code.

  If _actionCode_ is __EN_UNCONDITIONAL__ then the link and all simple and rule-based
  controls that contain it are deleted. If set to __EN_CONDITIONAL__ then the link
  is not deleted if it appears in any control and error 261 is returned.

  */
  int DLLEXPORT EN_deletelink(EN_Project ph, int index, int actionCode);

  /**
  @brief Gets the index of a link given its ID name.
  @param ph an EPANET project handle.
  @param id a link's ID name.
  @param[out] index the link's index.
  @return an error code.
  */
  int  DLLEXPORT EN_getlinkindex(EN_Project ph, char *id, int *index);

  /**
  @brief Gets the ID name of a link given its index.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param[out] id The link's ID name.
  @return an error code.

  The _id_ string must be sized to hold at least ::MAXID characters.
  */
  int  DLLEXPORT EN_getlinkid(EN_Project ph, int index, char *id);

  /**
  @brief Changes the ID name of a link.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param newid the new ID name for the link.
  @return Error code.
  */
  int DLLEXPORT EN_setlinkid(EN_Project ph, int index, char *newid);

  /**
  @brief Retrieves a link's type given its index.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param[out] linkType the link's type (see ::EN_LinkType).
  @return an error code.
  */
  int  DLLEXPORT EN_getlinktype(EN_Project ph, int index, int *linkType);

  /**
  @brief Changes a link's type.
  @param ph an EPANET project handle.
  @param[in,out] index the link's index before [in] and after [out] the type change.
  @param linkType the new type to change the link to (see ::EN_LinkType).
  @param actionCode the action taken if any controls contain the link.
  @return an error code.

  If _actionCode_ is __EN_UNCONDITIONAL__ then all simple and rule-based controls that
  contain the link are deleted when the link's type is changed. If set to
  __EN_CONDITIONAL__ then the type change is cancelled if the link appears in any
  control and error 261 is returned.
  */
  int  DLLEXPORT EN_setlinktype(EN_Project ph, int *index, int linkType, int actionCode);

  /**
  @brief Gets the indexes of a link's start- and end-nodes.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param[out] node1 the index of the link's start node.
  @param[out] node2 the index of the link's end node.
  @return an error code.
  */
  int  DLLEXPORT EN_getlinknodes(EN_Project ph, int index, int *node1, int *node2);

  /**
  @brief Sets the indexes of a link's start- and end-nodes.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param node1 The index of the link's start node.
  @param node2 The index of the link's end node.
  @return an error code.
  */
  int  DLLEXPORT EN_setlinknodes(EN_Project ph, int index, int node1, int node2);

  /**
  @brief Retrieves a property value for a link.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param property the property to retrieve (see ::EN_LinkProperty).
  @param[out] value the current value of the property.
  @return an error code.
  */
  int  DLLEXPORT EN_getlinkvalue(EN_Project ph, int index, int property, double *value);

  /**
  @brief Sets a property value for a link.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param property the property to set (see ::EN_LinkProperty).
  @param value the new value for the property.
  @return an error code.
  */
  int  DLLEXPORT EN_setlinkvalue(EN_Project ph, int index, int property, double value);

  /**
  @brief Sets a group of properties for a pipe link.
  @param ph an EPANET project handle.
  @param index the index of a pipe link.
  @param length the pipe's length.
  @param diam the pipe's diameter.
  @param rough the pipe's roughness coefficient.
  @param mloss the pipe's minor loss coefficient.
  @return an error code.
  */
  int DLLEXPORT EN_setpipedata(EN_Project ph, int index, double length, double diam,
                double rough,  double mloss);


  /********************************************************************

  Pump Functions

  ********************************************************************/

  /**
  @brief Retrieves the type of head curve used by a pump.
  @param ph an EPANET project handle.
  @param linkIndex the index of a pump link.
  @param[out] pumpType the type of head curve used by the pump (see ::EN_PumpType).
  @return an error code.
  */
  int  DLLEXPORT EN_getpumptype(EN_Project ph, int linkIndex, int *pumpType);

  /**
  @brief Retrieves the curve assigned to a pump's head curve.
  @param ph an EPANET project handle.
  @param linkIndex the index of a pump link.
  @param[out] curveIndex the index of the curve assigned to the pump's head curve.
  @return an error code.
  */
  int  DLLEXPORT EN_getheadcurveindex(EN_Project ph, int linkIndex, int *curveIndex);

  /**
  @brief Assigns a curve to a pump's head curve.
  @param ph an EPANET project handle.
  @param linkIndex the index of a pump link.
  @param curveIndex the index of a curve to be assigned as the pump's head curve.
  @return an error code.
  */
  int  DLLEXPORT EN_setheadcurveindex(EN_Project ph, int linkIndex, int curveIndex);

  /********************************************************************

  Time Pattern Functions

  ********************************************************************/

  /**
  @brief Adds a new time pattern to a project.
  @param ph an EPANET project handle.
  @param id the ID name of the pattern to add.
  @return an error code.
  */
  int  DLLEXPORT EN_addpattern(EN_Project ph, char *id);

  /**
  @brief Retrieves the index of a time pattern given its ID name.
  @param ph an EPANET project handle.
  @param id the ID name of a time pattern.
  @param[out] index the time pattern's index.
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternindex(EN_Project ph, char *id, int *index);

  /**
  @brief Retrieves ID name of a time pattern given its index.
  @param ph an EPANET project handle.
  @param index a time pattern index.
  @param[out] id the time pattern's ID name.
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternid(EN_Project ph, int index, char *id);

  /**
  @brief Retrieves the number time periods in a time pattern.
  @param ph an EPANET project handle.
  @param index a time pattern index.
  @param[out] len the number of time periods in the pattern.
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternlen(EN_Project ph, int index, int *len);

  /**
  @brief Retrieves a time pattern's factor for a given time period.
  @param ph an EPANET project handle.
  @param index a time pattern index.
  @param period a time period in the pattern.
  @param[out] value the pattern factor for the given time period.
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternvalue(EN_Project ph, int index, int period, double *value);

  /**
  @brief Sets a time pattern's factor for a given time period.
  @param ph an EPANET project handle.
  @param index a time pattern index.
  @param period a time period in the pattern.
  @param value the new value of the pattern factor for the given time period.
  @return an error code.
  */
  int  DLLEXPORT EN_setpatternvalue(EN_Project ph, int index, int period, double value);

  /**
  @brief Retrieves the average of all pattern factors in a time pattern.
  @param ph an EPANET project handle.
  @param index a time pattern index.
  @param[out] value The average of all of the time pattern's factors.
  @return an error code.
  */
  int  DLLEXPORT EN_getaveragepatternvalue(EN_Project ph, int index, double *value);

  /**
  @brief Sets the pattern factors for a given time pattern.
  @param ph an EPANET project handle.
  @param index a time pattern index.
  @param f an array of new pattern factors.
  @param len the number of time periods contained in the factor array f.
  @return an error code.
  */
  int  DLLEXPORT EN_setpattern(EN_Project ph, int index, double *f, int len);

  /********************************************************************

  Data Curve Functions

  ********************************************************************/

  /**
  @brief Adds a new data curve to a project.
  @param ph an EPANET project handle.
  @param id The ID name of the curve to be added.
  @return an error code.
  */
  int  DLLEXPORT EN_addcurve(EN_Project ph, char *id);

  /**
  @brief Retrieves the index of a curve given its ID name.
  @param ph an EPANET project handle.
  @param id the ID name of a curve.
  @param[out] index The curve's index.
  @return an error code.
  */
  int  DLLEXPORT EN_getcurveindex(EN_Project ph, char *id, int *index);

  /**
  @brief Retrieves the ID name of a curve given its index.
  @param ph an EPANET project handle.
  @param index a curve's index.
  @param[out] id the curve's ID name.
  @return an error code.
  */
  int  DLLEXPORT EN_getcurveid(EN_Project ph, int index, char *id);

  /**
  @brief Retrieves the number of points in a curve.
  @param ph an EPANET project handle.
  @param index a curve's index.
  @param[out] len The number of data points assigned to the curve.
  @return an error code.
  */
  int  DLLEXPORT EN_getcurvelen(EN_Project ph, int index, int *len);

  /**
  @brief Retrieves a curve's type.
  @param ph an EPANET project handle.
  @param index a curve's index.
  @param[out] type the curve's type (see EN_CurveType).
  @return an error code.
  */
  int  DLLEXPORT EN_getcurvetype(EN_Project ph, int index, int *type);

  /**
  @brief Retrieves the value of a single data point for a curve.
  @param ph an EPANET project handle.
  @param curveIndex a curve's index.
  @param pointIndex the index of a point on the curve.
  @param[out] x the point's x-value.
  @param[out] y the point's y-value.
  @return an error code.
  */
  int  DLLEXPORT EN_getcurvevalue(EN_Project ph, int curveIndex, int pointIndex,
                 double *x, double *y);

  /**
  @brief Sets the value of a single data point for a curve.
  @param ph an EPANET project handle.
  @param curveIndex a curve's index.
  @param pointIndex the index of a point on the curve.
  @param x the point's x-value.
  @param y the point's y-value.
  @return an error code.
  */
  int  DLLEXPORT EN_setcurvevalue(EN_Project ph, int curveIndex, int pointIndex,
                 double x, double y);

  /**
  @brief Retrieves a curve's data.
  @param ph an EPANET project handle.
  @param curveIndex a curve's index.
  @param[out] id the curve's ID name.
  @param[out] nPoints the number of data points on the curve.
  @param[out] xValues the curve's x-values.
  @param[out] yValues the curve's y-values.
  @return an error code.

  The calling program is responsible for making _xValues_ and _yValues_ large enough
  to hold nPoints number of data points and sizing _id_ to hold at least MAXID characters.
  */
  int  DLLEXPORT EN_getcurve(EN_Project ph, int curveIndex, char* id, int *nPoints,
                 double **xValues, double **yValues);

  /**
  @brief Sets a curve's data points.
  @param ph an EPANET project handle.
  @param index a curve's index.
  @param xValues an array of new x-values for the curve.
  @param yValues an array of new y-values for the curve.
  @param nPoints a new number of data points for the curve.
  @return an error code.
  */
  int  DLLEXPORT EN_setcurve(EN_Project ph, int index, double *xValues,
                 double *yValues, int nPoints);

  /********************************************************************

  Simple Controls Functions

  ********************************************************************/

  /**
  @brief Adds a new simple control to a project.
  @param ph an EPANET project handle.
  @param type the type of control to add (see EN_ControlType).
  @param linkIndex the index of a link to control.
  @param setting control setting applied to the link.
  @param nodeIndex index of the node used to control the link, or 0 for TIMER / TIMEOFDAY control.
  @param level action level (tank level, junction pressure, or time in seconds) that triggers the control.
  @param[out] index index of the new control.
  @return an error code.
  */
  int  DLLEXPORT EN_addcontrol(EN_Project ph, int type, int linkIndex,
                 double setting, int nodeIndex, double level, int *index);

  /**
  @brief Deletes an existing simple control.
  @param ph an EPANET project handle.
  @param index the index of the control to delete.
  @return an error code.
  */
  int  DLLEXPORT EN_deletecontrol(EN_Project ph, int index);

  /**
  @brief Retrieves the properties of a simple control.
  @param ph an EPANET project handle.
  @param index the control's index.
  @param[out] type the type of control (see EN_ControlType).
  @param[out] linkIndex the index of the link being controlled.
  @param[out] setting the control setting applied to the link.
  @param[out] nodeIndex the index of the node used to trigger the control (0 for TIMER /  TIMEOFDAY control).
  @param[out] level the action level (tank level, junction pressure, or time in seconds) that triggers the control.
  @return an error code.
  */
  int  DLLEXPORT EN_getcontrol(EN_Project ph, int index, int *type, int *linkIndex,
                 double *setting, int *nodeIndex, double *level);

  /**
  @brief Sets the properties of an existing simple control.
  @param ph an EPANET project handle.
  @param index the control's index.
  @param type the type of control (see EN_ControlType).
  @param linkIndex the index of the link being controlled.
  @param setting the control setting applied to the link.
  @param nodeIndex the index of the node used to trigger the control (0 for TIMER / TIMEOFDAY control).
  @param level the action level (tank level, junction pressure, or time in seconds) that triggers the control.
  @return an error code.
  */
  int  DLLEXPORT EN_setcontrol(EN_Project ph, int index, int type, int linkIndex,
                 double setting, int nodeIndex, double level);


  /********************************************************************

  Rule-Based Controls Functions

  ********************************************************************/

  /**
  @brief Adds a new rule-based control to a project.
  @param ph an EPANET project handle.
  @param rule text of the rule following the format used in an EPANET input file.
  @return an error code.
  
  Consult Appendix C of the <a href="https://nepis.epa.gov/Adobe/PDF/P1007WWU.pdf">EPANET 2 Users Manual</a> to learn about a rule's format.
  */
  int DLLEXPORT EN_addrule(EN_Project ph, char *rule);

  /**
  @brief Deletes an existing rule-based control.
  @param ph an EPANET project handle.
  @param index the index of the rule to be deleted.
  @return an error code.
  */
  int  DLLEXPORT EN_deleterule(EN_Project ph, int index);

  /**
  @brief Retrieves summary information about a rule-based control.
  @param ph an EPANET project handle.
  @param index the rule's index.
  @param[out] nPremises number of premises in the rule's IF section.
  @param[out] nThenActions number of actions in the rule's THEN section.
  @param[out] nElseActions number of actions in the rule's ELSE section.
  @param[out] priority the rule's priority.
  @return an error code.
  */
  int  DLLEXPORT EN_getrule(EN_Project ph, int index, int *nPremises,
                 int *nThenActions, int *nElseActions, double *priority);

  /**
  @brief Gets the ID name of a rule-based control given its index.
  @param ph an EPANET project handle.
  @param index the rule's index.
  @param[out] id the rule's ID name.
  @return Error code.
  */
  int  DLLEXPORT EN_getruleID(EN_Project ph, int index, char* id);

  /**
  @brief Gets the properties of a premise in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param premiseIndex the position of the premise in the rule's list of premises.
  @param[out] logop the premise's logical operator (IF = 1, AND = 2, OR = 3).
  @param[out] object the type of object the premise refers to (see EN_RuleObject).
  @param[out] objIndex the index of the object (e.g. the index of the tank).
  @param[out] variable the object's variable being compared (see EN_RuleVariable).
  @param[out] relop the premise's comparison operator (see EN_RuleOperator).
  @param[out] status the status that the object's status is compared to (see EN_RuleStatus).
  @param[out] value the value that the object's variable is compared to.
  @return an error code.
  */
  int  DLLEXPORT EN_getpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                 int *logop,  int *object, int *objIndex, int *variable,
                 int *relop, int *status, double *value);

  /**
  @brief Sets the properties of a premise in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param premiseIndex the position of the premise in the rule's list of premises.
  @param logop the premise's logical operator (IF = 1, AND = 2, OR = 3).
  @param object the type of object the premise refers to (see EN_RuleObject).
  @param objIndex the index of the object (e.g. the index of the tank).
  @param variable the object's variable being compared (see EN_RuleVariable).
  @param relop the premise's comparison operator (see EN_RuleOperator).
  @param status the status that the object's status is compared to (see ::EN_RuleStatus).
  @param value the value that the object's variable is compared to.
  @return an error code.
  */
  int  DLLEXPORT EN_setpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                 int logop, int object, int objIndex, int variable, int relop,
                 int status, double value);

  /**
  @brief Sets the index of an object in a premise of a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param premiseIndex the premise's index.
  @param objIndex the index of the premise's object (e.g. the index of the tank).
  @return an error code.
  */
  int  DLLEXPORT EN_setpremiseindex(EN_Project ph, int ruleIndex, int premiseIndex,
                 int objIndex);

  /**
  @brief Sets the status being compared to in a premise of a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param premiseIndex the premise's index.
  @param status the status that the premise's object status is compared to (see ::EN_RuleStatus).
  @return an error code.
  */
  int  DLLEXPORT EN_setpremisestatus(EN_Project ph, int ruleIndex, int premiseIndex,
                 int status);

  /**
  @brief Sets the value in a premise of a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param premiseIndex the premise's index.
  @param value The value that the premise's variable is compared to.
  @return an error code.
  */
  int  DLLEXPORT EN_setpremisevalue(EN_Project ph, int ruleIndex, int premiseIndex,
                 double value);

  /**
  @brief Gets the properties of a THEN action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param actionIndex the index of the THEN action to retrieve.
  @param[out] linkIndex the index of the link in the action.
  @param[out] status the status assigned to the link (see ::EN_RuleStatus)
  @param[out] setting the value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_getthenaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int *linkIndex, int *status, double *setting);

  /**
  @brief Sets the properties of a THEN action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param actionIndex the index of the THEN action to modify.
  @param[out] linkIndex the index of the link in the action.
  @param[out] status the new status assigned to the link (see ::EN_RuleStatus).
  @param[out] setting the new value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_setthenaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int linkIndex, int status, double setting);

  /**
  @brief Gets the properties of an ELSE action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param actionIndex the index of the ELSE action to retrieve.
  @param[out] linkIndex the index of the link in the action.
  @param[out] status the status assigned to the link (see ::EN_RuleStatus).
  @param[out] setting the value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_getelseaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int *linkIndex, int *status, double *setting);

  /**
  @brief Sets the properties of an ELSE action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index.
  @param actionIndex the index of the ELSE action being modified.
  @param[out] linkIndex the index of the link in the action.
  @param[out] status the new status assigned to the link (see ::EN_RuleStatus)
  @param[out] setting the new value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_setelseaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int linkIndex, int status, double setting);

  /**
  @brief Sets the priority of a rule-based control.
  @param ph an EPANET project handle.
  @param index the rule's index.
  @param priority the priority assigned to the rule.
  @return an error code.
  */
  int  DLLEXPORT EN_setrulepriority(EN_Project ph, int index, double priority);

#if defined(__cplusplus)
}
#endif

#endif //EPANET2_2_H
