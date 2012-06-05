#! /bin/bash

SERVICEURL=http://localhost:12001
NWORKERS=2
LOGLEVEL=0

# WORKER_LOGGING_LEVEL=$LOGLEVEL ./worker.pl http://128.135.125.17:$PORT swork01 ./workerlogs

RUN=`date +%m%d%H`
LOGDIR=/stage/swift-coasters/swift_coasters/logs/$RUN
WORKERDIR=/stage/swift-coasters/swift_coasters/worker_dir/worker.pl
mkdir $LOGDIR
cd $LOGDIR
#touch $LOGDIR/worker.log

for worker in $(seq -w 0 $(($NWORKERS-1))); do
 WORKER_LOGGING_LEVEL=$LOGLEVEL
$WORKERDIR $SERVICEURL swork${worker} $LOGDIR
done
ls -lt $LOGDIR/
tail $LOGDIR/worker*
