4th International Joint Conference on Water Distribution Systems Analysis and Computing and Control in the Water Industry (WDSA/CCWI 2026)


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

**ABSTRACT**

EPANET [1], with over 5,000 mentions in peer-reviewed literature (according to Scopus) and widespread use in both academic and professional domains, has long served as the reference engine for simulating pressurized water distribution networks. The release of EPANET 2.3 by the Open Water Analytics (OWA) community on GitHub, marks a major milestone in the software’s evolution. The new version incorporates substantial new modelling capabilities, improved numerical robustness, and significant enhancements to its API and interoperability features, achieved through the sustained efforts of a volunteer development team over a six-year period.

This paper presents the current release of EPANET 2.3, alongside its patch releases up to 2.3.3, as the open-source standard for simulating water distribution systems. We summarize the most significant additions to the engine, with particular focus on enhancements that expand its modelling realism and operational flexibility. One of the most notable developments is the integration of Fixed and Variable Area Discharge (FAVAD) leakage modelling. This new capability, implemented via a dedicated [LEAKAGE] input section, enables detailed accounting of water losses at links. Additionally, enhanced API functions provide comprehensive demand breakdown at each node, including emitter flow, leakage, and delivered demand components.

Pressure-dependent demand analysis has also been enhanced. Version 2.3 introduces improved convergence behaviour under low-pressure and intermittent supply conditions. Furthermore, the new Positional Control Valve (PCV) component allows for a more realistic representation of control elements using a customizable headloss curve and percent-open setting. Users can also configure whether emitters allow backflow, further enhancing flexibility in representing real-world conditions.

The new engine includes improved reporting and diagnostics. Specifically, a Flow Balance report summarizes inflows, outflows, and leakage losses at the end of each simulation, while new features allow users to flag components involved in control logic. Pressure units are now decoupled from flow units, enabling mixed-unit workflows, and additional units, including Cubic Meters per Second (CMS) have been added.

EPANET 2.3 introduces control logic and operation functions to enable (or disable) rule-based and simple controls dynamically. New diagnostics report the type of event causing the next hydraulic time step, aiding interpretation of time step adjustments. A new callback function mechanism allows reporting output to be redirected to custom destination, enabling seamless integration with external applications.

Reporting output can now be redirected to a callback function, allowing seamless integration into custom logging systems or GUIs. Moreover, users can force components fully open or closed using new control status constants.

The API has undergone major updates, enabling more comprehensive interaction with the EPANET model. New batch access functions enable efficient retrieval of properties across the network components, such as node and link properties, curve types, vertex coordinates, also supporting reading and writing tags. 

Software quality and numerical robustness have been significantly improved. Key enhancements include the relocation of network consistency checks to allow editing prior to simulation runs, the addition of continuous barrier functions for emitters, and improved handling of tanks, patterns, and initial settings. 

A continuous integration (CI) pipeline based on GitHub Actions runs regression tests and builds the library across all major platforms Windows (32/64-bit), Linux, and macOS, to ensure cross-platform consistency. Furthermore, a new function enables opening and editing input files containing formatting errors, facilitating error correction workflows. Moreover, C# support has been added.

The patch releases 2.3.1 through 2.3.3 address specific stability and correctness issues, including the handling of valve status initialization, minor loss coefficient assignment, and link status manipulation through the Toolkit API. These fixes improve the engine's reliability in automated or programmatic workflows.

To illustrate the benefits of these features, in the paper we will demonstrate illustrative case studies relevant to leakage management, e.g., using the FAVAD modeling capabilities to assess revenue and non-revenue water, manage pressure and leakage reduction using PCVs, and demonstration of the dynamic control management capabilities. These examples highlight how the expanded capabilities of EPANET 2.3 can enhance system analysis, inform operational decisions, and support research into pressure-dependent and intermittent supply regimes.

Finally, the paper outlines the roadmap for future development. The continued openness of development, enabled by GitHub-based workflows and community engagement, ensures that EPANET remains both a robust science-based open modelling platform, a foundation for research and innovation in the water sector, and an enabler of smart water systems.

**REFERENCES**

[1] Rossman, L. A. (2000). EPANET 2 Users Manual. U.S. Environmental Protection Agency, Cincinnati, OH
