#!/bin/bash

CACHESIZE_100=100*1000*1000 # 100 MB
CACHESIZE_ZERO=0

function run(){
    for ((i=0; i<$2; i++))
    do
        root -l -q -b "$1"
    done
}

file=$1
times=$2 
[ -z "$times" ] && times=1

# root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_100)";
run "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_100)" $times
run "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_ZERO)" $times

run "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_100)" $times
run "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_ZERO)" $times
