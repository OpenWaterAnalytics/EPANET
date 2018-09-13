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


def _binary_diff(path_test, path_ref, min_cdd):
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
                _print_diff(idx, lre, test, ref)

    return 


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
    
    
def _print_diff(idx, lre, test, ref):
    
    idx_val = (idx[0], ref[1])
    test_val = (test[0][idx[0]])
    ref_val = (ref[0][idx[0]])
    diff_val = (test_val - ref_val)
    lre_val = (lre[idx[0]])
    
    print("Idx: %s\nSut: %e  Ref: %e  Diff: %e  LRE: %.2f\n" 
          % (idx_val, test_val, ref_val, diff_val, lre_val))


def report(args):
    _binary_diff(args.test, args.ref, args.mincdd)


if __name__ == '__main__':
    from argparse import ArgumentParser
    
    parser = ArgumentParser(description='EPANET benchmark difference reporting')
    parser.set_defaults(func=report)
    parser.add_argument('-t', '--test', default=None, 
                        help='Path to test benchmark')
    parser.add_argument('-r', '--ref', default=None, 
                        help='Path to reference benchmark')
    parser.add_argument('-mc', '--mincdd', type=int, default=3, 
                        help='Minimum correct decimal digits')
    
    args = parser.parse_args()
    args.func(args)
    