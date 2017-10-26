
import outputapi as oapi 
from .epanet_reader import *

def reader(path_ref):
  
    with EPANET_BinaryReader(path_ref) as br:

        for period_index in range(0, br.report_periods()):
            for element_type in oapi.ElementType:
                for attribute in br.elementAttributes[element_type]:
        
                    yield br.element_attribute(element_type, period_index, attribute)
