Generating Documentation for OWA-EPANET 2.2
===========================================

You must have Doxygen (http://www.doxygen.nl/)installed on your machine to generate
documentation for OWA-EPANET's API (aka Toolkit). Assuming this is the case, open a
terminal window, navigate to the project's 'doc' directory and issue the command
'doxygen' to generate documentation in the following formats:

- HTML documentation will be placed in the 'html' sub-directory. Launch 'index.html'
  to view it in a web browser.

- Latex documentation will be placed in the 'latex' sub-directory. Assuming you
  have a TeX system, such as MikTex (https://miktex.org), installed on your machine
  you can generate a PDF of the documentation by issuing the 'make pdf' command from
  within the 'latex' directory. The resulting pdf file will be named 'refman.pdf'.

To generate a Windows compiled HTML Help file you must have Microsoft's HTML Help Workshop
(https://www.microsoft.com/en-us/download/details.aspx?id=21138) installed. In this case
you need to first edit the Doxygen configuration file 'Doxyfile' as follows:

1. Change the 'GENERATE_HTMLHELP' setting to 'YES'.

2. Enter the location where the Help Workshop system was installed next to the
   'HHC_LOCATION' setting.

After running 'doxygen' again the resulting Help file named 'owa-epanet.chm' will
appear in the 'html' sub-directory.

Doxygen uses the special comments placed in the project's 'epanet2_2.h' and
'epanet2_enums.h' header files to document EPANET's API. It also uses supplementary
material contained in the following files of the project's 'doc' folder to generate
additional pages of documentation:

main.dox - generates the Overview pages.

toolkit-usage.dox: generates the Toolkit Usage page.

toolkit-examples.dox : generates the Toolkit Examples pages.

modules.dox: generates the Reference section of the document consisting of several
             module pages that describe Toolkit functions by group, enumerated
             constants, file descriptions, error codes, property units, and output
             file format.
             
output-format.dox: generates the pages that describe the format used in different
                   sections of the output file. 

Finally, a group of special Doxygen files are used to customize the format of the
generated documentation. These include the following:

Doxyfile - the main Doxygen configuration file

DoxygenLayout.xml - replaces the title "Modules" with "Reference" and hides the
                    "Files" section in the tree view pane of the document. 
                    
extrastylesheet.css - reduces the size of the the h1 heading style.

newfooter.html - replaces the default Doxygen footer in HTML output with a custom one.

header.tex - replaces the standard title page and footer text used in Latex output.
                          