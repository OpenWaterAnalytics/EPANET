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
#   1 - test suite path
#   2 - version/build identifier 
# 

run-nrtest()
{	

return_value=0

test_suite_path=$1

nrtest_execute_cmd="nrtest execute"
test_app_path="apps/epanet-$2.json"
tests="tests/examples tests/network_one"
test_output_path="benchmark/epanet-$2"

nrtest_compare_cmd="nrtest compare"
ref_output_path="benchmark/epanet-2012"
rtol_value=0.1
atol_value=0.0


# change current directory to test_suite
cd ${test_suite_path}

# clean test benchmark results
rm -rf ${test_output_path}

echo INFO: Creating test benchmark
nrtest_command="${nrtest_execute_cmd} ${test_app_path} ${tests} -o ${test_output_path}"
echo INFO: "$nrtest_command"
if ! [ $( $nrtest_command ) ]; then
	echo  
 	echo INFO: Comparing test and ref benchmarks
 	nrtest_command="${nrtest_compare_cmd} ${test_output_path} ${ref_output_path} --rtol ${rtol_value} --atol ${atol_value}"
 	echo INFO: "$nrtest_command"
 	return_value=$( $nrtest_command ) 
else
 	echo ERROR: Test benchmark creation failed
 	exit 1
fi

return $return_value

}

run-nrtest $1 $2
