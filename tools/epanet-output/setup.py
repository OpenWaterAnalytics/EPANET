# -*- coding: utf-8 -*-
#
# setup.py 
# 
# Created:    9/20/2017
# Author:     Michael E. Tryby
#             US EPA - ORD/NRMRL
# 
# Setup up script for en_outputapi python extension
#
# Requires:
#   Platform C language compiler   
#   Python packages: numpy
#

try:
    from setuptools import setup, Extension
    from setuptools.command.build_ext import build_ext
except ImportError:
    from distutils.core import setup, Extension
    from distutils.command.build_ext import build_ext

setup(
    name = "epanet-output", 
    version = "1.0",
    ext_modules = [
        Extension("_epanet_output",
            sources = ['src/epanet_output.i', 'src/epanet_output.c', 'src/errormanager.c'],
            swig_opts=['-modern'],
            language = 'C'
        )
    ],
    package_dir = {'':'src'},  
    py_modules = ['epanet_output'],
      
    install_requires = [
        'enum34'
    ]
)
