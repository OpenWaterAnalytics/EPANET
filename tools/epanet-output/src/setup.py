# -*- coding: utf-8 -*-

try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension

# Third-party modules - we depend on numpy for everything
import numpy

# Obtain the numpy include directory.  This logic works across numpy versions.
try:
    numpy_include = numpy.get_include()
except AttributeError:
    numpy_include = numpy.get_numpy_include()

setup(name = "OUTPUTAPI",
    version = "1.0",
    ext_modules = [
        Extension(
            "_outputapi", 
            sources = ["outputapi.i", "outputapi.c", "errormanager.c"],
            include_dirs = [numpy_include], 
            swig_opts=['-modern']
        )
    ],
)
