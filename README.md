OWA-EPANET
======

## Build Status
[![Build status](https://ci.appveyor.com/api/projects/status/19wpg4g2cmj3oihl?svg=true)](https://ci.appveyor.com/project/OpenWaterAnalytics/epanet)

[![codecov](https://codecov.io/gh/OpenWaterAnalytics/EPANET/branch/master/graph/badge.svg)](https://codecov.io/gh/OpenWaterAnalytics/EPANET)

[![linux](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/ccpp.yml/badge.svg)](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/ccpp.yml)
[![macos](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/macos.yml/badge.svg)](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/macos.yml)
[![epanet2-win32](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/win32.yml/badge.svg)](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/win32.yml)
[![epanet2-win64](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/win64.yml/badge.svg)](https://github.com/OpenWaterAnalytics/EPANET/actions/workflows/win64.yml)

## DESCRIPTION

**EPANET** is an industry-standard program for modeling the hydraulic and water quality behavior of water distribution system pipe networks. The EPANET Programmer's Toolkit is a library of functions (or API) written in C that allow programmers to customize the use of EPANET's solution engine for their own applications. Both EPANET and its toolkit were originally developed by the U.S. Environmental Protection Agency (USEPA). If you are interested in using/extending the EPANET engine and its API for academic, personal, or commercial use, then you've come to the right place. [Read more about EPANET on Wikipedia](https://en.wikipedia.org/wiki/EPANET). (Please note that this project covers only the EPANET hydraulic and water quality solver engine, not the graphical user interface.)

## INSTALLATION

Instructions for building the OWA-EPANET Toolkit's function library as well as its command line executable from the source files in this repository can be found [here](https://github.com/OpenWaterAnalytics/EPANET/blob/master/BUILDING.md).

## USAGE

See the [full documentation](http://wateranalytics.org/EPANET/) of the OWA-EPANET API, along with examples of how to use the toolkit for water distribution system analysis. Additional information may be found on this project's [Wiki](https://github.com/openwateranalytics/epanet/wiki).

## CONTRIBUTING

Everyone is welcome to participate in this project. Whether you are helping others to resolve issues, reporting a new issue that hasn't yet been discovered, suggesting a new feature that would benefit your workflow, or writing code (or tests, or scripts, or ...), we value your time and effort. The path for contribution starts with the [Issues](https://github.com/OpenWaterAnalytics/EPANET/issues). Look around at open Issues and the conversation around them, get engaged by commenting on an outstanding Issue or creating a new one. If you want to contribute code, it helps to give the community time to discuss the ideas you present and offer constructive feedback. Once you get a clear path forward, Fork this repo to your own account. Make your commits on your dev branch (or one based on dev). Once you are finished, you can open a Pull Request to test the code and discuss merging your changes back into the community repository. A [step-by-step tutorial](http://www.slideshare.net/demetriseliades/contributing-to-epanet-using-github-in-windows) on how to contribute to OWA-EPANET using GitHub is also available.

## CREDITS

The **Open Water Analytics** (OWA) Community is an international group of EPANET developers and users, whose objective is to provide group interaction and coordinated development of the EPANET codebase, to ensure that important new user interface and algorithmic features are identified and that these features progress efficiently from prototype code to production implementations. OWA is actively maintaining OWA-EPANET, a community-supported branch of USEPA EPANET, since May 2014. The full list of individuals contributing to this project can be found [here](https://github.com/OpenWaterAnalytics/EPANET/blob/dev/AUTHORS).

## DISCLAIMER
Although OWA is not formally affiliated with nor endorsed by USEPA, this project has been a collaborative effort between the two that builds upon and extends the USEPA’s legacy EPANET 2.0 code base. For the last "official" release of EPANET please go to the [USEPA website](http://www2.epa.gov/water-research/epanet).

For more general community discussion, FAQ, and roadmapping of the project, please go to the [Community Forum](http://community.wateranalytics.org).
