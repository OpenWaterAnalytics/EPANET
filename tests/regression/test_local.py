from run_test import *
from os import path
import platform

# run regression tests locally
# usage: python3 test_local.py
# easy, right??

this_dir = path.dirname(path.abspath(__file__))
benchmark_ver = '1.0.1'
benchmark_plat = ('2012vs10' if platform.system() == 'Windows' else '2012gcc')
runcmd = ('runepanet.exe' if platform.system() == 'Windows' else 'runepanet')

ret = run_tests(
    path.join(this_dir, 'tmp_testing'),
    benchmark_plat,
    benchmark_ver,
    'local-build',
    path.join(this_dir, '..', '..', 'buildproducts', 'bin', runcmd)
)

print('run_tests returned {}'.format(ret))

sys.exit(ret)
