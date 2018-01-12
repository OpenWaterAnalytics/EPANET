# -*- coding: utf-8 -*-
#
# setup.py - Setup up script for en_toolkit python extension
# 
# Created:    11/27/2017
# Author:     Michael E. Tryby
#             US EPA - ORD/NRMRL
#
# Requires:
#   Platform C language compiler   
#   SWIG
#

try:
    from setuptools import setup, Extension
    from setuptools.command.build_ext import build_ext
except ImportError:
    from distutils.core import setup, Extension
    from distutils.command.build_ext import build_ext

setup(
    name = "epanet-toolkit", 
    version = "0.0.0",
    ext_modules = [
        Extension("_epanet_toolkit",
            include_dirs = ['lib\\'],           
            libraries = ['epanet'],
            library_dirs = ['lib\\'],      
            sources = ['src\\epanet_toolkit.i'],
            swig_opts=['-modern'],
            language = 'C'
        )
    ],
    package_dir = {'':'src'},  
    py_modules = ['epanet_toolkit'],
      
    install_requires = [
        'enum34'
    ]
)
