#!/bin/bash

#TRACE=$1
#PORT=$2
#LPORT=$3

RUN=`date +%m%d%H`
LOGDIR=/stage/swift-coasters/swift_coasters/logs/$RUN
mkdir $LOGDIR
touch $LOGDIR/worker-$RUN.log
touch $LOGDIR/coaster-$RUN.log
strace -f -e trace=sendto,recvfrom -s 10000 -o ${LOGDIR}/strace-coaster-$RUN.log coaster-service -passive -nosec -p 12000 -localport 12001 | tee -a ${LOGDIR}/coaster-$RUN.log
