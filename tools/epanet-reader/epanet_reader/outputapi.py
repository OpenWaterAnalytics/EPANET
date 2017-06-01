'''Wrapper for outputapi.h

Generated with:
C:\Users\mtryby\dev\Anaconda2\Scripts\ctypesgen.py -a -l libepanet-output -o outputapi.py .\src\outputapi.h

Do not modify this file ... This file HAS been modified! (See line 592)
'''

__docformat__ =  'restructuredtext'

# Begin preamble

import ctypes, os, sys
from ctypes import *

_int_types = (c_int16, c_int32)
if hasattr(ctypes, 'c_int64'):
    # Some builds of ctypes apparently do not have c_int64
    # defined; it's a pretty good bet that these builds do not
    # have 64-bit pointers.
    _int_types += (c_int64,)
for t in _int_types:
    if sizeof(t) == sizeof(c_size_t):
        c_ptrdiff_t = t
del t
del _int_types

class c_void(Structure):
    # c_void_p is a buggy return type, converting to int, so
    # POINTER(None) == c_void_p is actually written as
    # POINTER(c_void), so it can be treated as a real pointer.
    _fields_ = [('dummy', c_int)]

def POINTER(obj):
    p = ctypes.POINTER(obj)

    # Convert None to a real NULL pointer to work around bugs
    # in how ctypes handles None on 64-bit platforms
    if not isinstance(p.from_param, classmethod):
        def from_param(cls, x):
            if x is None:
                return cls()
            else:
                return x
        p.from_param = classmethod(from_param)

    return p

class UserString:
    def __init__(self, seq):
        if isinstance(seq, basestring):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else:
            self.data = str(seq)
    def __str__(self): return str(self.data)
    def __repr__(self): return repr(self.data)
    def __int__(self): return int(self.data)
    def __long__(self): return long(self.data)
    def __float__(self): return float(self.data)
    def __complex__(self): return complex(self.data)
    def __hash__(self): return hash(self.data)

    def __cmp__(self, string):
        if isinstance(string, UserString):
            return cmp(self.data, string.data)
        else:
            return cmp(self.data, string)
    def __contains__(self, char):
        return char in self.data

    def __len__(self): return len(self.data)
    def __getitem__(self, index): return self.__class__(self.data[index])
    def __getslice__(self, start, end):
        start = max(start, 0); end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, basestring):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other))
    def __radd__(self, other):
        if isinstance(other, basestring):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other) + self.data)
    def __mul__(self, n):
        return self.__class__(self.data*n)
    __rmul__ = __mul__
    def __mod__(self, args):
        return self.__class__(self.data % args)

    # the following methods are defined in alphabetical order:
    def capitalize(self): return self.__class__(self.data.capitalize())
    def center(self, width, *args):
        return self.__class__(self.data.center(width, *args))
    def count(self, sub, start=0, end=sys.maxint):
        return self.data.count(sub, start, end)
    def decode(self, encoding=None, errors=None): # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.decode(encoding, errors))
            else:
                return self.__class__(self.data.decode(encoding))
        else:
            return self.__class__(self.data.decode())
    def encode(self, encoding=None, errors=None): # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else:
            return self.__class__(self.data.encode())
    def endswith(self, suffix, start=0, end=sys.maxint):
        return self.data.endswith(suffix, start, end)
    def expandtabs(self, tabsize=8):
        return self.__class__(self.data.expandtabs(tabsize))
    def find(self, sub, start=0, end=sys.maxint):
        return self.data.find(sub, start, end)
    def index(self, sub, start=0, end=sys.maxint):
        return self.data.index(sub, start, end)
    def isalpha(self): return self.data.isalpha()
    def isalnum(self): return self.data.isalnum()
    def isdecimal(self): return self.data.isdecimal()
    def isdigit(self): return self.data.isdigit()
    def islower(self): return self.data.islower()
    def isnumeric(self): return self.data.isnumeric()
    def isspace(self): return self.data.isspace()
    def istitle(self): return self.data.istitle()
    def isupper(self): return self.data.isupper()
    def join(self, seq): return self.data.join(seq)
    def ljust(self, width, *args):
        return self.__class__(self.data.ljust(width, *args))
    def lower(self): return self.__class__(self.data.lower())
    def lstrip(self, chars=None): return self.__class__(self.data.lstrip(chars))
    def partition(self, sep):
        return self.data.partition(sep)
    def replace(self, old, new, maxsplit=-1):
        return self.__class__(self.data.replace(old, new, maxsplit))
    def rfind(self, sub, start=0, end=sys.maxint):
        return self.data.rfind(sub, start, end)
    def rindex(self, sub, start=0, end=sys.maxint):
        return self.data.rindex(sub, start, end)
    def rjust(self, width, *args):
        return self.__class__(self.data.rjust(width, *args))
    def rpartition(self, sep):
        return self.data.rpartition(sep)
    def rstrip(self, chars=None): return self.__class__(self.data.rstrip(chars))
    def split(self, sep=None, maxsplit=-1):
        return self.data.split(sep, maxsplit)
    def rsplit(self, sep=None, maxsplit=-1):
        return self.data.rsplit(sep, maxsplit)
    def splitlines(self, keepends=0): return self.data.splitlines(keepends)
    def startswith(self, prefix, start=0, end=sys.maxint):
        return self.data.startswith(prefix, start, end)
    def strip(self, chars=None): return self.__class__(self.data.strip(chars))
    def swapcase(self): return self.__class__(self.data.swapcase())
    def title(self): return self.__class__(self.data.title())
    def translate(self, *args):
        return self.__class__(self.data.translate(*args))
    def upper(self): return self.__class__(self.data.upper())
    def zfill(self, width): return self.__class__(self.data.zfill(width))

class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""
    def __init__(self, string=""):
        self.data = string
    def __hash__(self):
        raise TypeError("unhashable type (it is mutable)")
    def __setitem__(self, index, sub):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data): raise IndexError
        self.data = self.data[:index] + sub + self.data[index+1:]
    def __delitem__(self, index):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data): raise IndexError
        self.data = self.data[:index] + self.data[index+1:]
    def __setslice__(self, start, end, sub):
        start = max(start, 0); end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start]+sub.data+self.data[end:]
        elif isinstance(sub, basestring):
            self.data = self.data[:start]+sub+self.data[end:]
        else:
            self.data =  self.data[:start]+str(sub)+self.data[end:]
    def __delslice__(self, start, end):
        start = max(start, 0); end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]
    def immutable(self):
        return UserString(self.data)
    def __iadd__(self, other):
        if isinstance(other, UserString):
            self.data += other.data
        elif isinstance(other, basestring):
            self.data += other
        else:
            self.data += str(other)
        return self
    def __imul__(self, n):
        self.data *= n
        return self

class String(MutableString, Union):

    _fields_ = [('raw', POINTER(c_char)),
                ('data', c_char_p)]

    def __init__(self, obj=""):
        if isinstance(obj, (str, unicode, UserString)):
            self.data = str(obj)
        else:
            self.raw = obj

    def __len__(self):
        return self.data and len(self.data) or 0

    def from_param(cls, obj):
        # Convert None or 0
        if obj is None or obj == 0:
            return cls(POINTER(c_char)())

        # Convert from String
        elif isinstance(obj, String):
            return obj

        # Convert from str
        elif isinstance(obj, str):
            return cls(obj)

        # Convert from c_char_p
        elif isinstance(obj, c_char_p):
            return obj

        # Convert from POINTER(c_char)
        elif isinstance(obj, POINTER(c_char)):
            return obj

        # Convert from raw pointer
        elif isinstance(obj, int):
            return cls(cast(obj, POINTER(c_char)))

        # Convert from object
        else:
            return String.from_param(obj._as_parameter_)
    from_param = classmethod(from_param)

def ReturnString(obj, func=None, arguments=None):
    return String.from_param(obj)

# As of ctypes 1.0, ctypes does not support custom error-checking
# functions on callbacks, nor does it support custom datatypes on
# callbacks, so we must ensure that all callbacks return
# primitive datatypes.
#
# Non-primitive return values wrapped with UNCHECKED won't be
# typechecked, and will be converted to c_void_p.
def UNCHECKED(type):
    if (hasattr(type, "_type_") and isinstance(type._type_, str)
        and type._type_ != "P"):
        return type
    else:
        return c_void_p

# ctypes doesn't have direct support for variadic functions, so we have to write
# our own wrapper class
class _variadic_function(object):
    def __init__(self,func,restype,argtypes):
        self.func=func
        self.func.restype=restype
        self.argtypes=argtypes
    def _as_parameter_(self):
        # So we can pass this variadic function as a function pointer
        return self.func
    def __call__(self,*args):
        fixed_args=[]
        i=0
        for argtype in self.argtypes:
            # Typecheck what we can
            fixed_args.append(argtype.from_param(args[i]))
            i+=1
        return self.func(*fixed_args+list(args[i:]))

# End preamble

_libs = {}
_libdirs = []

# Begin loader

# ----------------------------------------------------------------------------
# Copyright (c) 2008 David James
# Copyright (c) 2006-2008 Alex Holkner
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

import os.path, re, sys, glob
import ctypes
import ctypes.util

def _environ_path(name):
    if name in os.environ:
        return os.environ[name].split(":")
    else:
        return []

class LibraryLoader(object):
    def __init__(self):
        self.other_dirs=[]

    def load_library(self,libname):
        """Given the name of a library, load it."""
        paths = self.getpaths(libname)

        for path in paths:
            if os.path.exists(path):
                return self.load(path)

        raise ImportError("%s not found." % libname)

    def load(self,path):
        """Given a path to a library, load it."""
        try:
            # Darwin requires dlopen to be called with mode RTLD_GLOBAL instead
            # of the default RTLD_LOCAL.  Without this, you end up with
            # libraries not being loadable, resulting in "Symbol not found"
            # errors
            if sys.platform == 'darwin':
                return ctypes.CDLL(path, ctypes.RTLD_GLOBAL)
            else:
                return ctypes.cdll.LoadLibrary(path)
        except OSError,e:
            raise ImportError(e)

    def getpaths(self,libname):
        """Return a list of paths where the library might be found."""
        if os.path.isabs(libname):
            yield libname

        else:
            for path in self.getplatformpaths(libname):
                yield path

            path = ctypes.util.find_library(libname)
            if path: yield path

    def getplatformpaths(self, libname):
        return []

# Darwin (Mac OS X)

class DarwinLibraryLoader(LibraryLoader):
    name_formats = ["lib%s.dylib", "lib%s.so", "lib%s.bundle", "%s.dylib",
                "%s.so", "%s.bundle", "%s"]

    def getplatformpaths(self,libname):
        if os.path.pathsep in libname:
            names = [libname]
        else:
            names = [format % libname for format in self.name_formats]

        for dir in self.getdirs(libname):
            for name in names:
                yield os.path.join(dir,name)

    def getdirs(self,libname):
        '''Implements the dylib search as specified in Apple documentation:

        http://developer.apple.com/documentation/DeveloperTools/Conceptual/
            DynamicLibraries/Articles/DynamicLibraryUsageGuidelines.html

        Before commencing the standard search, the method first checks
        the bundle's ``Frameworks`` directory if the application is running
        within a bundle (OS X .app).
        '''

        dyld_fallback_library_path = _environ_path("DYLD_FALLBACK_LIBRARY_PATH")
        if not dyld_fallback_library_path:
            dyld_fallback_library_path = [os.path.expanduser('~/lib'),
                                          '/usr/local/lib', '/usr/lib']

        dirs = []

        if '/' in libname:
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
        else:
            dirs.extend(_environ_path("LD_LIBRARY_PATH"))
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))

        dirs.extend(self.other_dirs)
        dirs.append(".")

        if hasattr(sys, 'frozen') and sys.frozen == 'macosx_app':
            dirs.append(os.path.join(
                os.environ['RESOURCEPATH'],
                '..',
                'Frameworks'))

        dirs.extend(dyld_fallback_library_path)

        return dirs

# Posix

class PosixLibraryLoader(LibraryLoader):
    _ld_so_cache = None

    def _create_ld_so_cache(self):
        # Recreate search path followed by ld.so.  This is going to be
        # slow to build, and incorrect (ld.so uses ld.so.cache, which may
        # not be up-to-date).  Used only as fallback for distros without
        # /sbin/ldconfig.
        #
        # We assume the DT_RPATH and DT_RUNPATH binary sections are omitted.

        directories = []
        for name in ("LD_LIBRARY_PATH",
                     "SHLIB_PATH", # HPUX
                     "LIBPATH", # OS/2, AIX
                     "LIBRARY_PATH", # BE/OS
                    ):
            if name in os.environ:
                directories.extend(os.environ[name].split(os.pathsep))
        directories.extend(self.other_dirs)
        directories.append(".")

        try: directories.extend([dir.strip() for dir in open('/etc/ld.so.conf')])
        except IOError: pass

        directories.extend(['/lib', '/usr/lib', '/lib64', '/usr/lib64'])

        cache = {}
        lib_re = re.compile(r'lib(.*)\.s[ol]')
        ext_re = re.compile(r'\.s[ol]$')
        for dir in directories:
            try:
                for path in glob.glob("%s/*.s[ol]*" % dir):
                    file = os.path.basename(path)

                    # Index by filename
                    if file not in cache:
                        cache[file] = path

                    # Index by library name
                    match = lib_re.match(file)
                    if match:
                        library = match.group(1)
                        if library not in cache:
                            cache[library] = path
            except OSError:
                pass

        self._ld_so_cache = cache

    def getplatformpaths(self, libname):
        if self._ld_so_cache is None:
            self._create_ld_so_cache()

        result = self._ld_so_cache.get(libname)
        if result: yield result

        path = ctypes.util.find_library(libname)
        if path: yield os.path.join("/lib",path)

# Windows

class _WindowsLibrary(object):
    def __init__(self, path):
        self.cdll = ctypes.cdll.LoadLibrary(path)
        self.windll = ctypes.windll.LoadLibrary(path)

    def __getattr__(self, name):
        try: return getattr(self.cdll,name)
        except AttributeError:
            try: return getattr(self.windll,name)
            except AttributeError:
                raise

class WindowsLibraryLoader(LibraryLoader):
    name_formats = ["%s.dll", "lib%s.dll", "%slib.dll"]

    def load_library(self, libname):
        try:
            result = LibraryLoader.load_library(self, libname)
        except ImportError:
            result = None
            if os.path.sep not in libname:
                for name in self.name_formats:
                    try:
                        result = getattr(ctypes.cdll, name % libname)
                        if result:
                            break
                    except WindowsError:
                        result = None
            if result is None:
                try:
                    result = getattr(ctypes.cdll, libname)
                except WindowsError:
                    result = None
            if result is None:
                raise ImportError("%s not found." % libname)
        return result

    def load(self, path):
        return _WindowsLibrary(path)

    def getplatformpaths(self, libname):
        if os.path.sep not in libname:
            for name in self.name_formats:
                dll_in_current_dir = os.path.abspath(name % libname)
                if os.path.exists(dll_in_current_dir):
                    yield dll_in_current_dir
                path = ctypes.util.find_library(name % libname)
                if path:
                    yield path

# Platform switching

# If your value of sys.platform does not appear in this dict, please contact
# the Ctypesgen maintainers.

loaderclass = {
    "darwin":   DarwinLibraryLoader,
    "cygwin":   WindowsLibraryLoader,
    "win32":    WindowsLibraryLoader
}

loader = loaderclass.get(sys.platform, PosixLibraryLoader)()

def add_library_search_dirs(other_dirs):
    loader.other_dirs = other_dirs

load_library = loader.load_library

del loaderclass

# End loader

add_library_search_dirs([])

# Begin libraries
###############################################################################
###### THIS NEEDS TO BE HAND CODED OR LIBRARY LOAD WILL BREAK ON WINDOWS ######
import site
_libs["libepanet-output"] = load_library(site.getsitepackages()[1] + "\epanet_reader-0.2.0-py2.7.egg\epanet_reader\libepanet-output")
###############################################################################
###############################################################################
# 1 libraries
# End libraries

# No modules

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 16
class struct_ENResultsAPI(Structure):
    pass

ENResultsAPI = struct_ENResultsAPI # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 16

enum_anon_1 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 21

ENR_node = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 21

ENR_link = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 21

ENR_ElementType = enum_anon_1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 21

enum_anon_2 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

ENR_getSeries = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

ENR_getAttribute = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

ENR_getResult = 3 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

ENR_getReacts = 4 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

ENR_getEnergy = 5 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

ENR_ApiFunction = enum_anon_2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 29

enum_anon_3 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

ENR_nodeCount = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

ENR_tankCount = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

ENR_linkCount = 3 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

ENR_pumpCount = 4 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

ENR_valveCount = 5 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

ENR_ElementCount = enum_anon_3 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 37

enum_anon_4 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 42

ENR_flowUnits = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 42

ENR_pressUnits = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 42

ENR_Unit = enum_anon_4 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 42

enum_anon_5 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 49

ENR_reportStart = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 49

ENR_reportStep = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 49

ENR_simDuration = 3 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 49

ENR_numPeriods = 4 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 49

ENR_Time = enum_anon_5 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 49

enum_anon_6 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 56

ENR_demand = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 56

ENR_head = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 56

ENR_pressure = 3 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 56

ENR_quality = 4 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 56

ENR_NodeAttribute = enum_anon_6 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 56

enum_anon_7 = c_int # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_flow = 1 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_velocity = 2 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_headloss = 3 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_avgQuality = 4 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_status = 5 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_setting = 6 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_rxRate = 7 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_frctnFctr = 8 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

ENR_LinkAttribute = enum_anon_7 # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 67

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 90
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_init'):
        continue
    ENR_init = _lib.ENR_init
    ENR_init.argtypes = []
    ENR_init.restype = POINTER(ENResultsAPI)
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 92
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_open'):
        continue
    ENR_open = _lib.ENR_open
    ENR_open.argtypes = [POINTER(ENResultsAPI), String]
    ENR_open.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 94
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getVersion'):
        continue
    ENR_getVersion = _lib.ENR_getVersion
    ENR_getVersion.argtypes = [POINTER(ENResultsAPI), POINTER(c_int)]
    ENR_getVersion.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 96
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getNetSize'):
        continue
    ENR_getNetSize = _lib.ENR_getNetSize
    ENR_getNetSize.argtypes = [POINTER(ENResultsAPI), ENR_ElementCount, POINTER(c_int)]
    ENR_getNetSize.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 98
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getUnits'):
        continue
    ENR_getUnits = _lib.ENR_getUnits
    ENR_getUnits.argtypes = [POINTER(ENResultsAPI), ENR_Unit, POINTER(c_int)]
    ENR_getUnits.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 100
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getTimes'):
        continue
    ENR_getTimes = _lib.ENR_getTimes
    ENR_getTimes.argtypes = [POINTER(ENResultsAPI), ENR_Time, POINTER(c_int)]
    ENR_getTimes.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 102
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getElementName'):
        continue
    ENR_getElementName = _lib.ENR_getElementName
    ENR_getElementName.argtypes = [POINTER(ENResultsAPI), ENR_ElementType, c_int, String]
    ENR_getElementName.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 109
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getEnergyUsage'):
        continue
    ENR_getEnergyUsage = _lib.ENR_getEnergyUsage
    ENR_getEnergyUsage.argtypes = [POINTER(ENResultsAPI), c_int, POINTER(c_int), POINTER(c_float)]
    ENR_getEnergyUsage.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 111
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getNetReacts'):
        continue
    ENR_getNetReacts = _lib.ENR_getNetReacts
    ENR_getNetReacts.argtypes = [POINTER(ENResultsAPI), POINTER(c_float)]
    ENR_getNetReacts.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 113
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_newOutValueSeries'):
        continue
    ENR_newOutValueSeries = _lib.ENR_newOutValueSeries
    ENR_newOutValueSeries.argtypes = [POINTER(ENResultsAPI), c_int, c_int, POINTER(c_int), POINTER(c_int)]
    ENR_newOutValueSeries.restype = POINTER(c_float)
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 115
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_newOutValueArray'):
        continue
    ENR_newOutValueArray = _lib.ENR_newOutValueArray
    ENR_newOutValueArray.argtypes = [POINTER(ENResultsAPI), ENR_ApiFunction, ENR_ElementType, POINTER(c_int), POINTER(c_int)]
    ENR_newOutValueArray.restype = POINTER(c_float)
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 118
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getNodeSeries'):
        continue
    ENR_getNodeSeries = _lib.ENR_getNodeSeries
    ENR_getNodeSeries.argtypes = [POINTER(ENResultsAPI), c_int, ENR_NodeAttribute, c_int, c_int, POINTER(c_float)]
    ENR_getNodeSeries.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 120
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getLinkSeries'):
        continue
    ENR_getLinkSeries = _lib.ENR_getLinkSeries
    ENR_getLinkSeries.argtypes = [POINTER(ENResultsAPI), c_int, ENR_LinkAttribute, c_int, c_int, POINTER(c_float)]
    ENR_getLinkSeries.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 123
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getNodeAttribute'):
        continue
    ENR_getNodeAttribute = _lib.ENR_getNodeAttribute
    ENR_getNodeAttribute.argtypes = [POINTER(ENResultsAPI), c_int, ENR_NodeAttribute, POINTER(c_float), POINTER(c_int)]
    ENR_getNodeAttribute.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 125
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getLinkAttribute'):
        continue
    ENR_getLinkAttribute = _lib.ENR_getLinkAttribute
    ENR_getLinkAttribute.argtypes = [POINTER(ENResultsAPI), c_int, ENR_LinkAttribute, POINTER(c_float), POINTER(c_int)]
    ENR_getLinkAttribute.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 128
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getNodeResult'):
        continue
    ENR_getNodeResult = _lib.ENR_getNodeResult
    ENR_getNodeResult.argtypes = [POINTER(ENResultsAPI), c_int, c_int, POINTER(c_float)]
    ENR_getNodeResult.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 130
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_getLinkResult'):
        continue
    ENR_getLinkResult = _lib.ENR_getLinkResult
    ENR_getLinkResult.argtypes = [POINTER(ENResultsAPI), c_int, c_int, POINTER(c_float)]
    ENR_getLinkResult.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 133
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_free'):
        continue
    ENR_free = _lib.ENR_free
    ENR_free.argtypes = [POINTER(c_float)]
    ENR_free.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 135
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_close'):
        continue
    ENR_close = _lib.ENR_close
    ENR_close.argtypes = [POINTER(ENResultsAPI)]
    ENR_close.restype = c_int
    break

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 137
for _lib in _libs.itervalues():
    if not hasattr(_lib, 'ENR_errMessage'):
        continue
    ENR_errMessage = _lib.ENR_errMessage
    ENR_errMessage.argtypes = [c_int, String, c_int]
    ENR_errMessage.restype = c_int
    break

__const = c_int # <command-line>: 5

# <command-line>: 8
try:
    CTYPESGEN = 1
except:
    pass

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 12
try:
    MAXFNAME = 259
except:
    pass

# C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 13
try:
    MAXID = 31
except:
    pass

ENResultsAPI = struct_ENResultsAPI # C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\epanet-output\\src\\outputapi.h: 16

# No inserted files

