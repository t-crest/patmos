#!/bin/bash
#
# Author: Stefan Hepp <stefan@stefant.org>
#
# Demo script for platin WCET analysis
#
# The -mserialize-roots option is optional, but makes the scripts run faster.
# If you specify an export root, then the trace analysis must also use that
# root function (--trace-entry).
# The platin tool-config calls are optional if the default architecture
# configuration is used.
#

function run() {
  echo $@
  $@
}

# Architecture configuration file
#CONFIG_PML=config_ait.pml
CONFIG_PML=config_de2_115.pml

# Functions to analyse
#ENTRIES=audio_distortion
ENTRIES="tte_loop tte_receive tte_receive_log handle_integration_frame handle_integration_frame_log tte_prepare_test_data tte_schedule_send tte_clock_tick tte_send_data mem_iord_byte"

FILES="../c/ethlib_tte_wcet.c ../c/ethlib/tte.c ../c/ethlib/eth_mac_driver.c ../c/ethlib/eth_patmos_io.c"

# - Compile
CLANG_OPTS=`platin tool-config -t clang -i $CONFIG_PML`

# - Simulate
#PASIM_OPTS=`platin tool-config -t simulator -i $CONFIG_PML`
PASIM_OPTS=`platin tool-config -t pasim -i $CONFIG_PML`

#
# Execute commands
#
for ENTRY in $ENTRIES; do
    echo " "
    echo "EXECUTING WCET ANALYSIS OF $ENTRY"
    echo " "

    #
    # Options for "platin wcet"
    #
    # Default mode, just run aiT
    #WCET_OPTS=

    # Enable trace analysis, do not use its results
    #WCET_OPTS="--enable-trace-analysis --trace-entry $ENTRY"

    # Use trace analysis and use its results (unsafe bounds, only for benchmarking!)
    #WCET_OPTS="--use-trace-facts --trace-entry $ENTRY"
    # Run platin WCA tool in addition to aiT, and run trace analysis
    #WCET_OPTS="--enable-wca --enable-trace-analysis --trace-entry $ENTRY"

    run patmos-clang $CLANG_OPTS -O2 -o tte $FILES -mserialize=tte.pml -mserialize-roots=$ENTRY

    # - Analyse
    # The --outdir option is optional. If ommited, a temporary directoy will be used. Otherwise, the outdir
    # must exist before the tool executed.
    run mkdir -p tmp
    run platin wcet $WCET_OPTS -b tte -i $CONFIG_PML -i tte.pml -e $ENTRY --outdir tmp -o output/$ENTRY.pml --report output/report_$ENTRY.txt --disable-ait

    run platin visualize -i output/$ENTRY.pml -o output -f $ENTRY --show-timings=platin



done
