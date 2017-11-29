#
#   epanet_output_test.py
#
#   Created: 11/8/2017
#   Author: Michael E. Tryby
#           US EPA - ORD/NRMRL
#
#   Unit testing for EPANET Output API using pytest.
#

import pytest
import numpy as np

import epanet_output as oapi

from data import OUTPUT_FILE_EXAMPLE1
    
@pytest.fixture()
def enr_handle(request):    
    _handle = oapi.enr_init()
    oapi.enr_open(_handle, OUTPUT_FILE_EXAMPLE1)
    
    def enr_close():
        oapi.enr_close()
    
    request.addfinalizer(enr_close)    
    return _handle    


def test_get_times(enr_handle):
    num_periods = oapi.enr_get_times(enr_handle, oapi.Time.NUM_PERIODS)
    assert num_periods == 25


# def test_get_size(file_path):
#     handle = oapi.enr_init()
#     oapi.enr_open(handle, file_path)
#     
#     size = oapi.enr_get_net_size(handle)
#     
#     print(size)
#     
#     handle = oapi.enr_close()
#                     
# def test_get_names(file_path):
#     handle = oapi.enr_init()
#     oapi.enr_open(handle, file_path)
#     
#     name = oapi.enr_get_element_name(handle, oapi.ElementType.NODE, 10)
#     
#     print(name)
#     
#     handle = oapi.enr_close()        
# 
# def test_get_energy(file_path):
#     handle = oapi.enr_init()
#     oapi.enr_open(handle, file_path)
#     
#     result = oapi.enr_get_energy_usage(handle, 1)
#     
#     print(result)
#     
#     handle = oapi.enr_close()
#     
# def test_get_react(file_path):
#     handle = oapi.enr_init()
#     oapi.enr_open(handle, file_path)
#     
#     result = oapi.enr_get_net_reacts(handle)
#     
#     print(result)
#     
#     handle = oapi.enr_close()     
#     
def test_get_node_attribute(enr_handle):
    ref_array = np.array([ 1., 0.44407997, 0.43766347, 0.42827705, 0.41342604, 
        0.42804748, 0.44152543, 0.40502965, 0.38635802, 1., 0.96745253])

    array = oapi.enr_get_node_attribute(enr_handle, 1, oapi.NodeAttribute.QUALITY)
    assert len(array) == 11
    assert np.allclose(array, ref_array)
 
def test_get_link_attribute(enr_handle):
    ref_array = np.array([ 1848.58117676, 1220.42736816, 130.11161804, 
        187.68930054, 119.88839722, 40.46448898, -748.58111572, 478.15377808, 
        191.73458862, 30.11160851, 140.4644928, 59.53551483, 1848.58117676])
    
    array = oapi.enr_get_link_attribute(enr_handle, 1, oapi.LinkAttribute.FLOW)
    assert len(array) == 13             
    assert np.allclose(array, ref_array)
     
# if __name__ == "__main__":
#     
#     file_path = "M:\\net mydocuments\\EPA Projects\\EPAnet Examples\\net1.out"
#     test_get_times(file_path)
#     test_get_size(file_path)
#     test_get_names(file_path)
#     test_get_energy(file_path)
#     test_get_react(file_path)
#     test_get_node_attribute(file_path)
#     test_get_link_attribute(file_path)
#     