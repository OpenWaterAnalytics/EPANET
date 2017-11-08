#! /bin/bash

#
# run-nrtest.sh - Runs numerical regression test 
#
# Arguments:
#  1 - test suite path
#  2 - version/build identifier 
# 
run-nrtest()
{	
return_value=0

test_suite_path=$1

nrtest_execute_cmd="nrtest execute"
test_app_path="apps/epanet-$2.json"
tests="tests/examples"
test_output_path="benchmark/epanet-$2"

nrtest_compare_cmd="nrtest compare"
ref_output_path="benchmark/epanet-2012"
rtol_value=1.0
atol_value=1.0


# change current directory to test_suite
cd ${test_suite_path}

# clean test benchmark results
rm -rf ${test_output_path}

# python nrtest execute apps<app.json> tests<test.json> -o output
echo INFO: Creating test benchmark
${nrtest_execute_cmd} ${test_app_path} ${tests} -o ${test_output_path}

echo 

# python nrtest compare test_benchmark\ ref_benchmark\ --rtol --atol
echo INFO: Comparing test and ref benchmarks
return_value=$( ${nrtest_compare_cmd} ${test_output_path} ${ref_output_path} --rtol ${rtol_value} --atol ${atol_value} )

return $return_value

}

run-nrtest $1 $2