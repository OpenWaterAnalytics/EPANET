#! /bin/bash

#
#  run-nrtest.sh - Runs numerical regression test
#
#  Date Created: 10/16/2017
#
#  Author:       Michael E. Tryby
#                US EPA - ORD/NRMRL
#
#  Arguments:
#   1 - REF build identifier
#   2 - SUT build identifier
#   3 - relative path to location there test suite is staged
#

run-nrtest()
{

return_value=0

test_suite_path=$4


nrtest_execute_cmd="nrtest execute"
sut_app_path="apps/epanet-$3.json"
tests="tests/examples tests/exeter tests/large tests/network_one tests/small tests/tanks tests/valves"
sut_output_path="benchmark/epanet-$3"

nrtest_compare_cmd="nrtest compare"
ref_output_path="benchmark/epanet-$2"
rtol_value=0.1
atol_value=0.0


# change current directory to test_suite
cd ${test_suite_path}

# clean test benchmark results
rm -rf ${test_output_path}

echo INFO: Creating test benchmark
nrtest_command="${nrtest_execute_cmd} ${sut_app_path} ${tests} -o ${sut_output_path}"
echo INFO: "$nrtest_command"
return_value=$( $nrtest_command )

if [ $1 = 'true' ]; then
  echo
  echo INFO: Comparing test and ref benchmarks
  nrtest_command="${nrtest_compare_cmd} ${sut_output_path} ${ref_output_path} --rtol ${rtol_value} --atol ${atol_value} --output benchmark\receipt.json"
  echo INFO: "$nrtest_command"
  return_value=$( $nrtest_command )
fi

return $return_value
}

print_usage() {
   echo " "
   echo "run-nrtest.sh -  generates artifacts for SUT and performes benchmark comparison "
   echo " "
   echo "options:"
   echo "-c               don't compare SUT and REF artifacts"
   echo "-r ref_build id  REF build identifier"
   echo "-s sut build id  SUT build identifier"
   echo "-t test_path     relative path to location where test suite is staged"
   echo " "
}

# Default option values
compare='true'
ref_build_id='unknown'
sut_build_id='local'
test_path='nrtestsuite'

while getopts "cr:s:t:" flag; do
  case "${flag}" in
    c  ) compare='false' ;;
    r  ) ref_build_id=${OPTARG} ;;
    s  ) sut_build_id=${OPTARG} ;;
    t  ) test_path="${OPTARG}" ;;
    \? ) print_usage
         exit 1 ;;
  esac
done
shift $(($OPTIND - 1))


# determine ref_build_id from manifest file
if [[ $ref_build_id == 'unknown' ]] && [[ $compare == 'true' ]]; then
  description=(`cat ${test_path}/manifest.json | jq '.Application.description | splits(" ")'`)
  ref_build_id=${description[1]//\"/}
fi

# Invoke command
run_command="run-nrtest ${compare} ${ref_build_id} ${sut_build_id} ${test_path}"
echo INFO: "$run_command"
$run_command
