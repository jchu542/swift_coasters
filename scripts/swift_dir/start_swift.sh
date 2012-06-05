#!/bin/bash

if [ $1==NULL ] ; then
SCRIPT="/stage/swift-coasters/swift-0.93/cog/modules/swift/examples/tutorial/hello.swift"

else 
SCRIPT=$1
fi

RUN=`date +%m%d%H`
LOGDIR=/stage/swift-coasters/swift_coasters/logs/$RUN/
mkdir $LOGDIR
#strace -f -e trace=$TRACE -o ${LOGDIR}strace-swift-$RUN.log 
swift -sites.file sites.xml -tc.file tc.data $SCRIPT -config cf -logfile ${LOGDIR}hello-$RUN.log
