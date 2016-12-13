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
CONFIG_PML=config_ait.pml

# Function to analyse
ENTRY=audio_distortion

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
WCET_OPTS="--enable-wca --enable-trace-analysis --trace-entry $ENTRY"

#
# Execute commands
#

# - Compile
CLANG_OPTS=`platin tool-config -t clang -i $CONFIG_PML`
run patmos-clang $CLANG_OPTS -o wcet_audio wcet_audio.c -mserialize=wcet_audio.pml -mserialize-roots=$ENTRY

# - Simulate
PASIM_OPTS=`platin tool-config -t simulator -i $CONFIG_PML`
run pasim $PASIM_OPTS wcet_audio

# - Analyse
# The --outdir option is optional. If ommited, a temporary directoy will be used. Otherwise, the outdir
# must exist before the tool executed.
run mkdir -p tmp
run platin wcet $WCET_OPTS -b wcet_audio -i $CONFIG_PML -i wcet_audio.pml -e $ENTRY --outdir tmp --report
