/** @file epanet2_2.h
 @see http://github.com/openwateranalytics/epanet
 */

/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.h
 Description:  API function declarations
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 10/29/2019
 ******************************************************************************
 */

#ifndef EPANET2_2_H
#define EPANET2_2_H

#ifdef epanet_py_EXPORTS
  #define DLLEXPORT
#else
  #ifndef DLLEXPORT
    #ifdef _WIN32
      #ifdef epanet2_EXPORTS
        #define DLLEXPORT __declspec(dllexport) __stdcall
      #else
        #define DLLEXPORT __declspec(dllimport) __stdcall
      #endif
    #elif defined(__CYGWIN__)
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

    Project Functions

********************************************************************/

  /**
  @brief Creates an EPANET project.
  @param[out] ph an EPANET project handle that is passed into all other API functions.
  @return an error code.

  EN_createproject must be called before any other API functions are used.
  */
  int DLLEXPORT EN_createproject(EN_Project *ph);

  /**
  @brief Deletes a currently opened EPANET project.
  @param[in,out] ph an EPANET project handle which is returned as NULL.
  @return an error code.

  EN_deleteproject should be called after all network analysis has been completed.
  */
  int DLLEXPORT EN_deleteproject(EN_Project ph);

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
  \code {.c}
  void  writeConsole(char *s)
  {
      fprintf(stdout, "\n%s", s);
  }
 \endcode
  It would be passed into EN_runproject as `&writeConsole`. If this feature is not needed then
  the pviewprog argument should be `NULL`.
  */
  int DLLEXPORT EN_runproject(EN_Project ph, const char *inpFile, const char *rptFile,
                const char *outFile, void (*pviewprog)(char *));

  /**
  @brief Initializes an EPANET project.
  @param ph an EPANET project handle.
  @param rptFile the name of a report file to be created (or "" if not needed).
  @param outFile the name of a binary output file to be created (or "" if not needed).
  @param unitsType the choice of flow units (see @ref EN_FlowUnits).
  @param headLossType the choice of head loss formula (see @ref EN_HeadLossType).
  @return an error code.

  This function should be called immediately after ::EN_createproject if an EPANET-formatted input
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

  This function should be called immediately after ::EN_createproject if an EPANET-formatted
  input file will be used to supply network data.
  */
  int DLLEXPORT EN_open(EN_Project ph, const char *inpFile, const char *rptFile,
                const char *outFile);

  /**
  @brief Retrieves the title lines of the project
  @param ph an EPANET project handle.
  @param[out] out_line1 first title line
  @param[out] out_line2 second title line
  @param[out] out_line3 third title line
  @return an error code
  */
  int  DLLEXPORT EN_gettitle(EN_Project ph, char *out_line1, char *out_line2, char *out_line3);

  /**
  @brief Sets the title lines of the project
  @param ph an EPANET project handle.
  @param line1 first title line
  @param line2 second title line
  @param line3 third title line
  @return an error code
  */
  int  DLLEXPORT EN_settitle(EN_Project ph, char *line1, char *line2, char *line3);

  /**
  @brief Retrieves a descriptive comment assigned to a Node, Link, Pattern or Curve.
  @param ph an EPANET project handle.
  @param object a type of object (either EN_NODE, EN_LINK, EN_TIMEPAT or EN_CURVE)
  @param index the object's index starting from 1
  @param[out] out_comment the comment string assigned to the object
  @return an error code
  */
  int  DLLEXPORT EN_getcomment(EN_Project ph, int object, int index, char *out_comment);

  /**
  @brief Assigns a descriptive comment to a Node, Link, Pattern or Curve.
  @param ph an EPANET project handle.
  @param object a type of object (either EN_NODE, EN_LINK, EN_TIMEPAT or EN_CURVE)
  @param index the object's index starting from 1
  @param[out] comment the comment string assigned to the object
  @return an error code
  */
  int  DLLEXPORT EN_setcomment(EN_Project ph, int object, int index, char *comment);

  /**
  @brief Retrieves the number of objects of a given type in a project.
  @param ph an EPANET project handle.
  @param object a type of object to count (see @ref EN_CountType)
  @param[out] count number of objects of the specified type
  @return an error code
  */
  int  DLLEXPORT EN_getcount(EN_Project ph, int object, int *count);

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
  project, so it can be re-used with another set of network data. Use ::EN_deleteproject
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

  Use ::EN_solveH to generate a complete hydraulic solution which can stand alone
  or be used as input to a water quality analysis. This function will not allow one to
  examine intermediate hydraulic results as they are generated. It can also be followed by calls
  to ::EN_saveH and ::EN_report to write hydraulic results to the report file.

  The sequence ::EN_openH - ::EN_initH - ::EN_runH - ::EN_nextH - ::EN_closeH
  can be used instead to gain access to results at intermediate time periods and
  directly adjust link status and control settings as a simulation proceeds.

 <b>Example:</b>
  \code {.c}
  EN_Project ph;
  EN_createproject(&ph);
  EN_open(ph, "net1.inp", "net1.rpt", "");
  EN_solveH(ph);
  EN_solveQ(ph);
  EN_report(ph);
  EN_deleteproject(ph);
  \endcode
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
  int DLLEXPORT EN_usehydfile(EN_Project ph, const char *filename);

  /**
  @brief Opens a project's hydraulic solver.
  @param ph an EPANET project handle.
  @return an error code.

  Call ::EN_openH prior to running the first hydraulic analysis using the
  ::EN_initH - ::EN_runH - ::EN_nextH sequence. Multiple analyses can be made before
  calling ::EN_closeH to close the hydraulic solver.

  Do not call this function if ::EN_solveH is being used to run a complete hydraulic
  analysis or if hydraulics are being supplied by a previously saved hydraulics file
  using ::EN_usehydfile.
  */
  int DLLEXPORT EN_openH(EN_Project ph);

  /**
  @brief Initializes a network prior to running a hydraulic analysis.
  @param ph an EPANET project handle.
  @param initFlag a 2-digit initialization flag (see @ref EN_InitHydOption).
  @return an error code.

  This function initializes storage tank levels, link status and settings, and
  the simulation time clock prior to running a hydraulic analysis.

  The initialization flag is a two digit number where the 1st (left) digit
  indicates if link flows should be re-initialized (1) or not (0), and the
  2nd digit indicates if hydraulic results should be saved to a temporary
  binary hydraulics file (1) or not (0).

  Be sure to call ::EN_initH prior to running a hydraulic analysis using a
  ::EN_runH - ::EN_nextH loop.

  Choose to save hydraulics results if you will be:
  - making a subsequent water quality run,
  - using ::EN_report to generate a report
  - using ::EN_savehydfile to save the binary hydraulics file.

  There is no need to save hydraulics if you will be writing custom code to
  process hydraulic results as they are generated using the functions ::EN_getnodevalue
  and ::EN_getlinkvalue.
  */
  int DLLEXPORT EN_initH(EN_Project ph, int initFlag);

  /**
  @brief Computes a hydraulic solution for the current point in time.
  @param ph an EPANET project handle.
  @param[out] currentTime the current simulation time in seconds.
  @return an error or warning code.

  This function is used in a loop with ::EN_nextH to run an extended period hydraulic
  simulation. This process automatically updates the simulation clock time so \b currentTime
  should be treated as a read-only variable.

  ::EN_initH must have been called prior to running the ::EN_runH - ::EN_nextH loop.

  See ::EN_nextH for an example of using this function.
  */
  int DLLEXPORT EN_runH(EN_Project ph, long *currentTime);

  /**
  @brief Determines the length of time until the next hydraulic event occurs in an
  extended period simulation.
  @param ph an EPANET project handle.
  @param[out] tStep the time (in seconds) until the next hydraulic event or 0 if at
  the end of the full simulation duration.
  @return an error code.

  This function is used in a loop with ::EN_runH to run an extended period hydraulic
  simulation.

  The value of \b tstep should be treated as a read-only variable. It is automatically
  computed as the smaller of:
    - the time interval until the next hydraulic time step begins
    - the time interval until the next reporting time step begins
    - the time interval until the next change in demands occurs
    - the time interval until a tank becomes full or empty
    - the time interval until a control or rule fires.

 <B>Example:</B>
  \code {.c}
  long t, tstep;
  EN_openH(ph);
  EN_initH(ph, EN_NOSAVE);
  do {
    EN_runH(ph, &t);
    // Retrieve hydraulic results for time t
    EN_nextH(ph, &tstep);
  } while (tstep > 0);
  EN_closeH(ph);
  \endcode
  */
  int DLLEXPORT EN_nextH(EN_Project ph, long *tStep);

  /**
  @brief Transfers a project's hydraulics results from its temporary hydraulics file
  to its binary output file, where results are only reported at uniform reporting intervals.
  @param ph an EPANET project handle.
  @return an error code.

  ::EN_saveH is used when only a hydraulic analysis is run and results at uniform reporting
  intervals need to be transferred to a project's binary output file. Such would be the case
  when results are to be written in formatted fashion to the project's report file using ::EN_report.
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

  Before calling this function hydraulic results must have been generated and saved by having
  called ::EN_solveH or the ::EN_initH - ::EN_runH - ::EN_nextH sequence with the initflag
  argument of ::EN_initH set to \b EN_SAVE or \b EN_SAVE_AND_INIT.
  */
  int DLLEXPORT EN_savehydfile(EN_Project ph, const char *filename);

  /**
  @brief Closes the hydraulic solver freeing all of its allocated memory.
  @return an error code.

  Call ::EN_closeH after all hydraulics analyses have been made using
  ::EN_initH - ::EN_runH - ::EN_nextH. Do not call this function if ::EN_solveH is being used.
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
  calling ::EN_solveQ. This function will not allow one to examine intermediate water
  quality results as they are generated. It can be followed by a call to ::EN_report
  to write all hydraulic and water quality results to a formatted report file.

  One can instead use the ::EN_openQ - ::EN_initQ - ::EN_runQ - ::EN_nextQ - ::EN_closeQ
  sequence to gain access to gain access to water quality results at intermediate time
  periods.

   <b>Example:</b> see ::EN_solveH.
  */
  int DLLEXPORT EN_solveQ(EN_Project ph);

  /**
  @brief Opens a project's water quality solver.
  @param ph n EPANET project handle.
  @return an error code.

  Call ::EN_openQ prior to running the first water quality analysis using an
  ::EN_initQ - ::EN_runQ - ::EN_nextQ (or ::EN_stepQ) sequence. Multiple water
  quality analyses can be made before calling ::EN_closeQ to close the water
  quality solver.

  Do not call this function if a complete water quality analysis will be made
  using ::EN_solveQ.
  */
  int DLLEXPORT EN_openQ(EN_Project ph);

  /**
  @brief Initializes a network prior to running a water quality analysis.
  @param ph an EPANET project handle.
  @param saveFlag set to \b EN_SAVE (1) if results are to be saved to the project's
  binary output file, or to \b EN_NOSAVE (0) if not.
  @return an error code.

  Call ::EN_initQ prior to running a water quality analysis using ::EN_runQ in
  conjunction with either ::EN_nextQ or ::EN_stepQ.

  ::EN_openQ must have been called prior to calling ::EN_initQ.

  Do not call ::EN_initQ if a complete water quality analysis will be made using ::EN_solveQ.
  */
  int DLLEXPORT EN_initQ(EN_Project ph, int saveFlag);

  /**
  @brief Makes hydraulic and water quality results at the start of the current time
  period available to a project's water quality solver.
  @param ph an EPANET project handle.
  @param[out] currentTime current simulation time in seconds.
  @return an error code.

  Use ::EN_runQ along with ::EN_nextQ in a loop to access water quality results at the
  start of each hydraulic period in an extended period simulation. Or use it in a loop
  with ::EN_stepQ to access results at the start of each water quality time step. See
  each of these functions for examples of how to code such loops.

  ::EN_initQ must have been called prior to running an ::EN_runQ - ::EN_nextQ
  (or ::EN_stepQ) loop.

  The current time of the simulation is determined from information saved with the
  hydraulic analysis that preceded the water quality analysis. Treat it as a read-only
  variable.
  */
  int DLLEXPORT EN_runQ(EN_Project ph, long *currentTime);

  /**
  @brief Advances a water quality simulation over the time until the next hydraulic event.
  @param ph an EPANET project handle.
  @param[out] tStep time (in seconds) until the next hydraulic event or 0 if at the end
  of the full simulation duration.
  @return an error code.

  This function is used in a loop with ::EN_runQ to perform an extended period water
  quality analysis. It reacts and routes a project's water quality constituent over a
  time step determined by when the next hydraulic event occurs. Use ::EN_stepQ instead
  if you wish to generate results over each water quality time step.

  The value of \b tStep is determined from information produced by the hydraulic analysis
  that preceded the water quality analysis. Treat it as a read-only variable.

 <b>Example:</b>
  \code {.c}
  long t, tStep;
  EN_solveH(ph);  // Generate & save hydraulics
  EN_openQ(ph);
  EN_initQ(ph, EN_NOSAVE);
  do {
    EN_runQ(ph, &t);
    // Monitor results at time t, which
    // begins a new hydraulic time period
    EN_nextQ(ph, &tStep);
  } while (tStep > 0);
  EN_closeQ(ph);
  \endcode
  */
  int DLLEXPORT EN_nextQ(EN_Project ph, long *tStep);

  /**
  @brief Advances a water quality simulation by a single water quality time step.
  @param ph an EPANET project handle.
  @param[out] timeLeft time left (in seconds) to the overall simulation duration.
  @return an error code.

  This function is used in a loop with ::EN_runQ to perform an extended period water
  quality simulation. It allows one to generate water quality results at each water
  quality time step of the simulation, rather than over each hydraulic event period
  as with ::EN_nextQ.

  Use the argument \b timeLeft to determine when no more calls to ::EN_runQ are needed
  because the end of the simulation period has been reached (i.e., when \b timeLeft = 0).
  */
  int DLLEXPORT EN_stepQ(EN_Project ph, long *timeLeft);

  /**
  @brief Closes the water quality solver, freeing all of its allocated memory.
  @param ph an EPANET project handle.
  @return an error code.

  Call ::EN_closeQ after all water quality analyses have been made using the
  ::EN_initQ - ::EN_runQ - ::EN_nextQ (or ::EN_stepQ) sequence of function calls.

  Do not call this function if ::EN_solveQ is being used.
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

  Either a full hydraulic analysis or full hydraulic and water quality analysis must
  have been run, with results saved to file, before ::EN_report is called. In the
  former case, ::EN_saveH must also be called first to transfer results from the
  project's intermediate hydraulics file to its output file.

  The format of the report is controlled by commands issued with ::EN_setreport.
  */
  int  DLLEXPORT EN_report(EN_Project ph);

  /**
  @brief Copies the current contents of a project's report file to another file.
  @param ph an EPANET project handle.
  @param filename the full path name of the destination file.
  @return an error code.

  This function allows toolkit clients to retrieve the contents of a project's
  report file while the project is still open.
  */
  int  DLLEXPORT EN_copyreport(EN_Project ph, char *filename);

  /**
  @brief Clears the contents of a project's report file.
  @param ph an EPANET project handle.
  @return an error code.
  */
  int DLLEXPORT EN_clearreport(EN_Project ph);

  /**
  @brief Resets a project's report options to their default values.
  @param ph an EPANET project handle.
  @return an error code

  After calling this function the default reporting options are in effect. These are:
  - no status report
  - no energy report
  - no nodes reported on
  - no links reported on
  - node variables reported to 2 decimal places
  - link variables reported to 2 decimal places (3 for friction factor)
  - node variables reported are elevation, head, pressure, and quality
  - link variables reported are flow, velocity, and head loss.
  */
  int  DLLEXPORT EN_resetreport(EN_Project ph);

  /**
  @brief Processes a reporting format command.
  @param ph an EPANET project handle.
  @param format a report formatting command.
  @return an error code

  Acceptable report formatting commands are described in the @ref ReportPage section of
  the @ref InpFile topic.

  Formatted results of a simulation can be written to a project's report file
  using the ::EN_report function.
  */
  int  DLLEXPORT EN_setreport(EN_Project ph, char *format);

  /**
  @brief Sets the level of hydraulic status reporting.
  @param ph an EPANET project handle.
  @param level a status reporting level code (see @ref EN_StatusReport).
  @return an error code.

  Status reporting writes changes in the hydraulics status of network elements to a
  project's  report file as a hydraulic simulation unfolds. There are three levels
  of reporting: \b EN_NO_REPORT (no status reporting), \b EN_NORMAL_REPORT (normal
  reporting) \b EN_FULL_REPORT (full status reporting).

  The full status report contains information at each trial of the solution to the
  system hydraulic equations at each time step of a simulation. It is useful mainly
  for debugging purposes.

  If many hydraulic analyses will be run in the application it is recommended that
  status reporting be turned off (<b>level = EN_NO_REPORT</b>).
  */
  int  DLLEXPORT EN_setstatusreport(EN_Project ph, int level);

  /**
  @brief Retrieves the toolkit API version number.
  @param[out] version the version of the OWA-EPANET toolkit.
  @return an error code.

  The version number is to be interpreted with implied decimals, i.e.,
  "20100" == "2(.)01(.)00"
  */
  int  DLLEXPORT EN_getversion(int *version);

  /**
  @brief Returns the text of an error message generated by an error code.
  @param errcode an error code.
  @param[out] out_errmsg the error message generated by the error code
  @param maxLen maximum number of characters that errmsg can hold
  @return an error code

  Error message strings should be at least @ref EN_SizeLimits "EN_MAXMSG" characters in length.
  */
  int  DLLEXPORT EN_geterror(int errcode, char *out_errmsg, int maxLen);

  /**
  @brief Retrieves a particular simulation statistic.
  @param ph an EPANET project handle.
  @param type the type of statistic to retrieve (see @ref EN_AnalysisStatistic).
  @param[out] value the value of the statistic.
  @return an error code
  */
  int  DLLEXPORT EN_getstatistic(EN_Project ph, int type, double* value);

  /**
  @brief Retrieves the order in which a node or link appears in an @ref OutFile "output file".
  @param ph an EPANET project handle.
  @param type a type of element (either @ref EN_NODE or @ref EN_LINK).
  @param index the element's current index (starting from 1).
  @param[out] value the order in which the element's results were written to file.
  @return an error code.

  If the element does not appear in the file then its result index is 0.

  This function can be used to correctly retrieve results from an EPANET binary output file
  after the order of nodes or links in a network's database has been changed due to editing
  operations.
  */
  int  DLLEXPORT EN_getresultindex(EN_Project ph, int type, int index, int *value);

  /********************************************************************

  Analysis Options Functions

  ********************************************************************/

  /**
  @brief Retrieves the value of an analysis option.
  @param ph an EPANET project handle.
  @param option a type of analysis option (see @ref EN_Option).
  @param[out] value the current value of the option.
  @return an error code
  */
  int  DLLEXPORT EN_getoption(EN_Project ph, int option, double *value);

  /**
  @brief Sets the value for an anlysis option.
  @param ph an EPANET project handle.
  @param option a type of analysis option (see @ref EN_Option).
  @param value the new value assigned to the option.
  @return an error code.
  @see EN_Option
  */
  int  DLLEXPORT EN_setoption(EN_Project ph, int option, double value);

  /**
  @brief Retrieves a project's flow units.
  @param ph an EPANET project handle.
  @param[out] units a flow units code (see @ref EN_FlowUnits)
  @return an error code.

  Flow units in liters or cubic meters implies that SI metric units are used for all
  other quantities in addition to flow. Otherwise US Customary units are employed.
  */
  int  DLLEXPORT EN_getflowunits(EN_Project ph, int *units);

  /**
  @brief Sets a project's flow units.
  @param ph an EPANET project handle.
  @param units a flow units code (see @ref EN_FlowUnits)
  @return an error code.

  Flow units in liters or cubic meters implies that SI metric units are used for all
  other quantities in addition to flow. Otherwise US Customary units are employed.
  */
  int  DLLEXPORT EN_setflowunits(EN_Project ph, int units);

  /**
  @brief Retrieves the value of a time parameter.
  @param ph an EPANET project handle.
  @param param a time parameter code (see @ref EN_TimeParameter).
  @param[out] value the current value of the time parameter (in seconds).
  @return an error code.
  */
  int  DLLEXPORT EN_gettimeparam(EN_Project ph, int param, long *value);

  /**
  @brief Sets the value of a time parameter.
  @param ph an EPANET project handle.
  @param param a time parameter code (see @ref EN_TimeParameter).
  @param value the new value of the time parameter (in seconds)
  @return an error code.
  */
  int  DLLEXPORT EN_settimeparam(EN_Project ph, int param, long value);

  /**
  @brief Gets information about the type of water quality analysis requested.
  @param ph an EPANET project handle.
  @param[out] qualType type of analysis to run (see @ref EN_QualityType).
  @param[out] out_chemName name of chemical constituent.
  @param[out] out_chemUnits concentration units of the constituent.
  @param[out] traceNode index of the node being traced (if applicable).
  @return an error code.
  */
  int  DLLEXPORT EN_getqualinfo(EN_Project ph, int *qualType, char *out_chemName,
                 char *out_chemUnits, int *traceNode);

  /**
  @brief Retrieves the type of water quality analysis to be run.
  @param ph an EPANET project handle.
  @param[out] qualType the type of analysis to run (see @ref EN_QualityType).
  @param[out] traceNode the index of node being traced, if <b>qualType = EN_TRACE</b>.
  @return an error code.
  */
  int  DLLEXPORT EN_getqualtype(EN_Project ph, int *qualType, int *traceNode);

  /**
  @brief Sets the type of water quality analysis to run.
  @param ph an EPANET project handle.
  @param qualType the type of analysis to run (see @ref EN_QualityType).
  @param chemName the name of the quality constituent.
  @param chemUnits the concentration units of the constituent.
  @param traceNode the ID name of the node being traced if <b>qualType = EN_TRACE</b>.
  @return an error code.

  Chemical name and units can be an empty string if the analysis is not for a chemical.
  The same holds for the trace node if the analysis is not for source tracing.

  Note that the trace node is specified by ID name and not by index.
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
  @param nodeType the type of node being added (see @ref EN_NodeType)
  @param[out] index the index of the newly added node
  @return an error code.

  When a new node is created all of its properties (see @ref EN_NodeProperty) are set to 0.
  */
  int DLLEXPORT EN_addnode(EN_Project ph, char *id, int nodeType, int *index);

  /**
  @brief Deletes a node from a project.
  @param ph an EPANET project handle.
  @param index the index of the node to be deleted.
  @param actionCode the action taken if any control contains the node and its links.
  @return an error code.

  If \b actionCode is \b EN_UNCONDITIONAL then the node, its incident links and all
  simple and rule-based controls that contain them are deleted. If set to
  \b EN_CONDITIONAL then the node is not deleted if it or its incident links appear
  in any controls and error code 261 is returned.

  */
  int DLLEXPORT EN_deletenode(EN_Project ph, int index, int actionCode);

  /**
  @brief Gets the index of a node given its ID name.
  @param ph an EPANET project handle.
  @param id a node ID name.
  @param[out] index the node's index (starting from 1).
  @return an error code
  */
  int  DLLEXPORT EN_getnodeindex(EN_Project ph, char *id, int *index);

  /**
  @brief Gets the ID name of a node given its index.
  @param ph an EPANET project handle.
  @param index a node's index (starting from 1).
  @param[out] out_id the node's ID name.
  @return an error code

  The ID name must be sized to hold at least @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_getnodeid(EN_Project ph, int index, char *out_id);

  /**
  @brief Changes the ID name of a node.
  @param ph an EPANET project handle.
  @param index a node's index (starting from 1).
  @param newid the new ID name for the node.
  @return an error code.

  The ID name must not be longer than @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int DLLEXPORT EN_setnodeid(EN_Project ph, int index, char *newid);

  /**
  @brief Retrieves a node's type given its index.
  @param ph an EPANET project handle.
  @param index a node's index (starting from 1).
  @param[out] nodeType the node's type (see @ref EN_NodeType).
  @return an error code.
  */
  int  DLLEXPORT EN_getnodetype(EN_Project ph, int index, int *nodeType);

  /**
  @brief Retrieves a property value for a node.
  @param ph an EPANET project handle.
  @param index a node's index.
  @param property the property to retrieve (see @ref EN_NodeProperty).
  @param[out] value the current value of the property.
  @return an error code.

  Values are returned in units that depend on the units used for flow rate
  (see @ref Units).
  */
  int  DLLEXPORT EN_getnodevalue(EN_Project ph, int index, int property, double *value);

  /**
  @brief Sets a property value for a node.
  @param ph an EPANET project handle.
  @param index a node's index (starting from 1).
  @param property the property to set (see @ref EN_NodeProperty).
  @param value the new value for the property.
  @return an error code.

  Values are in units that depend on the units used for flow rate (see @ref Units).
  */
  int  DLLEXPORT EN_setnodevalue(EN_Project ph, int index, int property, double value);

  /**
  @brief Sets a group of properties for a junction node.
  @param ph an EPANET project handle.
  @param index a junction node's index (starting from 1).
  @param elev the value of the junction's elevation.
  @param dmnd the value of the junction's primary base demand.
  @param dmndpat the ID name of the demand's time pattern ("" for no pattern)
  @return an error code.

  These properties have units that depend on the units used for flow rate (see @ref Units).
  */
  int  DLLEXPORT EN_setjuncdata(EN_Project ph, int index, double elev, double dmnd,
      char *dmndpat);

  /**
  @brief Sets a group of properties for a tank node.
  @param ph an EPANET project handle.
  @param index a tank node's index (starting from 1).
  @param elev the tank's bottom elevation.
  @param initlvl the initial water level in the tank.
  @param minlvl the minimum water level for the tank.
  @param maxlvl the maximum water level for the tank.
  @param diam the tank's diameter (0 if a volume curve is supplied).
  @param minvol the volume of the tank at its minimum water level.
  @param volcurve the name of the tank's volume curve ("" for no curve)
  @return an error code.

  These properties have units that depend on the units used for flow rate (see @ref Units).
  */
  int  DLLEXPORT EN_settankdata(EN_Project ph, int index, double elev, double initlvl,
                 double minlvl, double maxlvl, double diam, double minvol, char *volcurve);

  /**
  @brief Gets the (x,y) coordinates of a node.
  @param ph an EPANET project handle.
  @param index a node index (starting from 1).
  @param[out] x the node's X-coordinate value.
  @param[out] y the node's Y-coordinate value.
  @return an error code.
  */
  int  DLLEXPORT EN_getcoord(EN_Project ph, int index, double *x, double *y);

  /**
  @brief Sets the (x,y) coordinates of a node.
  @param ph an EPANET project handle.
  @param index a node index (starting from 1).
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
  @param[out] type  Type of demand model (see @ref EN_DemandModel).
  @param[out] pmin  Pressure below which there is no demand.
  @param[out] preq  Pressure required to deliver full demand.
  @param[out] pexp  Pressure exponent in demand function.
  @return an error code.

  Parameters <b>pmin, preq,</b> and \b pexp are only used when the demand model is \b EN_PDA.
  */
  int DLLEXPORT EN_getdemandmodel(EN_Project ph, int *type, double *pmin,
                double *preq, double *pexp);

  /**
  @brief Sets the type of demand model to use and its parameters.
  @param ph an EPANET project handle.
  @param type  Type of demand model (see @ref EN_DemandModel).
  @param pmin  Pressure below which there is no demand.
  @param preq  Pressure required to deliver full demand.
  @param pexp  Pressure exponent in demand function.
  @return an error code.

  Set \b type to \b EN_DDA for a traditional demand driven analysis (in which case the
  remaining three parameter values are ignored) or to \b EN_PDA for a pressure driven
  analysis. In the latter case a node's demand is computed as:
  >  `Dfull * [ (P - pmin) / (preq - pmin) ] ^ pexp`
  where `Dfull` is the full demand and `P` is the current pressure.

  Setting \b preq equal to \b pmin will result in a solution with the smallest amount of
  demand reductions needed to insure that no node delivers positive demand at a pressure
  below \b pmin.
  */
  int DLLEXPORT EN_setdemandmodel(EN_Project ph, int type, double pmin,
                double preq, double pexp);


  /**
  @brief appends a new demand to a junction node demands list.
  @param ph an EPANET project handle.
  @param nodeIndex the index of a node (starting from 1).
  @param baseDemand the demand's base value.
  @param demandPattern the name of a time pattern used by the demand
  @param demandName the name of the demand's category
  @return an error code.

  A NULL or blank string can be used for `demandPattern` and for `demandName` to indicate
  that no time pattern or category name is associated with the demand.
  */
  int DLLEXPORT EN_adddemand(EN_Project ph, int nodeIndex, double baseDemand,
                char *demandPattern, char *demandName);

  /**
  @brief deletes a demand from a junction node.
  @param ph an EPANET project handle.
  @param nodeIndex the index of a node (starting from 1).
  @param demandIndex the position of the demand in the node's demands list (starting from 1).
  @return an error code.
  */
  int DLLEXPORT EN_deletedemand(EN_Project ph, int nodeIndex, int demandIndex);

  /**
  @brief Retrieves the index of a node's named demand category
  @param ph an EPANET project handle.
  @param nodeIndex the index of a node (starting from 1)
  @param demandName the name of a demand category for the node
  @param[out] demandIndex the index of the demand being sought
  @return an error code
  */
  int DLLEXPORT EN_getdemandindex(EN_Project ph, int nodeIndex, char *demandName,
                int *demandIndex);

  /**
  @brief Retrieves the number of demand categories for a junction node.
  @param ph an EPANET project handle.
  @param nodeIndex the index of a node (starting from 1).
  @param[out] numDemands the number of demand categories assigned to the node.
  @return an error code.
  */
  int  DLLEXPORT EN_getnumdemands(EN_Project ph, int nodeIndex, int *numDemands);

  /**
  @brief Gets the base demand for one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index (starting from 1).
  @param demandIndex the index of a demand category for the node (starting from 1).
  @param[out] baseDemand the category's base demand.
  @return an error code.
  */
  int  DLLEXPORT EN_getbasedemand(EN_Project ph, int nodeIndex, int demandIndex,
                 double *baseDemand);

  /**
  @brief Sets the base demand for one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index (starting from 1).
  @param demandIndex the index of a demand category for the node (starting from 1).
  @param baseDemand the new base demand for the category.
  @return an error code.
  */
  int  DLLEXPORT EN_setbasedemand(EN_Project ph, int nodeIndex, int demandIndex,
                 double baseDemand);

  /**
  @brief Retrieves the index of a time pattern assigned to one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex the node's index (starting from 1).
  @param demandIndex the index of a demand category for the node (starting from 1).
  @param[out] patIndex the index of the category's time pattern.
  @return an error code.

  A returned pattern index of 0 indicates that no time pattern has been assigned to the
  demand category.
  */
  int  DLLEXPORT EN_getdemandpattern(EN_Project ph, int nodeIndex, int demandIndex,
       int *patIndex);

  /**
  @brief Sets the index of a time pattern used for one of a node's demand categories.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index (starting from 1).
  @param demandIndex the index of one of the node's demand categories (starting from 1).
  @param patIndex the index of the time pattern assigned to the category.
  @return an error code.

  Specifying a pattern index of 0 indicates that no time pattern is assigned to the
  demand category.
  */
  int  DLLEXPORT EN_setdemandpattern(EN_Project ph, int nodeIndex, int demandIndex, int patIndex);

  /**
  @brief Retrieves the name of a node's demand category.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index (starting from 1).
  @param demandIndex the index of one of the node's demand categories (starting from 1).
  @param[out] out_demandName The name of the selected category.
  @return an error code.

  \b demandName must be sized to contain at least @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int DLLEXPORT EN_getdemandname(EN_Project ph, int nodeIndex, int demandIndex, char *out_demandName);

  /**
  @brief Assigns a name to a node's demand category.
  @param ph an EPANET project handle.
  @param nodeIndex a node's index (starting from 1).
  @param demandIdx the index of one of the node's demand categories (starting from 1).
  @param demandName the new name assigned to the category.
  @return Error code.

  The category name must contain no more than @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int DLLEXPORT EN_setdemandname(EN_Project ph, int nodeIndex, int demandIdx, char *demandName);

  /********************************************************************

  Link Functions

  ********************************************************************/

  /**
  @brief Adds a new link to a project.
  @param ph an EPANET project handle.
  @param id the ID name of the link to be added.
  @param linkType The type of link being added (see @ref EN_LinkType)
  @param fromNode The ID name of the link's starting node.
  @param toNode The ID name of the link's ending node.
  @param[out] index the index of the newly added link.
  @return an error code.

  A new pipe is assigned a diameter of 10 inches (254 mm) and a length of 330
  feet (~ 100 meters). Its roughness coefficient depends on the head loss formula in effect (see @ref EN_HeadLossType) as follows:
  - Hazen-Williams formula: 130
  - Darcy-Weisbach formula: 0.5 millifeet (0.15 mm)
  - Chezy-Manning formula: 0.01

  All other pipe properties are set to 0.

  A new pump has a status of \b EN_OPEN, a speed setting of 1, and has no pump
  curve or power rating assigned to it.

  A new valve has a diameter of 10 inches (254 mm) and all other properties set to 0.

  See @ref EN_LinkProperty.
  */
  int DLLEXPORT EN_addlink(EN_Project ph, char *id, int linkType, char *fromNode,
                          char *toNode, int *index);

  /**
  @brief Deletes a link from the project.
  @param ph an EPANET project handle.
  @param index the index of the link to be deleted.
  @param actionCode The action taken if any control contains the link.
  @return an error code.

  If \b actionCode is \b EN_UNCONDITIONAL then the link and all simple and rule-based
  controls that contain it are deleted. If set to \b EN_CONDITIONAL then the link
  is not deleted if it appears in any control and error 261 is returned.
  */
  int DLLEXPORT EN_deletelink(EN_Project ph, int index, int actionCode);

  /**
  @brief Gets the index of a link given its ID name.
  @param ph an EPANET project handle.
  @param id a link's ID name.
  @param[out] index the link's index (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_getlinkindex(EN_Project ph, char *id, int *index);

  /**
  @brief Gets the ID name of a link given its index.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param[out] out_id The link's ID name.
  @return an error code.

  The ID name must be sized to hold at least @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_getlinkid(EN_Project ph, int index, char *out_id);

  /**
  @brief Changes the ID name of a link.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param newid the new ID name for the link.
  @return Error code.

  The ID name must not be longer than @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int DLLEXPORT EN_setlinkid(EN_Project ph, int index, char *newid);

  /**
  @brief Retrieves a link's type.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param[out] linkType the link's type (see @ref EN_LinkType).
  @return an error code.
  */
  int  DLLEXPORT EN_getlinktype(EN_Project ph, int index, int *linkType);

  /**
  @brief Changes a link's type.
  @param ph an EPANET project handle.
  @param[in,out] inout_index the link's index before [in] and after [out] the type change.
  @param linkType the new type to change the link to (see @ref EN_LinkType).
  @param actionCode the action taken if any controls contain the link.
  @return an error code.

  If \b actionCode is \b EN_UNCONDITIONAL then all simple and rule-based controls that
  contain the link are deleted when the link's type is changed. If set to
  \b EN_CONDITIONAL then the type change is cancelled if the link appears in any
  control and error 261 is returned.
  */
  int  DLLEXPORT EN_setlinktype(EN_Project ph, int *inout_index, int linkType, int actionCode);

  /**
  @brief Gets the indexes of a link's start- and end-nodes.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param[out] node1 the index of the link's start node (starting from 1).
  @param[out] node2 the index of the link's end node (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_getlinknodes(EN_Project ph, int index, int *node1, int *node2);

  /**
  @brief Sets the indexes of a link's start- and end-nodes.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param node1 The index of the link's start node (starting from 1).
  @param node2 The index of the link's end node (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_setlinknodes(EN_Project ph, int index, int node1, int node2);

  /**
  @brief Retrieves a property value for a link.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param property the property to retrieve (see @ref EN_LinkProperty).
  @param[out] value the current value of the property.
  @return an error code.

  Values are returned in units that depend on the units used for flow rate (see @ref Units).
  */
  int  DLLEXPORT EN_getlinkvalue(EN_Project ph, int index, int property, double *value);

  /**
  @brief Sets a property value for a link.
  @param ph an EPANET project handle.
  @param index a link's index.
  @param property the property to set (see @ref EN_LinkProperty).
  @param value the new value for the property.
  @return an error code.

  Values are in units that depend on the units used for flow rate (see @ref Units).
  */
  int  DLLEXPORT EN_setlinkvalue(EN_Project ph, int index, int property, double value);

  /**
  @brief Sets a group of properties for a pipe link.
  @param ph an EPANET project handle.
  @param index the index of a pipe link (starting from 1).
  @param length the pipe's length.
  @param diam the pipe's diameter.
  @param rough the pipe's roughness coefficient.
  @param mloss the pipe's minor loss coefficient.
  @return an error code.

  These properties have units that depend on the units used for flow rate (see @ref Units).
  */
  int DLLEXPORT EN_setpipedata(EN_Project ph, int index, double length, double diam,
                double rough,  double mloss);

  /**
  @brief Retrieves the number of internal vertex points assigned to a link.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param[out] count the number of vertex points that describe the link's shape.
  @return an error code.
  */
  int DLLEXPORT EN_getvertexcount(EN_Project ph, int index, int *count);

  /**
  @brief Retrieves the coordinate's of a vertex point assigned to a link.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param vertex a vertex point index (starting from 1).
  @param[out] x the vertex's X-coordinate value.
  @param[out] y the vertex's Y-coordinate value.
  @return an error code.
  */
  int DLLEXPORT EN_getvertex(EN_Project ph, int index, int vertex, double *x, double *y);

  /**
  @brief Assigns a set of internal vertex points to a link.
  @param ph an EPANET project handle.
  @param index a link's index (starting from 1).
  @param x an array of X-coordinates for the vertex points.
  @param y an array of Y-coordinates for the vertex points.
  @param count the number of vertex points being assigned.
  @return an error code.

  Replaces any existing vertices previously assigned to the link.
  */
  int DLLEXPORT EN_setvertices(EN_Project ph, int index, double *x, double *y, int count);

  /********************************************************************

  Pump Functions

  ********************************************************************/

  /**
  @brief Retrieves the type of head curve used by a pump.
  @param ph an EPANET project handle.
  @param linkIndex the index of a pump link (starting from 1).
  @param[out] pumpType the type of head curve used by the pump (see @ref EN_PumpType).
  @return an error code.
  */
  int  DLLEXPORT EN_getpumptype(EN_Project ph, int linkIndex, int *pumpType);

  /**
  @brief Retrieves the curve assigned to a pump's head curve.
  @param ph an EPANET project handle.
  @param linkIndex the index of a pump link (starting from 1).
  @param[out] curveIndex the index of the curve assigned to the pump's head curve.
  @return an error code.
  */
  int  DLLEXPORT EN_getheadcurveindex(EN_Project ph, int linkIndex, int *curveIndex);

  /**
  @brief Assigns a curve to a pump's head curve.
  @param ph an EPANET project handle.
  @param linkIndex the index of a pump link (starting from 1).
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

  The new pattern contains a single time period whose factor is 1.0.
  */
  int  DLLEXPORT EN_addpattern(EN_Project ph, char *id);

  /**
  @brief Deletes a time pattern from a project.
  @param ph an EPANET project handle.
  @param index the time pattern's index (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_deletepattern(EN_Project ph, int index);

  /**
  @brief Retrieves the index of a time pattern given its ID name.
  @param ph an EPANET project handle.
  @param id the ID name of a time pattern.
  @param[out] index the time pattern's index (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternindex(EN_Project ph, char *id, int *index);

  /**
  @brief Retrieves the ID name of a time pattern given its index.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param[out] out_id the time pattern's ID name.
  @return an error code.

  The ID name must be sized to hold at least @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_getpatternid(EN_Project ph, int index, char *out_id);

  /**
  @brief Changes the ID name of a time pattern given its index.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param id the time pattern's new ID name.
  @return an error code.

  The new ID name must not exceed @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_setpatternid(EN_Project ph, int index, char *id);

  /**
  @brief Retrieves the number of time periods in a time pattern.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param[out] len the number of time periods in the pattern.
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternlen(EN_Project ph, int index, int *len);

  /**
  @brief Retrieves a time pattern's factor for a given time period.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param period a time period in the pattern (starting from 1).
  @param[out] value the pattern factor for the given time period.
  @return an error code.
  */
  int  DLLEXPORT EN_getpatternvalue(EN_Project ph, int index, int period, double *value);

  /**
  @brief Sets a time pattern's factor for a given time period.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param period a time period in the pattern (starting from 1).
  @param value the new value of the pattern factor for the given time period.
  @return an error code.
  */
  int  DLLEXPORT EN_setpatternvalue(EN_Project ph, int index, int period, double value);

  /**
  @brief Retrieves the average of all pattern factors in a time pattern.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param[out] value The average of all of the time pattern's factors.
  @return an error code.
  */
  int  DLLEXPORT EN_getaveragepatternvalue(EN_Project ph, int index, double *value);

  /**
  @brief Sets the pattern factors for a given time pattern.
  @param ph an EPANET project handle.
  @param index a time pattern index (starting from 1).
  @param values an array of new pattern factor values.
  @param len the number of factor values supplied.
  @return an error code.

  \b values is a zero-based array that contains \b len elements.

  Use this function to redefine (and resize) a time pattern all at once;
  use @ref EN_setpatternvalue to revise pattern factors one at a time.
  */
  int  DLLEXPORT EN_setpattern(EN_Project ph, int index, double *values, int len);

  /********************************************************************

  Data Curve Functions

  ********************************************************************/

  /**
  @brief Adds a new data curve to a project.
  @param ph an EPANET project handle.
  @param id The ID name of the curve to be added.
  @return an error code.

  The new curve contains a single data point (1.0, 1.0).
  */
  int  DLLEXPORT EN_addcurve(EN_Project ph, char *id);

  /**
  @brief Deletes a data curve from a project.
  @param ph an EPANET project handle.
  @param index the data curve's index (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_deletecurve(EN_Project ph, int index);

  /**
  @brief Retrieves the index of a curve given its ID name.
  @param ph an EPANET project handle.
  @param id the ID name of a curve.
  @param[out] index The curve's index (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_getcurveindex(EN_Project ph, char *id, int *index);

  /**
  @brief Retrieves the ID name of a curve given its index.
  @param ph an EPANET project handle.
  @param index a curve's index (starting from 1).
  @param[out] out_id the curve's ID name.
  @return an error code.

  The ID name must be sized to hold at least @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_getcurveid(EN_Project ph, int index, char *out_id);

  /**
  @brief Changes the ID name of a data curve given its index.
  @param ph an EPANET project handle.
  @param index a data curve index (starting from 1).
  @param id the data curve's new ID name.
  @return an error code.

  The new ID name must not exceed @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_setcurveid(EN_Project ph, int index, char *id);

  /**
  @brief Retrieves the number of points in a curve.
  @param ph an EPANET project handle.
  @param index a curve's index (starting from 1).
  @param[out] len The number of data points assigned to the curve.
  @return an error code.
  */
  int  DLLEXPORT EN_getcurvelen(EN_Project ph, int index, int *len);

  /**
  @brief Retrieves a curve's type.
  @param ph an EPANET project handle.
  @param index a curve's index (starting from 1).
  @param[out] type the curve's type (see @ref EN_CurveType).
  @return an error code.
  */
  int  DLLEXPORT EN_getcurvetype(EN_Project ph, int index, int *type);

  /**
  @brief Retrieves the value of a single data point for a curve.
  @param ph an EPANET project handle.
  @param curveIndex a curve's index (starting from 1).
  @param pointIndex the index of a point on the curve (starting from 1).
  @param[out] x the point's x-value.
  @param[out] y the point's y-value.
  @return an error code.
  */
  int  DLLEXPORT EN_getcurvevalue(EN_Project ph, int curveIndex, int pointIndex,
                 double *x, double *y);

  /**
  @brief Sets the value of a single data point for a curve.
  @param ph an EPANET project handle.
  @param curveIndex a curve's index (starting from 1).
  @param pointIndex the index of a point on the curve (starting from 1).
  @param x the point's new x-value.
  @param y the point's new y-value.
  @return an error code.
  */
  int  DLLEXPORT EN_setcurvevalue(EN_Project ph, int curveIndex, int pointIndex,
                 double x, double y);

  /**
  @brief Retrieves all of a curve's data.
  @param ph an EPANET project handle.
  @param index a curve's index (starting from 1).
  @param[out] out_id the curve's ID name.
  @param[out] nPoints the number of data points on the curve.
  @param[out] xValues the curve's x-values.
  @param[out] yValues the curve's y-values.
  @return an error code.

  The calling program is responsible for making `xValues` and `yValues` large enough
  to hold `nPoints` number of data points and for sizing `id` to hold at least
  @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_getcurve(EN_Project ph, int index, char *out_id, int *nPoints,
                 double *xValues, double *yValues);

  /**
  @brief assigns a set of data points to a curve.
  @param ph an EPANET project handle.
  @param index a curve's index (starting from 1).
  @param xValues an array of new x-values for the curve.
  @param yValues an array of new y-values for the curve.
  @param nPoints the new number of data points for the curve.
  @return an error code.

  \b xValues and \b yValues are zero-based arrays that contains \b nPoints elements.

  Use this function to redefine (and resize) a curve all at once;
  use @ref EN_setcurvevalue to revise a curve's data points one at a time.
  */
  int  DLLEXPORT EN_setcurve(EN_Project ph, int index, double *xValues,
                 double *yValues, int nPoints);

  /********************************************************************

  Simple Controls Functions

  ********************************************************************/

  /**
  @brief Adds a new simple control to a project.
  @param ph an EPANET project handle.
  @param type the type of control to add (see @ref EN_ControlType).
  @param linkIndex the index of a link to control (starting from 1).
  @param setting control setting applied to the link.
  @param nodeIndex index of the node used to control the link
  (0 for \b EN_TIMER and \b EN_TIMEOFDAY controls).
  @param level action level (tank level, junction pressure, or time in seconds)
  that triggers the control.
  @param[out] index index of the new control.
  @return an error code.
  */
  int  DLLEXPORT EN_addcontrol(EN_Project ph, int type, int linkIndex,
                 double setting, int nodeIndex, double level, int *index);

  /**
  @brief Deletes an existing simple control.
  @param ph an EPANET project handle.
  @param index the index of the control to delete (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_deletecontrol(EN_Project ph, int index);

  /**
  @brief Retrieves the properties of a simple control.
  @param ph an EPANET project handle.
  @param index the control's index (starting from 1).
  @param[out] type the type of control (see @ref EN_ControlType).
  @param[out] linkIndex the index of the link being controlled.
  @param[out] setting the control setting applied to the link.
  @param[out] nodeIndex the index of the node used to trigger the control
  (0 for \b EN_TIMER and  \b EN_TIMEOFDAY controls).
  @param[out] level the action level (tank level, junction pressure, or time in seconds)
  that triggers the control.
  @return an error code.
  */
  int  DLLEXPORT EN_getcontrol(EN_Project ph, int index, int *type, int *linkIndex,
                 double *setting, int *nodeIndex, double *level);

  /**
  @brief Sets the properties of an existing simple control.
  @param ph an EPANET project handle.
  @param index the control's index (starting from 1).
  @param type the type of control (see @ref EN_ControlType).
  @param linkIndex the index of the link being controlled.
  @param setting the control setting applied to the link.
  @param nodeIndex the index of the node used to trigger the control
  (0 for \b EN_TIMER and \b EN_TIMEOFDAY controls).
  @param level the action level (tank level, junction pressure, or time in seconds)
  that triggers the control.
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

  Consult the @ref RulesPage section of the @ref InpFile topic to learn about a
  rule's format. Each clause of the rule must end with a newline character <b>`\n`</b>.
  */
  int DLLEXPORT EN_addrule(EN_Project ph, char *rule);

  /**
  @brief Deletes an existing rule-based control.
  @param ph an EPANET project handle.
  @param index the index of the rule to be deleted (starting from 1).
  @return an error code.
  */
  int  DLLEXPORT EN_deleterule(EN_Project ph, int index);

  /**
  @brief Retrieves summary information about a rule-based control.
  @param ph an EPANET project handle.
  @param index the rule's index (starting from 1).
  @param[out] nPremises number of premises in the rule's IF section.
  @param[out] nThenActions number of actions in the rule's THEN section.
  @param[out] nElseActions number of actions in the rule's ELSE section.
  @param[out] priority the rule's priority value.
  @return an error code.
  */
  int  DLLEXPORT EN_getrule(EN_Project ph, int index, int *nPremises,
                 int *nThenActions, int *nElseActions, double *priority);

  /**
  @brief Gets the ID name of a rule-based control given its index.
  @param ph an EPANET project handle.
  @param index the rule's index (starting from 1).
  @param[out] out_id the rule's ID name.
  @return Error code.

  The ID name must be sized to hold at least @ref EN_SizeLimits "EN_MAXID" characters.
  */
  int  DLLEXPORT EN_getruleID(EN_Project ph, int index, char *out_id);

  /**
  @brief Gets the properties of a premise in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param premiseIndex the position of the premise in the rule's list of premises
  (starting from 1).
  @param[out] logop the premise's logical operator ( \b IF = 1, \b AND = 2, \b OR = 3 ).
  @param[out] object the type of object the premise refers to (see @ref EN_RuleObject).
  @param[out] objIndex the index of the object (e.g. the index of a tank).
  @param[out] variable the object's variable being compared (see @ref EN_RuleVariable).
  @param[out] relop the premise's comparison operator (see @ref EN_RuleOperator).
  @param[out] status the status that the object's status is compared to
  (see @ref EN_RuleStatus).
  @param[out] value the value that the object's variable is compared to.
  @return an error code.
  */
  int  DLLEXPORT EN_getpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                 int *logop,  int *object, int *objIndex, int *variable,
                 int *relop, int *status, double *value);

  /**
  @brief Sets the properties of a premise in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param premiseIndex the position of the premise in the rule's list of premises.
  @param logop the premise's logical operator ( \b IF = 1, \b AND = 2, \b OR = 3 ).
  @param object the type of object the premise refers to (see @ref EN_RuleObject).
  @param objIndex the index of the object (e.g. the index of a tank).
  @param variable the object's variable being compared (see @ref EN_RuleVariable).
  @param relop the premise's comparison operator (see @ref EN_RuleOperator).
  @param status the status that the object's status is compared to
  (see @ref EN_RuleStatus).
  @param value the value that the object's variable is compared to.
  @return an error code.
  */
  int  DLLEXPORT EN_setpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                 int logop, int object, int objIndex, int variable, int relop,
                 int status, double value);

  /**
  @brief Sets the index of an object in a premise of a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param premiseIndex the premise's index (starting from 1).
  @param objIndex the index of the premise's object (e.g. the index of a tank).
  @return an error code.
  */
  int  DLLEXPORT EN_setpremiseindex(EN_Project ph, int ruleIndex, int premiseIndex,
                 int objIndex);

  /**
  @brief Sets the status being compared to in a premise of a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param premiseIndex the premise's index (starting from 1).
  @param status the status that the premise's object status is compared to
  (see @ref EN_RuleStatus).
  @return an error code.
  */
  int  DLLEXPORT EN_setpremisestatus(EN_Project ph, int ruleIndex, int premiseIndex,
                 int status);

  /**
  @brief Sets the value in a premise of a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (staring from 1).
  @param premiseIndex the premise's index (starting from 1).
  @param value The value that the premise's variable is compared to.
  @return an error code.
  */
  int  DLLEXPORT EN_setpremisevalue(EN_Project ph, int ruleIndex, int premiseIndex,
                 double value);

  /**
  @brief Gets the properties of a THEN action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param actionIndex the index of the THEN action to retrieve (starting from 1).
  @param[out] linkIndex the index of the link in the action (starting from 1).
  @param[out] status the status assigned to the link (see @ref EN_RuleStatus)
  @param[out] setting the value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_getthenaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int *linkIndex, int *status, double *setting);

  /**
  @brief Sets the properties of a THEN action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param actionIndex the index of the THEN action to modify (starting from 1).
  @param linkIndex the index of the link in the action.
  @param status the new status assigned to the link (see @ref EN_RuleStatus).
  @param setting the new value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_setthenaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int linkIndex, int status, double setting);

  /**
  @brief Gets the properties of an ELSE action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param actionIndex the index of the ELSE action to retrieve (starting from 1).
  @param[out] linkIndex the index of the link in the action.
  @param[out] status the status assigned to the link (see @ref EN_RuleStatus).
  @param[out] setting the value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_getelseaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int *linkIndex, int *status, double *setting);

  /**
  @brief Sets the properties of an ELSE action in a rule-based control.
  @param ph an EPANET project handle.
  @param ruleIndex the rule's index (starting from 1).
  @param actionIndex the index of the ELSE action being modified (starting from 1).
  @param linkIndex the index of the link in the action (starting from 1).
  @param status the new status assigned to the link (see @ref EN_RuleStatus)
  @param setting the new value assigned to the link's setting.
  @return an error code.
  */
  int  DLLEXPORT EN_setelseaction(EN_Project ph, int ruleIndex, int actionIndex,
                 int linkIndex, int status, double setting);

  /**
  @brief Sets the priority of a rule-based control.
  @param ph an EPANET project handle.
  @param index the rule's index (starting from 1).
  @param priority the priority value assigned to the rule.
  @return an error code.
  */
  int  DLLEXPORT EN_setrulepriority(EN_Project ph, int index, double priority);

#if defined(__cplusplus)
}
#endif

#endif //EPANET2_2_H
