#!/bin/bash

# file, string to grep
function show_avg(){
    avg_res_memory=`cat $1 | grep -A 6 "$2" | grep RES| cut -d' ' -f5 | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'`
    avg_real_time=`cat $1 | grep -A 6 "$2" | grep REAL| cut -d' ' -f6 | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'`
    avg_cpu_time=`cat $1 | grep -A 6 "$2" | grep CPU| cut -d' ' -f7 | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'`
    bytes_read=`cat $1 | grep -A 6 "$2" | grep "bytes in" | cut -d' ' -f2 | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'`
    transactions=`cat $1 | grep -A 6 "$2" | grep "transactions" | cut -d' ' -f5 | awk '{ sum += $1 } END { if (NR > 0) print sum / NR }'`

    [ -z $avg_res_memory ] && return;

    echo "Read $2"
    echo "------------------------------------------------"
    echo "Avg RES memory: $avg_res_memory KB"
    echo "Avg REAL time : $avg_real_time"
    echo "Avg CPU time  : $avg_cpu_time"
    echo "Read $bytes_read bytes in $transactions transactions"
    echo ""
}

show_avg $1 "REMOTE file with STANDARD prefetch and CACHE SIZE 100000000"
show_avg $1 "REMOTE file with ASYNC prefetch and CACHE SIZE 100000000"

show_avg $1 "REMOTE file with STANDARD prefetch and CACHE SIZE 0"
show_avg $1 "REMOTE file with ASYNC prefetch and CACHE SIZE 0"

show_avg $1 "LOCAL file with STANDARD prefetch and CACHE SIZE 100000000"
show_avg $1 "LOCAL file with ASYNC prefetch and CACHE SIZE 100000000"

show_avg $1 "LOCAL file with STANDARD prefetch and CACHE SIZE 0"
show_avg $1 "LOCAL file with ASYNC prefetch and CACHE SIZE 0"