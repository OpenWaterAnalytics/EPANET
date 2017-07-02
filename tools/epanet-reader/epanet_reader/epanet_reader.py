'''
Created on Aug 31, 2016

@author: mtryby
'''

import ctypes
import outputapi as _lib

import numpy as np
import enum

_err_max_char = 80

class ElementType(enum.Enum):
    node = _lib.ENR_node
    link = _lib.ENR_link
    
class NodeAttribute(enum.Enum):
    demand   = _lib.ENR_demand
    head     = _lib.ENR_head
    pressure = _lib.ENR_pressure
    quality  = _lib.ENR_quality
    
class LinkAttribute(enum.Enum):
    flow       = _lib.ENR_flow
    velocity   = _lib.ENR_velocity
    headloss   = _lib.ENR_headloss
    avgQuality = _lib.ENR_avgQuality
    status     = _lib.ENR_status
    setting    = _lib.ENR_setting
    rxRate     = _lib.ENR_rxRate
    frctnFctr  = _lib.ENR_frctnFctr

class EPANET_BinaryReaderError(Exception):
    def __init__(self, error_code, error_message):
        self.warning = False
        self.args = (error_code,)
        self.message = error_message
            
    def __str__(self):
        return self.message
                    
class EPANET_BinaryReader():    
    ''' 
    Provides a minimal API used to implement the EPANET result generator. 
    '''
    def __init__(self, filename):
        self.filepath = filename
        self.ptr_api = ctypes.c_void_p
        self.ptr_resultbuff = ctypes.c_void_p
        self.bufflength = ctypes.c_long()
        self.elementAttributes = {ElementType.node: NodeAttribute,
                                 ElementType.link: LinkAttribute}
        self.getElementAttribute = {ElementType.node: _lib.ENR_getNodeAttribute, 
                                    ElementType.link: _lib.ENR_getLinkAttribute}
        
    def __enter__(self):     
        self.ptr_api = _lib.ENR_init()
        self._error_check(_lib.ENR_open(self.ptr_api, ctypes.c_char_p(self.filepath.encode())))

        # Links generally outnumber nodes so setting result buffer to number of links   
        error = ctypes.c_long()
        self.ptr_resultbuff = _lib.ENR_newOutValueArray(self.ptr_api, ctypes.c_int(_lib.ENR_getAttribute), 
            ctypes.c_int(ElementType.link.value), ctypes.byref(self.bufflength), ctypes.byref(error))
        self._error_check(error.value)
        return self

    def __exit__(self, type, value, traceback):
        _lib.ENR_free(self.ptr_resultbuff)
        self._error_check(_lib.ENR_close(self.ptr_api))

    def _error_message(self, code):
        error_code = ctypes.c_int(code)
        error_message = _lib.String(ctypes.create_string_buffer(_err_max_char))
        _lib.ENR_errMessage(error_code, error_message, _err_max_char)
        return error_message.data

    def _error_check(self, err):
        if err != 0:
            raise EPANET_BinaryReaderError(err, self._error_message(err))
            
    def report_periods(self):
        num_periods = ctypes.c_int()
        self._error_check(_lib.ENR_getTimes(self.ptr_api, _lib.ENR_numPeriods, ctypes.byref(num_periods)))
        return num_periods.value
 
    def element_attribute(self, element_type, time_index, attribute):
        time_idx = ctypes.c_long(time_index)
        attribute_idx = ctypes.c_int(attribute.value)
        count = ctypes.c_int()
        self._error_check(self.getElementAttribute[element_type](self.ptr_api, time_idx, attribute_idx, self.ptr_resultbuff, ctypes.byref(count)))        
        return np.fromiter((self.ptr_resultbuff[i] for i in range(count.value)), np.float, count.value)
