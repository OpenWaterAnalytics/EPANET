

## Generating Documentation for OWA-EPANET 2.3

You must have [Doxygen](http://www.doxygen.nl) version 1.13 installed on your machine to generate documentation for the OWA-EPANET Toolkit. Assuming this is the case, open a terminal window, navigate to the project's `doc` directory and issue the command `doxygen`. This will generate HTML documentation placed in a sub-directory named `html`.  From that directory you can launch the `index.html` file to view the full documentation in a web browser.

To generate a Windows compiled HTML Help file you must have [Microsoft's HTML Help Workshop](http://web.archive.org/web/20160201063255/http://download.microsoft.com/download/0/A/9/0A939EF6-E31C-430F-A3DF-DFAE7960D564/htmlhelp.exe) installed. Then follow these steps:
1. Open a terminal window and navigate to the project's `doc` directory.
2. Edit the configuration file `Doxyfile-chm` by entering the location where the Help Workshop system was installed next to the `HHC_LOCATION` setting.
3. Issue the command `doxygen Doxyfile-chm` to generate a compiled Help file named `owa-epanet.chm` in the `doc` directory.
4. You can delete the `html-chm` sub-directory created by Doxygen. 

Doxygen uses the special comments placed in the project's `epanet2_2.h` and  `epanet2_enums.h` header files to document EPANET's API. It also uses supplementary material contained in the following files of the project's `doc` directory to generate additional pages of documentation:

- `main.dox`:  generates the *Overview* section.
- `toolkit-usage.dox`: generates the *Usage* section.
- `toolkit-examples.dox` : generates the *Examples* section.
- `toolkit-files.dox`: generates the *Toolkit Files* section.
- `toolkit-input.dox`: generates the *Input File* sub-section.
- `toolkit-units.dox`:  generates the *Measurement Units* section.
- `toolkit-topics.dox`: defines the contents of the *API Reference* section.
             
Finally, a group of special Doxygen files are used to customize the format of the generated documentation. These include the following:
- `Doxyfile`: the Doxygen configuration file for HTML output
- `Doxyfile-chm`: the Doxygen configuration file for HTML Help output
- `DoxygenLayout.xml`: sets the title of the automatically generated *Topics* section to *API Reference* and hides the  *Files* section in the tree view pane of the document. 
- `doxygen-awsome.css`: applies a custom theme provided by [doxygen-awesome](https://github.com/jothepro/doxygen-awesome-css) to produce a more modern visual style for HTML output.
- `newfooter.html`: replaces the default Doxygen footer in HTML output with a custom one.
