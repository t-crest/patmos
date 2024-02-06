#!/bin/bash
# Directories containing assembler tests
cd asm
test_dirs="./inst_tests ./vliw_tests"
#for dir in ${test_dirs} ; do
#    mkdir ../tmp/${dir} 2>/dev/null
#done
# Discovering the tests in the specified directories.
test_disc=" "
for td in ${test_dirs}; do
    for f in ${td}/* ; do
        file="$f"
        extension="${file##*.}"
        if [[ "$extension" == "s" ]]; then
            # Only files with .s extension are accepted as test cases
            test_disc+="${file%.*} "
        fi
    done
done
cd ..

tests="basic minimal simple ALU ALUi ALUl compare dual_forwarding fetch_double dual_even_odd_address forward_issue ldst ld_st_test load_use load_store_stackcache spill sspill load_store_data_cache load_store_scratchpad load_store_scratchpad_new load_store_scratchpad_new2 scratchpad predication branch predicate predicated_predicate pred_issue call callr mulpipe mfsmts test test_asm test_case_plan test_mfs test_mts test_datacache"
tests+=${test_disc}

not_working=""
expect_fail=0

echo === Co-Simulation Tests ===
failed=()
for f in  ${tests}; do
	#
	echo "$not_working" | grep -w -q "$f"
	if [ "$?" -eq 0 ] ; then
		echo "$f"
		echo " Skipped"
		continue
	fi

	timeout 10m testsuite/single_cosim.sh ${f}
	result=$?
	if [ "$result" -eq 124 ] ; then
		echo " timeout"
	fi
	if [ "$result" -ne 0 ] ; then
		failed+=("${f}")
	fi
done

if [ "${#failed[@]}" -ne 0 ] ; then
	echo "Failed tests: ${failed[@]}" >&2
else
	echo "All tests ok"
fi

if [ ${#failed[@]} -ne $expect_fail ] ; then
	exit 1
else
	exit 0
fi
