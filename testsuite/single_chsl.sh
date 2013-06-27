#!/bin/bash
#
# Synopsis: ./single_chsl.sh APP
#
# Return Value:
#   0 ... test ok
#   1 ... test failed

LOG_DIR="tmp"
TEST="${1}"

mkdir -p "${LOG_DIR}"/`dirname ${TEST}`
echo "${TEST}"

# run chisel
make csim BOOTAPP="${TEST}" 1> "${LOG_DIR}/${TEST}.cs.out" 2> "${LOG_DIR}/${TEST}.cs.err"
echo "EXIT $?" >> "${LOG_DIR}/${TEST}.cs.out"

# run high-level simulator
make hsim BOOTAPP="${TEST}" 1> "${LOG_DIR}/${TEST}.hs.out" 2> "${LOG_DIR}/${TEST}.hs.err"
echo "EXIT $?" >> "${LOG_DIR}/${TEST}.hs.out"

# compare output
java -cp java/lib/patmos-tools.jar util.CompareChisel "${LOG_DIR}/${TEST}.hs.err" "${LOG_DIR}/${TEST}.cs.out" | \
    tee "${LOG_DIR}/${TEST}.comptest.out" | sed -e 's/^\(\S\)/ \1/'

# report failure or ok
if (grep '^[ ]*ok[ ]*$' "${LOG_DIR}/${TEST}.comptest.out" >/dev/null) ; then
    exit 0
else
    echo " failed"
    exit 1
fi
