// The directory with libEvent.so should be in LD_LIBRARY_PATH
R__LOAD_LIBRARY(libEvent.so)

#include <iostream>

#include "Event.h"

#define TREE_KEY "T"

#define LOCAL_FILE "sample.root"
#define REMOTE_FILE "http://test-gsoc.web.cern.ch/test-gsoc/sample.root"

enum Prefetching { STANDARD, ASYNC };

using namespace std;

void readBranches(TTree *tree, std::string base_branch_name, Long64_t nbranches_to_read)
{
    for (Long64_t b = 0; b < nbranches_to_read; b++)
    {
        std::string branch_name = base_branch_name + std::to_string(b) + ".";
        auto branch = tree->GetBranch(branch_name.c_str());
        Long64_t branch_entries = branch->GetEntries();
        for (Long64_t i = 0; i < branch_entries; i++)
            branch->GetEntry(i);
    }
}

void readSimpleBranches(TTree *tree, Long64_t nbranches){
    // read approximately 2/3 of entries of 2/3 of the branches. 2/3 is arbitrary.
    Long64_t nbranches_to_read = nbranches*(2/3);
    std::string base_branch_name = "SimpleBranch";
    readBranches(tree, base_branch_name, nbranches_to_read);
}

void readArrayBranches(TTree *tree, Long64_t nbranches){
    // read approximately 2/3 of entries of 2/3 of the branches. 2/3 is arbitrary.
    Long64_t nbranches_to_read = nbranches*(2/3);
    std::string base_branch_name = "ArrayBranch";
    readBranches(tree, base_branch_name, nbranches_to_read);
}

void readComplexBranches(TTree *tree, Long64_t nbranches, int splitlevel){
    // read approximately 2/3 of entries of 2/3 of the branches. 2/3 is arbitrary.
    Long64_t nbranches_to_read = nbranches*(2/3);
    std::string base_branch_name = "EventBranch" +  std::to_string(splitlevel) + "_";
    readBranches(tree, base_branch_name, nbranches_to_read);
}

// Read events from the tree into memory
void readTree(TTree *tree)
{
    Event *event = 0;
    
    Long64_t nentries = tree->GetEntries();
    Int_t nbranches = tree->GetNbranches();
    int branches_per_type = nbranches / 4; // don't about the float division result

    readSimpleBranches(tree, branches_per_type);
    readArrayBranches(tree, branches_per_type);
    readComplexBranches(tree, branches_per_type, 99);
    readComplexBranches(tree, branches_per_type, 1);
}

void mybenchmarks(string filename, Prefetching prefetching, int cachesize)
{
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