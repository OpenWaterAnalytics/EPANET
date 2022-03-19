# -*- coding: utf-8 -*-

#
#  __init__.py - nrtest_epanet module
# 
#  Author:     Michael E. Tryby
#              US EPA - ORD/NRMRL
#

'''
Numerical regression testing (nrtest) plugin for comparing EPANET binary results 
files and EPANET text based report files. 
'''

# system imports
import itertools as it

# third party imports
import header_detail_footer as hdf
import numpy as np

# project import
import nrtest_epanet.output_reader as ordr


__author__ = "Michael Tryby"
__copyright__ = "None"
__credits__ = "Colleen Barr, Maurizio Cingi, Mark Gray, David Hall, Bryant McDonnell"
__license__ = "CC0 1.0 Universal"

__version__ = "0.5.0"
__date__ = "September 6, 2017"

__maintainer__ = "Michael Tryby"
__email__ = "tryby.michael@epa.gov"
__status  = "Development"


def epanet_allclose_compare(path_test, path_ref, rtol, atol):
    ''' 
    Compares results in two EPANET binary files using the comparison criteria 
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
    for (test, ref) in it.izip(ordr.output_generator(path_test), 
                               ordr.output_generator(path_ref)):
        
        if len(test[0]) != len(ref[0]):
            raise ValueError('Inconsistent lengths')
        
        # Skip over arrays that are equal
        if np.array_equal(test[0], ref[0]):
            continue
        else:
            np.testing.assert_allclose(test[0], ref[0], rtol, atol)

    return True


def epanet_mincdd_compare(path_test, path_ref, rtol, atol):
    '''
    Compares the results of two EPANET binary files using a correct decimal
    digits (cdd) comparison criteria:

           min cdd(test, ref) >= atol

    Returns true if min cdd in the file is greater than or equal to atol,
    otherwise an AssertionError is thrown.

    Arguments:
        path_test - path to result file being testedgit
        path_ref  - path to reference result file
        rtol - ignored
        atol - minimum allowable cdd value (i.e. 3)

    Returns:
        True

    Raises:
        ValueError()
        AssertionError()

    '''
    #Turned off by L. Rossman (4/20/21)
    return True
    '''
    min_cdd = 100.0

    for (test, ref) in it.izip(ordr.output_generator(path_test), 
                               ordr.output_generator(path_ref)):

        if len(test[0]) != len(ref[0]):
            raise ValueError('Inconsistent lengths')

        # Skip over arrays that are equal
        if np.array_equal(test[0], ref[0]):
            continue
        else:
            lre = _log_relative_error(test[0], ref[0])
            idx = np.unravel_index(np.argmin(lre), lre.shape)
            if lre[idx] < min_cdd: 
                min_cdd = tmp;

            #diff = np.fabs(np.subtract(test[0], ref[0]))
            #idx = np.unravel_index(np.argmax(diff), diff.shape)

            #if diff[idx] != 0.0:
            #    tmp = - np.log10(diff[idx])

            #    if tmp < min_cdd:
            #        min_cdd = tmp;

    if np.floor(min_cdd) >= atol:
        return True
    else:
        raise AssertionError('min_cdd=%d less than atol=%g' % (min_cdd, atol))
    '''

def _log_relative_error(q, c):
    '''
    Computes log relative error, a measure of numerical accuracy. 
    
    Single precision machine epsilon is between 2^-24 and 2^-23.
    
    Reference:
    McCullough, B. D. "Assessing the Reliability of Statistical Software: Part I."
    The American Statistician, vol. 52, no. 4, 1998, pp. 358-366. 
    '''
    diff = np.subtract(q, c)
    tmp_c = np.copy(c)

    # If ref value is small compute absolute error
    tmp_c[np.fabs(tmp_c) < 1.0e-6] = 1.0    
    re = np.fabs(diff)/np.fabs(tmp_c)

    # If re is tiny set lre to number of digits
    re[re < 1.0e-7] = 1.0e-7
    # If re is very large set lre to zero
    re[re > 2.0] = 1.0

    lre = np.negative(np.log10(re))

    # If lre is negative set to zero
    lre[lre < 1.0] = 0.0

    return lre


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

# Comparison of Status Report files turned off - L.Rossman (4/20/21)

    '''
    HEADER = 10 
    FOOTER = 2
    with open(path_test ,'r') as ftest, open(path_ref, 'r') as fref:
        
        for (test_line, ref_line) in it.izip(hdf.parse(ftest, HEADER, FOOTER)[1], 
                                             hdf.parse(fref, HEADER, FOOTER)[1]): 
        
            if test_line != ref_line: 
                return False
    '''
    return True 
