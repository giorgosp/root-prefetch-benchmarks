# ROOT file prefetching benchmarks

This repo contains ROOT macros and helper scripts to compare the performance of asynchronous vs
standard data prefetching in the ROOT framework.

## Creating a file for benchmarks
A file suitable for the benchmarks can be created by the `create_sample_file.C` macro.
The script can be run like: `root -l -q "create_sample_file.C(\"sample.root\", 10000, 2000)"`.

The first argument is the name of the file to create, the second argument is the number of entries that each branch should contain and the third argument is the number of branches.

### How the file is created
~~The script creates 4 types of branches.~~  The script creates a file with branches that contain arrays of 100 elements. If 10k entries and 2k branches are specified during writing, then each branch will contain 10k of these arrays and the tree will contain 10k * 2k entries in total. This  will create a 7gb file approximately.

## Reading the files and running the benchmarks
The `benchmark` shell script can be used to run the benchmarks and read the files. It can be used like
`./benchmark.sh file [times to run]`. The file can be either a local file or a remote file (an http:// or root:// address).
For each file, the benchmark script runs 4 benchmarks:
```
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_100)";
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_ZERO)";

root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_100)";
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_ZERO)";
```
i.e.  alternating between reads with standard or async prefetch and using a TTreeCache of either 100mb or 0.

For the benchmarks, all entries of the Tree are read, in entry-wise order. i.e. The first entry of all branches will be read, then the second entry of all branches, etc.

## Output
The benchmark macro produces pretty output of the measurements, however multiple outputs of the benchmarks can be saved to a file, `out.txt` and the `avg.sh` script can be used like `./avg.sh out.txt` to parse the pretty printed output and calculate the average times for the benchmarks.

## Results

The average results of the benchmarks on a ~7gb remote file, with 10k events and 2k branches, accessed through HTTP, can be seen below:

```
Read REMOTE file with STANDARD prefetch and CACHE SIZE 100000000
------------------------------------------------
Avg RES memory: 212094 KB
Avg REAL time : 152,9
Avg CPU time  : 86
Read 7238513179 bytes in 104 transactions

Read REMOTE file with ASYNC prefetch and CACHE SIZE 100000000
------------------------------------------------
Avg RES memory: 280228 KB
Avg REAL time : 92,5833
Avg CPU time  : 89,5833
Read 7238513179 bytes in 104 transactions

Read REMOTE file with STANDARD prefetch and CACHE SIZE 0
------------------------------------------------
Avg RES memory: 129272 KB
Avg REAL time : 3944,67
Avg CPU time  : 568
Read 7238513277 bytes in 202004 transactions

Read REMOTE file with ASYNC prefetch and CACHE SIZE 0
------------------------------------------------
Avg RES memory: 155450 KB
Avg REAL time : 4507,5
Avg CPU time  : 881,5
Read 7238513277 bytes in 202004 transactions
```

It is obvious that using a ROOT Tree Cache is better than not using one at all.  Reading a remote file with asynchronous prefetching is faster than with standard prefetching, but it uses a bit more memory.
