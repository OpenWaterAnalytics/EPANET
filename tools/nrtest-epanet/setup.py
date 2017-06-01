'''
Created on Aug 30, 2016

@author: mtryby
'''

try: 
    from setuptools import setup
except ImportError:
    from distutils.core import setup

entry_points = {
    'nrtest.compare': [
        'epanet allclose = nrtest_epanet:epanet_allclose_compare',
        'epanet report = nrtest_epanet:epanet_report_compare',
        # Add entry point for new comparison functions here
    ]
}

setup(
    name='nrtest-epanet',
    version='0.2.0',
    description="EPANET extension for nrtest",
    
    author="Michael E. Tryby",
    author_email='tryby.michael@epa.gov',
    url='https://github.com/USEPA',
    
    packages=['nrtest_epanet',],
    entry_points=entry_points,
    include_package_data=True,
    install_requires=[
        'header_detail_footer>=2.3',
        'nrtest>=0.2.0',
        'numpy>=1.6.0',
        'epanet_reader>=0.2.0',
    ]
)
