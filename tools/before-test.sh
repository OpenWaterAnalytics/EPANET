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
#    1 - (platform)
#    2 - (build id for reference)
#    3 - (build id for software under test)
#    4 - (version id for software under test)
#    5 - (relative path regression test file staging location)
#
#  Note:
#    Tests and benchmark files are stored in the epanet-example-networks repo.
#    This script retreives them using a stable URL associated with a release on
#    GitHub and stages the files for nrtest to run. The script assumes that
#    before-test.sh and gen-config.sh are located together in the same folder.


if [ -z "$1" ]; then
  unset PLATFORM;
else
  PLATFORM=$1;
fi

if [ -z "$2" ]; then
  echo "ERROR: REF_BUILD_ID must be defined"; exit 1;
else
  REF_BUILD_ID=$2;
fi

if [ -z "$3" ]; then
  SUT_BUILD_ID="local";
else
  SUT_BUILD_ID=$3;
fi

if [ -z "$4" ]; then
  SUT_VERSION="unknown";
else
  SUT_VERSION=$4; fi

if [ -z "$5" ]; then
  TEST_HOME="nrtestsuite";
else
  TEST_HOME=$5; fi


echo INFO: Staging files for regression testing


SCRIPT_HOME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_HOME="$(dirname "$SCRIPT_HOME")"


SUT_PATH=(`find "$BUILD_HOME" -name "bin" -type d`)

# TODO: determine platform

# hack to determine latest tag from GitHub
LATEST_URL="https://github.com/OpenWaterAnalytics/epanet-example-networks/releases/latest"
LATEST_TAG="$(curl -sI "${LATEST_URL}" | grep -Po 'tag\/\K(v\S+)')"

TEST_URL="https://codeload.github.com/OpenWaterAnalytics/epanet-example-networks/tar.gz/${LATEST_TAG}"
BENCH_URL="https://github.com/OpenWaterAnalytics/epanet-example-networks/releases/download/${LATEST_TAG}/benchmark-${PLATFORM}-${REF_BUILD_ID}.tar.gz"


# create a clean directory for staging regression tests
# create a clean directory for staging regression tests
if [ -d "${TEST_HOME}" ]; then
  rm -rf "${TEST_HOME}"
fi
mkdir "${TEST_HOME}"
cd "${TEST_HOME}" || exit 1


# retrieve epanet-examples for regression testing
if ! curl -fsSL -o examples.tar.gz "${TEST_URL}"; then
    echo "ERROR: curl - ${TEST_URL}" & exit 2
fi

# retrieve epanet benchmark results
if ! curl -fsSL -o benchmark.tar.gz "${BENCH_URL}"; then
    echo "ERROR: curl - ${BENCH_URL}" & exit 3
fi

# extract tests, benchmarks, and manifest
tar xzf examples.tar.gz
ln -s "epanet-example-networks-${LATEST_TAG:1}/epanet-tests" tests

mkdir benchmark
tar xzf benchmark.tar.gz -C benchmark
tar xzf benchmark.tar.gz --wildcards --no-anchored --strip-components=1 '*/manifest.json' -C .


# generate json configuration file for software under test
mkdir apps
${SCRIPT_HOME}/gen-config.sh ${SUT_PATH} ${PLATFORM} ${SUT_BUILD_ID} ${SUT_VERSION} > apps/epanet-${SUT_BUILD_ID}.json
