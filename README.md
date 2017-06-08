# ROOT file prefetching benchmarks

In short, to run the benchmarks one has to create a sample file with the `create_sample_file` macro and then
run the benchmark script as  `./benchmark some_local_file.root 2` where 2 is the number that is benchmark
will be executed.

## Creating a file for benchmarks
A file suitable for the benchmarks can be created by the `create_sample_file.C` macro.
The script can be run like: `root -l -q "create_sample_file.C(\"sample.root\", 10000, 2000)"`.

The first argument is the name of the file to create, the second argument is the number of events that the
file should contain and the third argument is the number of branches.

### How the file is created
The script creates 4 types of branches. So, if the third argument is 2000, 500 branches of each type will
be created. The number of events is equally divided the number of types and then to the branches of each
type. The number of events in the tree will never surpass the number given as argument to the 
`create_sample_file` macro. The types of the branches are:
- Simple branches: Each of these branches will contain 4 simple fields. 2 float and 2 integer fields. The
    branch is created like so: `tree->Branch(branchName, &event, "fTemperature/F:fNtrack/I:fNseg:fFlag/i");`
- Array branches: Each of these branches will contain an array of 1000 float fields.
- Complex branches with split level 99: These branches will be filled with objects of the Event class.
- Complex branches with split level 1: These branches will be filled with objects of the Event class.

### File size
Running the command `root -l -q "create_sample_file.C(\"sample.root\", 20000, 2000)"` which creates
a file with 20K events and 2K branches, the file size is about 330MB. So, the way of writing a file
doesn't create large files.


## Reading the files and running the benchmarks
The `benchmark` shell script can be used to run the benchmarks and read the files. It can be used like
`./benchmark file1 [times to run]`.
For each file, the benchmark script runs 4 benchmarks:
```
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_100)";
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::STANDARD, $CACHESIZE_ZERO)";

root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_100)";
root -l -q -b "mybenchmarks.C(\"$1\", Prefetching::ASYNC, $CACHESIZE_ZERO)";
```
i.e. reads the files with standard or async prefetch and with TTreeCache of 100mb or 0.

To read the files, the benchmark script reads the 2/3 of branches of each branch type, and for each branch
it reads the 2/3 of its entries, i.e it will read the 2/3 of the entries of the 2/3 of the simple branches,
2/3 of the entries of the 2/3 of the array branches, etc.

This snippet shows how the entries are read. It will run for the 2/3 of the branches of each type.
```
for (Long64_t i = 0; i < nentries_per_branch; i++)
    {
        for (Long64_t b = 0; b < nbranches; b++)
        {
            std::string branch_name = base_branch_name + std::to_string(b) + ".";
            auto branch = tree->GetBranch(branch_name.c_str());
            branch->GetEntry(i);
        }
    }
```

## Event class
The scripts depend on the $ROOTSYS/test/Event.h class. To make it available in the benchmarks one 
should run `make` inside the `$ROOTSYS/test/` directory and append this directory to the `LD_LIBRARY_PATH`
environment variable.