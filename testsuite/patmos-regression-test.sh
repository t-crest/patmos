#!/bin/sh
#
# git clone and run Patmos regression test
#
LOG_DIR=`pwd`/logs

# get the Patmos source tree
rm -rf patmos
git clone git://github.com/schoeberl/patmos.git
cd patmos

# Make sure working tree is clean
# git status | grep "working directory clean" >/dev/null
# if [ $? -ne 0 ] ; then
#  echo "Working directory not clean - aborting nightly build" ; exit 1
# fi

# Setup the environment
#. testsuite/env.sh

# Run testsuite
#testsuite/rotate.sh ${LOG_DIR}
#mkdir ${LOG_DIR}/current
#testsuite/run.sh ${LOG_DIR}/current

./testsuite/run.sh > report.txt 2> report.txt

# Clean the working tree
# git -d clean

# Mail results
#LOG_FILE=${LOG_DIR}/current/report.txt
LOG_FILE=report.txt
RECIPIENTS=`cat testsuite/recipients.txt`
dos2unix ${LOG_FILE}
mail -s "[Patmos Nightly] Build report `date`" ${RECIPIENTS} < ${LOG_FILE}
