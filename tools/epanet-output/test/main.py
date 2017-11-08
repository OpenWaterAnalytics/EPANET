import enum
import outputapi as oapi

def test_get_times(file_path):
    handle = oapi.enr_init()
    oapi.enr_open(handle, file_path)
    
    time = oapi.enr_get_times(handle, oapi.Time.NUM_PERIODS)
    
    print(time)
    
    handle = oapi.enr_close()        
        
def test_get_names(file_path):
    handle = oapi.enr_init()
    oapi.enr_open(handle, file_path)
    
    name = oapi.enr_get_element_name(handle, oapi.ElementType.NODE, 10)
    
    print(name)
    
    handle = oapi.enr_close()        

def test_get_energy(file_path):
    handle = oapi.enr_init()
    oapi.enr_open(handle, file_path)
    
    result = oapi.enr_get_energy_usage(handle, 1)
    
    print(result)
    
    handle = oapi.enr_close()
    
def test_get_react(file_path):
    handle = oapi.enr_init()
    oapi.enr_open(handle, file_path)
    
    result = oapi.enr_get_net_reacts(handle)
    
    print(result)
    
    handle = oapi.enr_close()     
    
def test_get_node_attribute(file_path):
    handle = oapi.enr_init()
    oapi.enr_open(handle, file_path)
    
    array = oapi.enr_get_node_attribute(handle, 1, oapi.NodeAttribute.DEMAND)
            
    print(array)

    handle = oapi.enr_close()

def test_get_link_attribute(file_path):
    handle = oapi.enr_init()
    oapi.enr_open(handle, file_path)
    
#    // Test getNodeAttribute
    array = oapi.enr_get_link_attribute(handle, 1, oapi.LinkAttribute.FLOW)
            
    print(array)

    handle = oapi.enr_close()
    
if __name__ == "__main__":
    
    file_path = "M:\\net mydocuments\\EPA Projects\\EPAnet Examples\\net1.out"
    test_get_times(file_path)
    test_get_names(file_path)
    test_get_energy(file_path)
    test_get_react(file_path)
    test_get_node_attribute(file_path)
    test_get_link_attribute(file_path)
    