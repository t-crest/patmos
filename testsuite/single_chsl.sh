#!/bin/bash
#
# Synopsis: ./single_chsl.sh APP
#
# Return Value:
#   0 ... test ok
#   1 ... test failed

LOG_DIR="tmp"
TEST="${1}"

mkdir -p "${LOG_DIR}"
echo "${TEST}"

# run chisel
make csim APP="${TEST}" 1>"${LOG_DIR}/cs.out" 2>"${LOG_DIR}/cs.err"
echo "EXIT $?" >> "${LOG_DIR}/cs.out"

# run high-level simulator
make hsim APP="${TEST}" 1>"${LOG_DIR}/hs.out" 2>"${LOG_DIR}/hs.err"
echo "EXIT $?" >> "${LOG_DIR}/hs.out"

# compare output
java -cp java/lib/patmos-tools.jar util.CompareChisel "${LOG_DIR}/hs.err" "${LOG_DIR}/cs.out" | \
    tee "${LOG_DIR}/comptest.out" | sed -e 's/^\(\S\)/ \1/'

# report failure or ok
if (grep '^[ ]*ok[ ]*$' "${LOG_DIR}/comptest.out" >/dev/null) ; then
    exit 0
else
    echo " failed"
    exit 1
fi
