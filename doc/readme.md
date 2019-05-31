

## Generating Documentation for OWA-EPANET 2.2

You must have [Doxygen](http://www.doxygen.nl)  installed on your machine to generate documentation for the OWA-EPANET Toolkit. Assuming this is the case, open a terminal window, navigate to the project's **`doc`** directory and issue the command **`doxygen`**. This will generate HTML documentation placed in a sub-directory named **`html`**.  From that directory you can launch the **`index.html`** file to view the full documentation in a web browser.

To generate a Windows compiled HTML Help file you must have [Microsoft's HTML Help Workshop](https://www.microsoft.com/en-us/download/details.aspx?id=21138) installed. You  then need to edit the Doxygen configuration file **`doxyfile`** as follows:

1. Change the **`GENERATE_HTMLHELP`** setting to **`YES`**.

2. Enter the location where the Help Workshop system was installed next to the
   **`HHC_LOCATION`** setting.

After running Doxygen again the resulting Help file named **`owa-epanet.chm`** will appear in the **`html`** sub-directory.

Doxygen uses the special comments placed in the project's **`epanet2_2.h`** and  **`epanet2_enums.h`** header files to document EPANET's API. It also uses supplementary material contained in the following files of the project's **`doc`** directory to generate additional pages of documentation:

- **`main.dox`**:  generates the *Overview* section.
- **`usage.dox`**: generates the *Usage* section.
- **`toolkit-examples.dox`** : generates the *Examples* section.
- **`toolkit-files.dox`**: generates the *Toolkit Files* section.
- **`input-file.dox`**: generates the *Input File* sub-section.
- **`toolkit-units.dox`**:  generates the *Measurement Units* section.
- **`modules.dox`**: defines the contents of the *API Reference* section.
             
Finally, a group of special Doxygen files are used to customize the format of the generated documentation. These include the following:
- **`doxyfile`**: the main Doxygen configuration file
- **`DoxygenLayout.xml`**: sets the title of the automatically generated *Modules* section to *API Reference* and hides the  *Files* section in the tree view pane of the document. 
- **`extrastylesheet.css`**: reduces the size of the the h1 heading style.
- **`newfooter.html`**: replaces the default Doxygen footer in HTML output with a custom one.
