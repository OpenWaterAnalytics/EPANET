from os import path, makedirs
import sys
import json
import tarfile
import subprocess
import urllib.request

# this script:
# - downloads all example networks from the GH example network repo
# - downloads benchmark results files for all example networks
# - produces a configuration file for the nrtest suite
# - stages all files in specific locations
# - runs nrtest on the staged testing files
#
# use this either as a CI script, or on your computer using your platform-specific local testing script.

# usage:
# python3 run-test.py [test-staging-path] [compiler-name] [benchmark-version] [this-build-sha] [abs-run-epanet-path]


def run_tests(staging_path, compiler_name, benchmark_vers, this_build_sha, exe_path, rtol=0.01, atol=0.0001):
    base_url = 'https://github.com/OpenWaterAnalytics/epanet-example-networks'
    test_input_url = '{}/archive/v{}.tar.gz'.format(base_url, benchmark_vers)
    test_benchmark_url = '{}/releases/download/v{}/epanet-benchmark-{}.tar.gz'.format(base_url, benchmark_vers, compiler_name)
    config_path = path.join(staging_path, 'apps', 'epanet-{}.json'.format(this_build_sha))

    # get the test suite input files
    print('Downloading: {}'.format(test_input_url))
    test_input_fn, headers = urllib.request.urlretrieve(url=test_input_url)
    tar = tarfile.open(test_input_fn)
    tar.extractall(path=path.join(staging_path,'testsuite'))
    tar.close()

    # get the test suite benchmark binaries
    print('Downloading: {}'.format(test_benchmark_url))
    test_benchmark_fn, headers = urllib.request.urlretrieve(url=test_benchmark_url)
    tar = tarfile.open(test_benchmark_fn)
    tar.extractall(path=path.join(staging_path, 'testsuite'))
    tar.close()

    test_config = {
        "name" : "epanet",
        "version" : this_build_sha,
        "description" : "",
        "setup_script" : "",
        "exe" : exe_path
    }

    makedirs(path.dirname(config_path), exist_ok=True)
    with open(config_path, 'w') as outfile:
        json.dump(test_config, outfile, indent=2)

    test_cfg_path = path.join(staging_path, 'apps', 'epanet-{}.json'.format(this_build_sha))
    test_output_path = path.join(staging_path, 'benchmark', 'epanet-{}'.format(this_build_sha))
    test_benchmark_path = path.join(staging_path, 'benchmark', 'epanet-{}-{}'.format(compiler_name, benchmark_vers))

    test_base = path.join(staging_path, 'testsuite', 'epanet-example-networks-{}'.format(benchmark_vers), 'epanet-tests')
    tests = [
        path.join(test_base, 'examples'),
        path.join(test_base, 'exeter'),
        path.join(test_base, 'large'),
        path.join(test_base, 'network_one'),
        path.join(test_base, 'small'),
        path.join(test_base, 'tanks'),
        path.join(test_base, 'valves')
    ]
    run_nrtest = ["pipenv", "run", "nrtest"]
    # run_nrtest = ["nrtest"]
    generator_args = run_nrtest + ["execute", test_cfg_path] + tests + ["-o", test_output_path]
    comparison_args = run_nrtest + ["compare", test_output_path, test_benchmark_path, "--rtol", str(rtol), "--atol", str(atol)]

    # generate results for this build
    generated = subprocess.call(generator_args)
    if not generated:
        return 1

    # compare this build with benchmark results
    compare_success = subprocess.call(comparison_args)
    if not compare_success:
        return 1

    return 0


if __name__ == '__main__':
    # grab options from command line arguments
    print("running tests in command-line mode")
    print('options are: {}'.format(sys.argv))
    staging_path = path.join(this_dir, sys.argv[1])
    compiler_name = sys.argv[2]
    benchmark_vers = sys.argv[3]
    this_build_sha = sys.argv[4]
    exe_path = sys.argv[5]

    ret = run_tests(staging_path, compiler_name, benchmark_vers, this_build_sha, exe_path)
    sys.exit(ret)
