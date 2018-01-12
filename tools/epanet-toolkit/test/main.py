
import epanet_toolkit as tlkt

def epanet():
    [error, handle] = tlkt.EN_alloc()
    
    [error, handle] = tlkt.EN_free(handle)
    

if __name__ == "__main__":
    
    epanet()
    
