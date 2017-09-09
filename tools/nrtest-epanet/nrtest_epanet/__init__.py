# -*- coding: utf-8 -*-

# system imports
import itertools as it

# third party imports
import header_detail_footer as hdf
import numpy as np

# project import
import epanet_reader as er


__author__ = "Michael Tryby"
__copyright__ = "None"
__credits__ = "Colleen Barr, Maurizio Cingi, Mark Gray, David Hall, Bryant McDonnell"
__license__ = "CC0 1.0 Universal"

__version__ = "0.3.0"
__date__ = "September 6, 2017"

__maintainer__ = "Michael Tryby"
__email__ = "tryby.michael@epa.gov"
__status  = "Development"


def epanet_allclose_compare(path_test, path_ref, rtol, atol):
    ''' 
    Compares results in two EPANET binary files. Using the comparison criteria 
    described in the numpy assert_allclose documentation. 
            
        (test_value - ref_value) <= atol + rtol * abs(ref_value) 
    
    Returns true if all of the results in the two binary files meet the 
    comparison criteria; otherwise, an AssertionError is thrown. 
    
    Numpy allclose is quite expensive to evaluate. Test and reference results 
    are checked to see if they are equal before being compared using the 
    allclose criteria. This reduces comparison times significantly. 
    
    Arguments: 
        path_test - path to result file being tested
        path_ref  - path to reference result file
        rtol - relative tolerance
        atol - absolute tolerance
    
    Returns: 
        True
                
    Raises:
        ValueError()
        AssertionError()
        ...
    '''
    for test, ref in it.izip(er.reader(path_test), er.reader(path_ref)):
        
        if test.size != ref.size:
            raise ValueError('Inconsistent lengths')
        
        # Skip over arrays that are equal
        if np.array_equal(test, ref):
            continue
        else:
            np.testing.assert_allclose(test, ref, rtol, atol)

    return True

# def epanet_better_compare(path_test, path_ref, rtol, atol):
#     '''
#     If you don't like assert_allclose you can add another function here. 
#     '''
#     pass

def epanet_report_compare(path_test, path_ref, rtol, atol):
    '''
    Compares results in two report files ignoring contents of header and footer.
    
    Note: Header is 11 lines with report summary turned off. This test will fail 
    if the report summary is turned on because a time stamp is being written 
    immediately after it.  
    
    Arguments: 
        path_test - path to result file being tested
        path_ref  - path to reference result file
        rtol - ignored
        atol - ignored
    
    Returns: 
        True or False
        
    Raises: 
        HeaderError()
        FooterError()
        RunTimeError()
        ...
    '''
    HEADER = 11 
    FOOTER = 3
    
    with open(path_test ,'r') as ftest, open(path_ref, 'r') as fref:
        
        for (test_line, ref_line) in it.izip(hdf.parse(ftest, HEADER, FOOTER)[1], 
                                             hdf.parse(fref, HEADER, FOOTER)[1]): 
        
            if test_line != ref_line: 
                return False

    return True 
