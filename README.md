# ROOT file prefetching benchmarks

In short, to run the benchmarks one has to create a sample file with the `create_sample_file` macro and then
run the benchmark script as  `./benchmark some_local_file.root n` where n is the number of times to run the benchmark.

If the output is outputted in a file `out.txt`, the `avg` script can be used like `./avg out.txt` to parse the pretty printed output and calculate the average times for the benchmarks.

## Creating a file for benchmarks
A file suitable for the benchmarks can be created by the `create_sample_file.C` macro.
The script can be run like: `root -l -q "create_sample_file.C(\"sample.root\", 10000, 2000)"`.

The first argument is the name of the file to create, the second argument is the number of entries that each branch should contain and the third argument is the number of branches.

### How the file is created
~~The script creates 4 types of branches.~~  The script creates a file with branches that contain arrays of 100 elements. If 10k entries and 2k branches are specified during writing, then each branch will contain 10k of these arrays and the tree will contain 10k * 2k entries in total. This  will create a 7gb file approximately.

## Reading the files and running the benchmarks
The `benchmark` shell script can be used to run the benchmarks and read the files. It can be used like
`./benchmark file [times to run]`. The file can be either a local file or a remote file (an http:// or root:// address).
For each file, the benchmark script runs 4 benchmarks:
```
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_100)";
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_ZERO)";

root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_100)";
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_ZERO)";
```
i.e.  alternating between reads with standard or async prefetch and using a TTreeCache of either 100mb or 0.

For the benchmarks, all entries of the Tree are read, in entry-wise order. i.e. The first entry of all branches will be read, then the second entry of all branches, etc.

## Results
The average results of the benchmarks on a remote file through HTTP can be seen below:

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

It is obvious that using a cache is better than not using one at all.  Also, it can be observed that reading a remote file with asynchronous prefetching is faster than standard prefetch and it uses a bit more memory.