#!/bin/bash
#
# Synopsis: ./single_chsl.sh APP
#
# Return Value:
#   0 ... test ok
#   1 ... test failed
#   2 ... exception

LOG_DIR="tmp"
TEST="${1}"

errors=()
mkdir -p "${LOG_DIR}"
echo "${TEST}"

# run chisel
make csim APP="${TEST}" 1>"${LOG_DIR}/cs.out" 2>"${LOG_DIR}/cs.err" \
    || errors+=("cs")

# run high-level simulator
make     hsim APP="${TEST}" 1>"${LOG_DIR}/hs.out" 2>"${LOG_DIR}/hs.err"
# -- error codes correspond to patmos exceptions

# compare output
java -cp java/lib/patmos-tools.jar util.CompareChisel "${LOG_DIR}/hs.err" "${LOG_DIR}/cs.out" | \
    tee "${LOG_DIR}/comptest.out" | sed -e 's/^\(\S\)/ \1/'

# report errors, failure or ok
if [ "${#errors[@]}" -ne 0 ] ; then
    echo "Error: ${TEST}: Failed to execute test (${errors[@]})"
    for f in "${errors[@]}" ; do cat "${LOG_DIR}"/$f.err ; done
    exit 2
elif (grep '^[ ]*ok[ ]*$' "${LOG_DIR}/comptest.out" >/dev/null) ; then
    exit 0
else
    echo " failed"
    exit 1
fi
