'''
Created on Aug 31, 2016

@author: mtryby
'''

import outputapi as oapi
                    
class EPANET_BinaryReader():    
    ''' 
    Provides a minimal API used to implement the EPANET result generator. 
    '''
    def __init__(self, filename):
        self.filepath = filename
        self.handle = None
        self.elementAttributes = {oapi.ElementType.NODE: oapi.NodeAttribute,
                                 oapi.ElementType.LINK: oapi.LinkAttribute}
        self.getElementAttribute = {oapi.ElementType.NODE: oapi.enr_get_node_attribute, 
                                    oapi.ElementType.LINK: oapi.enr_get_link_attribute}
        
    def __enter__(self):     
        self.handle = oapi.enr_init()
        oapi.enr_open(self.handle, self.filepath.encode())
        return self

    def __exit__(self, type, value, traceback):
        self.handle = oapi.enr_close()
            
    def report_periods(self):
        return oapi.enr_get_times(self.handle, oapi.Time.NUM_PERIODS)
 
    def element_attribute(self, element_type, time_index, attribute):
        return self.getElementAttribute[element_type](self.handle, time_index, attribute)
