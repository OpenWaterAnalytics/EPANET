# EPANET Website

This is the official website for the EPANET project, built with HTML, CSS, and JavaScript.

## Structure

```
website/
├── index.html           # Homepage
├── documentation.html   # API Documentation
├── examples.html        # Code Examples
├── css/
│   └── style.css        # Main stylesheet
├── js/
│   └── main.js          # JavaScript functionality
└── img/
    ├── Network.png      # Network diagram
    ├── DataFlow.png     # Data flow diagram
    ├── DistributionSystem.png
    └── Example2.png
```

## Features

- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Modern UI**: Clean, professional design with water-themed colors
- **Documentation**: Complete API reference and function categories
- **Examples**: Practical code examples in C
- **Mobile Navigation**: Hamburger menu for mobile devices

## Viewing the Website

To view the website locally, you can use any web server. For example:

```bash
# Using Python
cd website
python -m http.server 8000

# Using Node.js
npx serve .

# Using VS Code
# Open index.html with Live Server extension
```

Then open http://localhost:8000 in your browser.

## About EPANET

EPANET is an industry-standard program for modeling the hydraulic and water quality behavior of water distribution system pipe networks. It is maintained by the Open Water Analytics (OWA) community.

For more information, visit:
- [GitHub Repository](https://github.com/OpenWaterAnalytics/EPANET)
- [API Documentation](http://wateranalytics.org/EPANET/)
- [Building Guide](BUILDING.md)