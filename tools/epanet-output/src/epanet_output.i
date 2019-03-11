/*
 *  epanet_output.i - SWIG interface description file for EPANET Output API
 *
 *  Created:    9/20/2017
 *
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 *
*/
%module epanet_output
%{
#include "errormanager.h"
#include "messages.h"
#include "epanet_output.h"

#define SWIG_FILE_WITH_INIT
%}

%include "typemaps.i"

/* DEFINE AND TYPEDEF MUST BE INCLUDED */
#define MAXMSG 53

typedef void* ENR_Handle;


%include "epanet_output_enums.h"


/* TYPEMAPS FOR OPAQUE POINTER */
/* Used for functions that output a new opaque pointer */
%typemap(in, numinputs=0) ENR_Handle* p_handle_out (ENR_Handle retval)
{
 /* OUTPUT in */
    retval = NULL;
    $1 = &retval;
}
/* used for functions that take in an opaque pointer (or NULL)
and return a (possibly) different pointer */
%typemap(argout) ENR_Handle* p_handle_out
{
 /* OUTPUT argout */
    %append_output(SWIG_NewPointerObj(SWIG_as_voidptr(retval$argnum), $1_descriptor, 0));
}
/* No need for special IN typemap for opaque pointers, it works anyway */


/* TYPEMAP FOR IGNORING INT ERROR CODE RETURN VALUE */
%typemap(out) int {
    $result = Py_None;
    Py_INCREF($result);
}

/* TYPEMAPS FOR INT ARGUMENT AS RETURN VALUE */
%typemap(in, numinputs=0) int* int_out (int temp) {
    $1 = &temp;
}
%typemap(argout) int* int_out {
    %append_output(PyInt_FromLong(*$1));
}

/* TYPEMAP FOR MEMORY MANAGEMENT AND ENCODING OF STRINGS */
%typemap(in, numinputs=0)char** string_out (char* temp), int* slen (int temp){
   $1 = &temp;
}
%typemap(argout)(char** string_out, int* slen) {
    if (*$1) {
        PyObject* o;
        o = PyUnicode_FromStringAndSize(*$1, *$2);

        $result = SWIG_Python_AppendOutput($result, o);
        free(*$1);
    }
}

/* TYPEMAPS FOR MEMORY MANAGEMNET OF FLOAT ARRAYS */
%typemap(in, numinputs=0)float** float_out (float* temp), int* int_dim (int temp){
   $1 = &temp;
}
%typemap(argout) (float** float_out, int* int_dim) {
    if (*$1) {
      PyObject *o = PyList_New(*$2);
      int i;
      float* temp = *$1;
      for(i=0; i<*$2; i++) {
        PyList_SetItem(o, i, PyFloat_FromDouble((double)temp[i]));
      }
      $result = SWIG_Python_AppendOutput($result, o);
      free(*$1);
    }
}

/* TYPEMAPS FOR MEMORY MANAGEMENT OF INT ARRAYS */
%typemap(in, numinputs=0)int** int_out (long* temp), int* int_dim (int temp){
   $1 = &temp;
}
%typemap(argout) (int** int_out, int* int_dim) {
    if (*$1) {
      PyObject *o = PyList_New(*$2);
      int i;
      long* temp = *$1;
      for(i=0; i<*$2; i++) {
        PyList_SetItem(o, i, PyInt_FromLong(temp[i]));
      }
      $result = SWIG_Python_AppendOutput($result, o);
      free(*$1);
    }
}

/* TYPEMAP FOR ENUMERATED TYPES */
%typemap(in) EnumeratedType (int val, int ecode = 0) {
    if (PyObject_HasAttrString($input,"value")) {
        PyObject* o;
        o = PyObject_GetAttrString($input, "value");
        ecode = SWIG_AsVal_int(o, &val);
    }
    else {
        SWIG_exception_fail(SWIG_ArgError(ecode), "in method '" "$symname" "', argument " "$argnum"" of type '" "$ltype""'");
    }

    $1 = ($1_type)(val);
}
%apply EnumeratedType {ENR_ElementType, ENR_Units, ENR_Time, ENR_NodeAttribute, ENR_LinkAttribute}


/* RENAME FUNCTIONS PYTHON STYLE */
%rename("%(undercase)s") "";

/* INSERTS CUSTOM EXCEPTION HANDLING IN WRAPPER */
%exception
{
	char* err_msg;
	ENR_clearError(arg1);
    $function
    if (ENR_checkError(arg1, &err_msg))
    {
        PyErr_SetString(PyExc_Exception, err_msg);
    	SWIG_fail;
    }
}
/* INSERT EXCEPTION HANDLING FOR THESE FUNCTIONS */
int  ENR_open(ENR_Handle p_handle, const char* path);

int  ENR_getVersion(ENR_Handle p_handle, int* int_out);
int  ENR_getNetSize(ENR_Handle p_handle, int** int_out, int* int_dim);
int  ENR_getUnits(ENR_Handle p_handle, ENR_Units t_enum, int* int_out);
int  ENR_getTimes(ENR_Handle p_handle, ENR_Time t_enum, int* int_out);
int  ENR_getElementName(ENR_Handle p_handle, ENR_ElementType t_enum,
		int elementIndex, char** string_out, int* slen);
int  ENR_getEnergyUsage(ENR_Handle p_handle, int pumpIndex,
		int* int_out, float** float_out, int* int_dim);
int  ENR_getNetReacts(ENR_Handle p_handle, float** float_out, int* int_dim);


int  ENR_getNodeAttribute(ENR_Handle p_handle, int periodIndex,
        ENR_NodeAttribute t_enum, float** float_out, int* int_dim);
int  ENR_getLinkAttribute(ENR_Handle p_handle, int periodIndex,
        ENR_LinkAttribute t_enum, float** float_out, int* int_dim);
%exception;

/* NO EXCEPTION HANDLING FOR THESE FUNCTIONS */
int  ENR_init(ENR_Handle* p_handle_out);
int  ENR_close(ENR_Handle* p_handle_out);
void  ENR_free(void** array);

void  ENR_clearError(ENR_Handle p_handle);
int  ENR_checkError(ENR_Handle p_handle, char** msg_buffer);


/* CODE ADDED DIRECTLY TO SWIGGED INTERFACE MODULE */
%pythoncode%{
import enum

class ElementType(enum.Enum):
    NODE = ENR_node
    LINK = ENR_link

class Units(enum.Enum):
    FLOW_UNIT = ENR_flowUnits
    PRESS_UNIT = ENR_pressUnits

class Time(enum.Enum):
    REPORT_START = ENR_reportStart
    REPORT_STEP  = ENR_reportStep
    SIM_DURATION = ENR_simDuration
    NUM_PERIODS  = ENR_numPeriods

class NodeAttribute(enum.Enum):
    DEMAND   = ENR_demand
    HEAD     = ENR_head
    PRESSURE = ENR_pressure
    QUALITY  = ENR_quality

class LinkAttribute(enum.Enum):
    FLOW        = ENR_flow
    VELOCITY    = ENR_velocity
    HEADLOSS    = ENR_headloss
    AVG_QUALITY = ENR_avgQuality
    STATUS      = ENR_status
    SETTING     = ENR_setting
    RX_RATE     = ENR_rxRate
    FRCTN_FCTR  = ENR_frctnFctr
%}
