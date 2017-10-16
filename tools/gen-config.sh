#! /bin/bash

# 
#  gen-config.sh - Generates nrtest app configuration file for test executable
#
#  Arguments:
#    1 - absolute path to test executable
#    2 - test executable version number
#    3 - build description
#

# Removes a leading '/c' from file path if present
abs_build_path="$( echo "$1" | sed -e 's#/c##' )"
version=""
build_description=""

cat<<EOF
{
    "name" : "epanet",
    "version" : "${version}",
    "description" : "${build_description}", 
    "setup_script" : "",
    "exe" : "${abs_build_path}/runepanet.exe"
}
EOF
