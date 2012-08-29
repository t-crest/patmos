#!/bin/sh
#
# git clone and run Patmos regression test
#

LOG_DIR=`pwd`/logs
LOG_FILE="${LOG_DIR}/report.txt"
MAIL_RESULTS=1

# prepare logdir
mkdir -p "${LOG_DIR}"

PATH=.:$PATH:/opt/modelsim_ase/bin
export PATH 
# get the Patmos source tree
rm -rf patmos
git clone git://github.com/t-crest/patmos.git
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

# Clean the working tree
# git -d clean

bash testsuite/run.sh > "${LOG_FILE}" 2>&1

# Mail results
if [ "${MAIL_RESULTS}" -eq 1 ] ; then
    RECIPIENTS=`cat testsuite/recipients.txt`
    dos2unix ${LOG_FILE}
    mail -s "[Patmos Nightly] Build report `date`" ${RECIPIENTS} < ${LOG_FILE}
fi
