// The directory with libEvent.so should be in LD_LIBRARY_PATH
R__LOAD_LIBRARY(libEvent.so)

#include <iostream>

#include "Event.h"

#define TREE_KEY "T"

#define READ_QUOTA 1 //0.66 // read approximately 2/3 of entries of 2/3 of the branches. 2/3 is arbitrary.

#define LOCAL_FILE "sample.root"
#define REMOTE_FILE "http://test-gsoc.web.cern.ch/test-gsoc/sample.root"

enum Prefetching { STANDARD, ASYNC };

using namespace std;

void readBranches(TTree *tree, std::string base_branch_name, Long64_t nbranches, Long64_t nentries_per_branch)
{

    // keep all needed branches in memory
    TBranch *branches[nbranches];
    for (Long64_t b = 0; b < nbranches; b++)
    {
        std::string branch_name = base_branch_name + std::to_string(b) + ".";
        branches[b] = tree->GetBranch(branch_name.c_str());
    }

    for (Long64_t i = 0; i < nentries_per_branch; i++)
    {
        for (Long64_t b = 0; b < nbranches; b++)
        {
            auto branch = branches[b];
            tree->LoadTree(i);
            branch->GetEntry(i);
            // std::cout << "read entry " << i << " from " << branch->GetName() << std::endl;
        }
    }
}

// Read events from the tree into memory
void readTree(TTree *tree)
{
    // get a sample of entries per branch
    std::string branch_name = "ArrayBranch" + std::to_string(0) + ".";
    auto branch = tree->GetBranch(branch_name.c_str());
    Long64_t nentries_per_branch = branch->GetEntries();

    // Read some portion of the entries
    nentries_per_branch = ceil(nentries_per_branch * READ_QUOTA);

    Int_t nbranches = tree->GetNbranches();
    Long64_t nbranches_per_type = nbranches / 3; 
    
    // Read some portion of the branches
    nbranches_per_type = ceil(nbranches_per_type * READ_QUOTA);

    std::string base_branch_name;
    // base_branch_name = "SimpleBranch";
    // readBranches(tree, base_branch_name, nbranches_per_type, nentries_per_branch);
    
    base_branch_name = "ArrayBranch";
    readBranches(tree, base_branch_name, nbranches_per_type, nentries_per_branch);

    // base_branch_name = "EventBranch" +  std::to_string(99) + "_";
    // readBranches(tree, base_branch_name, nbranches_per_type, nentries_per_branch);

    // base_branch_name = "EventBranch" +  std::to_string(1) + "_";
    // readBranches(tree, base_branch_name, nbranches_per_type, nentries_per_branch);
}

void mybenchmarks(string filename, Prefetching prefetching, int cachesize)
{
    // TODO 
    // spit out benchmarks in csv format at the end of the script only
    // spit out benchmarks only if file.size == file.bytesRead (but dont read 2/3) otherwise abort benchmark
    bool isLocalFile = filename.find("https:") == 0 || filename.find("http:") == 0 || filename.find("root:") == 0;
    std::string localityLabel = isLocalFile ? "REMOTE" : "LOCAL";

    std::string prefetchLabel = (prefetching == Prefetching::STANDARD) ? "STANDARD" : "ASYNC";

    // Works only for remote files
    // https://sft.its.cern.ch/jira/browse/ROOT-7637
    gEnv->SetValue("TFile.AsyncPrefetching", (int)(prefetching == Prefetching::ASYNC));

    printf("\nRead %s file with %s prefetch and CACHE SIZE %d\n", localityLabel.c_str(), prefetchLabel.c_str(), cachesize);
    printf("------------------------------------------------\n");

    struct ProcInfo_t procinfo;
    gSystem->GetProcInfo(&procinfo);
    Long_t start_fMemResident = procinfo.fMemResident;
    // cout << "RES memory before: " << procinfo.fMemResident << " KB" << endl;
    // cout << "Virtual memory before: " << procinfo.fMemVirtual << " KB" << endl;
    
    TStopwatch stopwatch;
    stopwatch.Start();

    auto file = TFile::Open(filename.c_str());
    if (file == 0) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }

    auto tree = (TTree *)file->Get(TREE_KEY);
    tree->SetCacheSize(cachesize);
    readTree(tree);

    gSystem->GetProcInfo(&procinfo);
    cout << "RES memory :  " << procinfo.fMemResident - start_fMemResident << " KB" << endl;
    cout << "REAL time  :  " << stopwatch.RealTime() << endl;
    cout << "CPU time   :  " << stopwatch.CpuTime() << endl;
    printf("Read %lld bytes in %d transactions\n", file->GetBytesRead(), file->GetReadCalls());

    file->Close();
    delete file;
}