// The directory with libEvent.so should be in LD_LIBRARY_PATH
R__LOAD_LIBRARY(libEvent.so)

#include <iostream>

#include "Event.h"

#define TREE_KEY "T"

#define LOCAL_FILE "sample.root"
#define REMOTE_FILE "http://test-gsoc.web.cern.ch/test-gsoc/sample.root"

enum Prefetching { STANDARD, ASYNC };

using namespace std;

// Read events from the tree into memory
void readTree(TTree *tree)
{
    Event *event = 0;
    // TODO read from branches
    // tree->SetBranchAddress("EventBranch", &event);
    auto nentries = tree->GetEntries();
    for (Long64_t i = 0; i < nentries; i++){
        // gSystem->Sleep(200)
        tree->GetEntry(i);
    }
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