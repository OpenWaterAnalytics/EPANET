#! /bin/bash

# 
#  before-test.sh - Prepares Travis CI worker to run epanet regression tests
#
#  Date Created: 04/04/2018
#
#  Author:       Michael E. Tryby
#                US EPA - ORD/NRMRL
#
#  Arguments: 
#    1 - relative path regression test file staging location 
#    2 - absolute path to location of software under test
#    3 - build identifier for software under test
#  
#  Note: 
#    Tests and benchmark files are stored in the epanet-example-networks repo.
#    This script retreives them using a stable URL associated with a release on 
#    GitHub and stages the files for nrtest to run. The script assumes that 
#    before-test.sh and gen-config.sh are located together in the same folder. 

SCRIPT_HOME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TEST_HOME=$1

EXAMPLES_VER="1.0.2-dev.1"
BENCHMARK_VER="220dev1"

TEST_URL="https://github.com/OpenWaterAnalytics/epanet-example-networks/archive/v${EXAMPLES_VER}.tar.gz"
BENCH_URL="https://github.com/OpenWaterAnalytics/epanet-example-networks/releases/download/v${EXAMPLES_VER}/epanet-benchmark-${BENCHMARK_VER}.tar.gz"


echo INFO: Staging files for regression testing

# create a clean directory for staging regression tests
if [ -d ${TEST_HOME} ]; then
  rm -rf ${TEST_HOME}
fi
mkdir ${TEST_HOME}
cd ${TEST_HOME}

# retrieve epanet-examples for regression testing
curl -fsSL -o examples.tar.gz ${TEST_URL}

# retrieve epanet benchmark results
curl -fsSL -o benchmark.tar.gz ${BENCH_URL}


# extract tests and benchmarks
tar xzf examples.tar.gz
ln -s epanet-example-networks-${EXAMPLES_VER}/epanet-tests tests

mkdir benchmark
tar xzf benchmark.tar.gz -C benchmark


# generate json configuration file for software under test
mkdir apps
${SCRIPT_HOME}/gen-config.sh $2 > apps/epanet-$3.json
