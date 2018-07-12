# -*- coding: utf-8 -*-

#
#  report_diff.py 
# 
#  Date Created: July 11, 2018
#
#  Author:     Michael E. Tryby
#              US EPA - ORD/NRMRL
#

# system imports
import itertools as it

# third party imports
import numpy as np

# project imports
import nrtest_epanet.output_reader as ordr
from numpy.f2py.auxfuncs import hasinitvalue
from numpy.tests.test_numpy_version import test_valid_numpy_version


def report_diff(path_test, path_ref, min_cdd):
    for (test, ref) in it.izip(ordr.output_generator(path_test), 
                               ordr.output_generator(path_ref)):
        
        if len(test[0]) != len(ref[0]):
            raise ValueError('Inconsistent lengths')
        
        # Skip over arrays that are equal
        if np.array_equal(test[0], ref[0]):
            continue
        else:
            lre = log_relative_error(test[0], ref[0])
            idx = np.unravel_index(np.argmin(lre), lre.shape)

            if lre[idx] < min_cdd: 
                print_diff(idx, lre, test, ref)

    return 


def log_relative_error(q, c):
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

    return np.negative(np.log10(re))
    
    
def print_diff(idx, lre, test, ref):
    
    idx_val = (idx[0], ref[1])
    test_val = (test[0][idx[0]])
    ref_val = (ref[0][idx[0]])
    diff_val = (test_val - ref_val)
    lre_val = (lre[idx[0]])
    
    print("Idx: %s\nSut: %f  Ref: %f  Diff: %f  LRE: %f\n" 
          % (idx_val, test_val, ref_val, diff_val, lre_val))
    