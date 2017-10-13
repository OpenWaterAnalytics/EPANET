# -*- coding: utf-8 -*-
#
# setup.py 
#
# Setup up script for outputapi python extension
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

class CustomBuildExtCommand(build_ext):
    """build_ext command for use when numpy headers are needed."""
    def run(self):
        # Import numpy here, only when headers are needed
        import numpy

        # Add numpy headers to include_dirs
        try:
            numpy_include = numpy.get_include()
        except AttributeError:
            numpy_include = numpy.get_numpy_include()

        self.include_dirs.append(numpy_include)
        
        # Call original build_ext command
        build_ext.run(self)

        
setup(
    name = "outputapi", 
    cmdclass = {'build_ext': CustomBuildExtCommand}, 
    version = "1.0",
    ext_modules = [
        Extension(
            "_outputapi", 
            sources = ['src/outputapi.i', 'src/outputapi.c', 'src/errormanager.c'],
            swig_opts=['-modern']
        )
    ],
    package_dir = {'':'src'},  
    py_modules = ['outputapi'],
      
    install_requires = [
        'numpy>=1.6.0'
    ]
)
