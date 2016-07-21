Release Notes for EPANET 2.1 {#release_2_1}
============================

The last update to the EPANET engine was "Build 2.00.12" in February of 2008. Since that time, a community effort to update and extend the open-source code has emerged. This group has made a number of bug-fixes and API additions that help to improve the EPANET engine for everyone. Version 2.1 is now released after 8.5 years.

Contributors to this version (listed in order of first contribution):

- Lew Rossman
- Michael Tryby
- Feng Shang
- James Uber
- Tom Taxon
- Sam Hatchett
- Hyoungmin Woo
- Jinduan Chen
- Yunier Soad
- Mike Kane
- Demetrios Eliades
- Will Furnass
- Steffen Macke
- Mariosmsk
- Elad Salomons
- Maurizio Cingi
- Bryant McDonnell

##API Additions (new functions):
- `ENgetaveragepatternvalue`
- `ENgetstatistic`
- `ENgetcoord / ENsetcoord`
- `ENgetpumptype`
- `ENgetqualinfo`

###Demands
- `ENgetnumdemands`
- `ENgetbasedemand / ENsetbasedemand`
- `ENgetdemandpattern`

###Curves
- `ENgetcurve`
- `ENgetcurveid`
- `ENgetcurvelen`
- `ENgetcurvevalue`
- `ENsetcurvevalue`
- `ENsetcurve`
- `ENaddcurve`
- `ENgetheadcurveindex`
- `ENgetcurveindex`


##API Extensions (additional parameters)
###node value types:
- `EN_TANKVOLUME`
- `EN_MAXVOLUME`

###link value types:
- `EN_LINKQUAL`
- `EN_LINKPATTERN`

###time parameters:
- `EN_STARTTIME`
- `EN_HTIME`
- `EN_QTIME`
- `EN_HALTFLAG`
- `EN_NEXTEVENT`

###(new) statistic values:
- `EN_ITERATIONS`
- `EN_RELATIVEERROR`

###pump types
- `EN_CONST_HP`
- `EN_POWER_FUNC`
- `EN_CUSTOM`

##Notable Performance Improvements, Bug Fixes, Usage Features, and other notes
- API float type is a compile-time option with the `EN_API_FLOAT_TYPE` definition. Use either `float` or `double` - default if left undefined is `float` to maintain compatibility with 2.0.x
- updated hash table algorithm
- fixed memory leak when saving output
- enables interleaved hydraulic and water quality analysis steps:

```
	ENopenH();
	ENinitH(0);
	ENinitQ(EN_NOSAVE);
	do {
	  ENrunH(&t);
	  ENrunQ(&qt);
	  ENnextQ(&qstep);
	  // collect results
	  ENnextH(&tstep);
	} while (tstep > 0);
	ENcloseQ();
	ENcloseH();
```

- engine code and command-line executable are now in separate implementation files
- parameter `#define` directives are now enumerated values
- main header now contains doxygen-compatible comment blocks for auto-generated documentation





