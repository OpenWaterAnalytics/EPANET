
#ifndef DLLEXPORT_H
#define DLLEXPORT_H

#ifdef SHARED_EXPORTS_BUILT_AS_STATIC
#  define DLLEXPORT
#  define EPANET_NO_EXPORT
#else
#  ifndef DLLEXPORT
#    ifdef epanet_EXPORTS
        /* We are building this library */
#      define DLLEXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define DLLEXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef EPANET_NO_EXPORT
#    define EPANET_NO_EXPORT 
#  endif
#endif

#ifndef EPANET_DEPRECATED
#  define EPANET_DEPRECATED __declspec(deprecated)
#endif

#ifndef EPANET_DEPRECATED_EXPORT
#  define EPANET_DEPRECATED_EXPORT DLLEXPORT EPANET_DEPRECATED
#endif

#ifndef EPANET_DEPRECATED_NO_EXPORT
#  define EPANET_DEPRECATED_NO_EXPORT EPANET_NO_EXPORT EPANET_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef EPANET_NO_DEPRECATED
#    define EPANET_NO_DEPRECATED
#  endif
#endif

#endif /* DLLEXPORT_H */
