#! /bin/bash
test_networks() 
{
returnValue=0
for d in network_tests/*/ ; do
	for netfile in `ls $d*.inp`; do
		officialBinFile=${netfile%.*}.enb
		candidateBinFile=${netfile%.*}-candidate.enb
	    echo "testing $netfile with known good binary output $officialBinFile"
	    if ../build/CMake/buildproducts/bin/runepanet $netfile ${netfile%.*}-candidate.rpt $candidateBinFile
	    then
	    	echo "epanet run for $netfile SUCCESS"
	    else
	    	echo "epanet run for $netfile FAILED"
	    	returnValue=1
	    fi
	    if python ENBinaryOutDiff.py $officialBinFile $candidateBinFile
	    then
	    	echo "binary output for $netfile PASSED"
	    else
	    	echo "binary output for $netfile FAILED"
	    	returnValue=1
	    fi
	    echo "+++++"
	done
done
return $returnValue
}

cp ../build/CMake/buildproducts/bin/libENBinaryOut.* ./libENBinaryOut.so
cp ../tools/outputapi/*.py ./



test_networks
