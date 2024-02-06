#!/bin/bash
#
# Synopsis: ./single_cosim.sh APP
#
# Return Value:
#   0 ... test ok
#   1 ... test failed

LOG_DIR="tmp"
TEST="${1}"
INSTALLDIR=../local

mkdir -p "${LOG_DIR}"/`dirname ${TEST}`
echo "${TEST}"

# run chisel
make hwsim BOOTAPP="${TEST}" 1> "${LOG_DIR}/${TEST}.emu.out" 2> "${LOG_DIR}/${TEST}.emu.err"
echo "EXIT $?" >> "${LOG_DIR}/${TEST}.emu.out"

# run Pasim simulator
make swsim BOOTAPP="${TEST}" 1> "${LOG_DIR}/${TEST}.sim.out" 2> "${LOG_DIR}/${TEST}.sim.err"
echo "EXIT $?" >> "${LOG_DIR}/${TEST}.ssim.out"

# compare output
java -cp $INSTALLDIR/lib/java/patmos-tools.jar util.CompareChisel "${LOG_DIR}/${TEST}.sim.err" "${LOG_DIR}/${TEST}.emu.out" | \
    tee "${LOG_DIR}/${TEST}.comptest.out" | sed -e 's/^\(\S\)/ \1/'

# report failure or ok
if (grep '^[ ]*ok[ ]*$' "${LOG_DIR}/${TEST}.comptest.out" >/dev/null) ; then
    exit 0
else
    echo " failed"
    exit 1
fi
