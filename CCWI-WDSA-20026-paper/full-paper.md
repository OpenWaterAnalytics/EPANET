# EPANET 2.3: Advancing Modelling in Water Distribution Systems Simulations

* Marios Kyriakou*, KIOS Research and Innovation Center of Excellence, University of Cyprus, Cyprus
* Luke Butler, Iterating, Canada
* Demetrios G. Eliades, KIOS Research and Innovation Center of Excellence, University of Cyprus, Cyprus
* Sam Hatchett, Xylem Inc., USA
* Abel Heinsbroek, Vitens N.V., Netherlands
* Oscar Vegas Niño, Aqualia, Spain
* Lewis A. Rossman, OpenWaterAnalytics (OWA), USA
* Elad Salomons, Optiwater, Israel
* Alexander Sinske, GLS Consulting, South Africa
* Stelios Vrachimis, KIOS Research and Innovation Center of Excellence, University of Cyprus, Cyprus
* Dennis Zanutto, KWR Water Research Institute

*Corresponding author: kiriakou.marios@ucy.ac.cy

**Keywords:** Water Distribution Systems, Simulation, EPANET, Hydraulic Modeling

### ABSTRACT

### 1. INTRODUCTION
- Objectives: new capabilities, demonstration, future directions

EPANET, with over 5,000 mentions in peer-reviewed literature (according to Scopus) and widespread use in both academic and professional domains, has long served as the reference engine for simulating pressurized water distribution networks. The release of EPANET 2.3 by the Open Water Analytics (OWA) community on GitHub, marks a major milestone in the software’s evolution. The new version incorporates substantial new modelling capabilities, improved numerical robustness, and significant enhancements to its API and interoperability features, achieved through the sustained efforts of a volunteer development team over a six-year period. This paper presents the current release of EPANET 2.3, alongside its patch releases up to 2.3.3, as the open-source standard for simulating water distribution systems. We summarize the most significant additions to the engine, with particular focus on enhancements that expand its modelling realism and operational flexibility.

#### 1.1 EPANET HISTORY

EPANET was originally developed by the U.S. Environmental Protection Agency (hence the name "EPANET") to transfer its research on water quality modeling in distribution systems into a freely available software package that would benefit the water industry. Its first public release was made in 1993 (Rossman, 1994) and ran as a command line executable accompanied by a graphical user interface. A major update in 2000 (Rossman, 2000) repackaged the engine code into a shared library called the EPANET Toolkit (Rossman, 1999) with more than 50 functions that could be called from other applications. Bindings were included for C/C++, Pascal and Visual Basic. Over the years, third parties have created additional bindings for MATLAB (Eliades et al., 2016), R (Arandia and Eck, 2018) and Python (Kyriakou et al., 2023).

US EPA continued to support the program and issue maintenance releases through 2008. In 2014 the Open Water Analytics Community was formed on GitHub.  It is an informal international group of EPANET developers and users, whose objective is to provide group interaction and coordinated development of the EPANET engine codebase in collaboration with US EPA. 

OWA created a fork of the EPA 2.0 engine into a GitHub repository (https://github.com/openwateranalytics/epanet). Its first code release, version 2.1, was made in 2016 and added 17 new functions into the Toolkit. A more substantial release, version 2.2, was made in 2019. It extended the EPANET engine to include pressure dependent demands and included a number of computational improvements as well. An additional 55 functions were added to the Toolkit, bringing the total to 122. These allowed users to build a complete EPANET network model using just function calls, without the need to open an EPANET-formatted input file. The functions were also designed to allow multiple EPANET projects to be analyzed concurrently in a thread-safe manner. The OWA 2.2 engine was adopted by US EPA as its official release in 2020 (https://www.epa.gov/water-research/epanet) .

#### 1.2 EPANET ARCHITECTURE

The EPANET engine is written in C11 compatible C (and not C++) which allows it to be cross-platform. It is compiled into a shared library (a DLL under Windows or a shared object file under Linux and Mac OS) and includes a separate code module that can run the engine as a command line executable.

A general view of the EPANET engine structure is shown in Figure 1. It contains separate code modules for network building, hydraulic analysis, water quality analysis, and report generation. These modules function as follows:

<figure>
  <img src= "EPANET Toolkit 2.png" alt="EPANET Toolkit">
  <figcaption>Figure 1. EPANET Code Structure</figcaption>
</figure>

- The network builder receives a description of the network being simulated either from an external input file (.inp) or from a series of function calls that create network objects and assign their properties via code. These data are stored in a Project data structure.
- The hydraulics solver carries out an extended period hydraulic simulation. The results obtained at every time step can be written to an external, unformatted (binary) hydraulics file (.hyd). Some of these time steps might represent intermediate points in time where system conditions change because of tanks becoming full or empty or pumps turning on or off due to level controls or timed operation.
- If a water quality simulation is requested, the water quality solver accesses the flow data from the hydraulics file as it computes substance transport and reaction throughout the network over each hydraulic time step. During this process it can write both the formerly computed hydraulic results as well as its water quality results for each preset reporting interval to an unformatted (binary) output file (.out). If no water quality analysis was called for, then the hydraulic results stored in the .hyd file can simply be written out to the binary output file at uniform reporting intervals.
- If requested, a report writer reads back simulation results from the binary output file (.out) for each reporting period and writes out selected values to a formatted report file (.rpt). Any error or warning messages generated during the run are also written to this file.

Toolkit functions exist to carry out all of these steps under the programmer's control, including the ability to read and modify the contents of the Project data structure. It is also possible to create multiple Projects and run them concurrently in a thread safe manner.



### 2. MODELLING CAPABILITIES & ENHANCEMENTS
#### 2.1 LEAKAGE MODELLING
- FAVAD
- [LEAKAGE]/DEMAND

One of the most notable developments is the integration of Fixed and Variable Area Discharge (FAVAD) leakage modelling. This new capability, implemented via a dedicated [LEAKAGE] input section, enables detailed accounting of water losses at links. Additionally, enhanced API functions provide comprehensive demand breakdown at each node, including emitter flow, leakage, and delivered demand components.

#### 2.2 PRESSURE-DEPENDENT DEMAND ANALYSIS
Pressure-dependent demand analysis has also been enhanced. Version 2.3 introduces improved convergence behaviour under low-pressure and intermittent supply conditions. Users can also configure whether emitters allow backflow, further enhancing flexibility in representing real-world conditions.

#### 2.3 CONTROL LOGIC

Furthermore, the new Positional Control Valve (PCV) component allows for a more realistic representation of control elements using a customizable headloss curve and percent-open setting. EPANET 2.3 introduces control logic and operation functions to enable (or disable) rule-based and simple controls dynamically.

### 3. SOFTWARE IMPROVEMENTS
- API/Language support/Toolkits
- Quality improvements

The new engine includes improved reporting and diagnostics. Specifically, a Flow Balance report summarizes inflows, outflows, and leakage losses at the end of each simulation, while new features allow users to flag components involved in control logic. Pressure units are now decoupled from flow units, enabling mixed-unit workflows, and additional units, including Cubic Meters per Second (CMS) have been added.

New diagnostics report the type of event causing the next hydraulic time step, aiding interpretation of time step adjustments. A new callback function mechanism allows reporting output to be redirected to custom destination, enabling seamless integration with external applications.

Reporting output can now be redirected to a callback function, allowing seamless integration into custom logging systems or GUIs. Moreover, users can force components fully open or closed using new control status constants.

The API has undergone major updates, enabling more comprehensive interaction with the EPANET model. New batch access functions enable efficient retrieval of properties across the network components, such as node and link properties, curve types, vertex coordinates, also supporting reading and writing tags.

Software quality and numerical robustness have been significantly improved. Key enhancements include the relocation of network consistency checks to allow editing prior to simulation runs, the addition of continuous barrier functions for emitters, and improved handling of tanks, patterns, and initial settings.

A continuous integration (CI) pipeline based on GitHub Actions runs regression tests and builds the library across all major platforms Windows (32/64-bit), Linux, and macOS, to ensure cross-platform consistency. Furthermore, a new function enables opening and editing input files containing formatting errors, facilitating error correction workflows. Moreover, C# support has been added.

The patch releases 2.3.1 through 2.3.3 address specific stability and correctness issues, including the handling of valve status initialization, minor loss coefficient assignment, and link status manipulation through the Toolkit API. These fixes improve the engine's reliability in automated or programmatic workflows.



### 4. CASE STUDIES
- FAVAD MODELLING FOR REVENUE & NRW
- PRESSURE AND LEAKAGE REDUCTION
- DYNAMIC CONTROL DEMONSTRATION

To illustrate the benefits of these features, in the paper we will demonstrate illustrative case studies relevant to leakage management, e.g., using the FAVAD modeling capabilities to assess revenue and non-revenue water, manage pressure and leakage reduction using PCVs, and demonstration of the dynamic control management capabilities. These examples highlight how the expanded capabilities of EPANET 2.3 can enhance system analysis, inform operational decisions, and support research into pressure-dependent and intermittent supply regimes.


### 5. ROADMAP AND CONCLUSIONS

Future plans for OWA EPANET can be categorized as either near-term or long-term. Because the OWA participants volunteer their spare time to the project it is not possible to assign a specific time line to any of these proposals.

#### 5.1 NEAR-TERM PLANS

A number of engine enhancements have been discussed in the Issues section of the OWA EPANET repository. The most notable of these are:
- Allow named variables, math expressions and compound logical terms in control rules.
- Add the ability to model pump groups that operate under EPANET’s control statements.
- Add a modulating float valve used to maintain tanks at a fixed water level.
- Model variable speed pumps that meet node pressure targets.
- Allow individual nodes to have different pressure limits for pressure dependent demands rather than use the global values.
- Employ a backwards Euler method to update tank levels in an extended period simulation to produce more stable results.
- Allow time patterns to have a varying time step (making them time series) to better support the use of SCADA data in toolkit applications.
- Make the hydraulic solver more robust by using one-sided barrier functions to replace status checks on CVs, FCVs and pumps so that their head loss functions become continuous while keeping their flows within feasible bounds.

Some of these features have already been implemented in branches spawned off of the OWA EPANET development branch and are awaiting further testing before being adopted.

#### 5.2 LONG-TERM PLANS

A number of other ideas for improving the EPANET engine have been proposed over the years. These have yet to be formerly considered by the OWA project team and therefore fall into the category of long-term proposals. A sampling of these ideas include:

- Implement alternative network hydraulic solvers that may be more efficient than EPANET’s Global Gradient Algorithm (see Simpson et al., 2014; Elhay et al., 2014;  Alvarruiz, et al., 2015).
- Add a Rigid Water Column Unsteady Flow hydraulic solver as an alternative option to improve the accuracy of modeling time-varying changes in network flow conditions (Nault and Karney, 2016).
- Replace EPANET’s 1980’s vintage built-in linear equation solver with one from a more modern library, such as CHOLMOD (https://github.com/DrTimothyAldenDavis/SuiteSparse), PARDISO (https://www.intel.com/content/www/us/en/docs/onemkl/developer-reference-c/2023-0/pardiso.html) or EIGEN (https://libeigen.gitlab.io/) .
- Include longitudinal dispersion in EPANET’s water quality solver using the method that Shang et al. (2021) incorporated into EPANET-MSX (a multi-species extension of EPANET).

Finally, we note that the C code used in the EPANET engine was written over thirty years ago and has not changed much since then. While it produces very efficient and compact code, its lack of modularity and encapsulation make it difficult to modify existing features or add new features and computational methods. In addition, C is not a memory safe language which has implications for adoption by governmental agencies. Nor does it provide built-in support for multi-core processors and SIMD capabilities.

Two alternatives for rewriting the EPANET engine code have emerged. One uses C++ to redesign the engine in an object-oriented fashion. The more modular codebase would be easier to understand and to update. A partially completed implementation of this option can be found at https://github.com/OpenWaterAnalytics/epanet-dev . A second alternative for an engine rewrite is to do it in Rust. Rust's memory safety and native support for concurrency have made it an increasingly popular choice for high-performance computing applications. A partially completed Rust implementation of EPANET's hydraulic engine can be found at https://github.com/Vitens/epanet-rs .


#### 5.3 CONCLUSIONS

Over the years, EPANET has become a global standard for water distribution system analysis, widely used in engineering practice, education, and research. The continued openness of development, enabled by GitHub-based workflows and community engagement, ensures that EPANET remains both a robust science-based open modelling platform, a foundation for research and innovation in the water sector, and an enabler of smart water systems.

### REFERENCES

Alvarruiz, F. Alzamora, F.M. and Vidal, A.M. (2015). “Improving the efficiency of the loop method for the simulation of water distribution systems”, J. Water Resour. Plann. and Manage., 141(10):04015019.

Arandia, E. and Eck, B.J. (2018). “An R package for EPANET simulations”, Environmental Modelling & Software, 107:59-63. (doi.org:10.1016/j.envsoft.2018.05.016).

Elhay, S., A. R. Simpson, J. Deuerlein, B. Alexander, and W. H. Schilders. (2014). “Reformulated co-tree flows method competitive with the global gradient algorithm for solving water distribution system equations”, J. Water Resour. Plann. Manage., 140 (12): 04014040. doi.org/10.1061/(ASCE)WR.1943-5452.0000431.

Eliades, D.G., Kyriakou, M., Vrachimis, S., and Polycarpou, M.M. (2016). “EPANET-MATLAB Toolkit: An Open-Source Software for Interfacing EPANET with MATLAB”, in Proc. 14th International Conference on Computing and Control for the Water Industry (CCWI), The Netherlands, Nov 2016, p.8. (doi:10.5281/zenodo.831493).


Kyriakou, M. S., Demetriades, M., Vrachimis, S. G., Eliades, D. G., & Polycarpou, M. M. (2023). “EPyT: An EPANET-Python Toolkit for Smart Water Network Simulations”, Journal of Open Source Software, 8(92), 5947. (doi:10.21105/joss.05947).

Nault, J. and Karney, B. (2016). "Improved rigid water column formulation for simulating slow transients and controlled operations". Journal of Hydraulic Engineering, 142(9), 04016025 (doi:org.10.1061/(ASCE)HY.1943-7900.0001145).

Rossman, L.A. (1994). EPANET Users Manual, EPA-600/R-94/057, Risk Reduction Engineering Laboratory, U.S. Environmental Protection Agency, Cincinnati, OH.

Rossman, L.A. (1999). “The EPANET Programmer’s Toolkit for Analysis of Water Distribution Systems”, Proceedings of the 26th Annual Water Resources Planning & Management Conference, American Society of Civil Engineers, Tempe, Arizona, June 6-9, 1999.

Rossman, L.A. (2000). EPANET 2 Users Manual, EPA/600/R-00/057, National Risk Management Research Laboratory, U.S. Environmental Protection Agency, Cincinnati, OH.

Shang, F.,  H. Woo,  J. B. Burkhardt, R. Murray (2021). “Lagrangian Method to Model Advection-Dispersion-Reaction Transport in Drinking Water Pipe Networks”, Journal of Water Resources Planning and Management, 147(9): 04021057. (doi: 10.1061/(ASCE)WR.1943-
5452.0001421).

Simpson, A. R., S. Elhay, and B. Alexander. (2014). “Forest-core partitioning algorithm for speeding up analysis of water distribution systems.” J. Water Resour. Plann. Manage., 140 (4): 435–443. doi:10.1061/(ASCE)WR.1943-5452.0000336.
