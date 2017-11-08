# -*- coding: utf-8 -*-

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup
    

setup(
    name = "epanet-reader",
    version = "0.2.0",
    description = "Tools for reading EPANET binary results file",
    
    author="Michael E. Tryby",
    author_email='tryby.michael@epa.gov',
    url='https://github.com/USEPA',
    
    packages = ['epanet_reader'],
    
    install_requires = ['numpy', 'enum34'],

    keywords='epanet_reader'
)
