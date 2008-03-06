#!/bin/bash
SVNREV=`svn info "$1" | grep Revision | awk -F": " '{print $2}'`
echo $2$3${SVNREV}$4
