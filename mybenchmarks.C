// This part is available only in ROOT6. For ROOT 5, use something like
// root -l -q -e "gSystem->Load(\"../test/libEvent.so\");" mybenchmarks.C
R__LOAD_LIBRARY(../test/libEvent.so)

#include <iostream>

#include "../test/Event.h"

#define TREE_KEY "T"

// #define LOCAL_FILE "sample.root"
// #define REMOTE_FILE "http://test-gsoc.web.cern.ch/test-gsoc/sample.root"

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
    string localityLabel = isLocalFile ? "REMOTE" : "LOCAL";

    string prefetchLabel = (prefetching == Prefetching::STANDARD) ? "STANDARD" : "ASYNC";

    // Works only for remote files
    // https://sft.its.cern.ch/jira/browse/ROOT-7637
    gEnv->SetValue("TFile.AsyncPrefetching", (int)(prefetching == Prefetching::ASYNC));

    cout << "\nRead " << localityLabel << " file with " << prefetchLabel << " prefetch" << endl;
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
    // TODO
    // auto-flush setting of 100 and read it back using a TTreeCache of the same size.
    // tree->SetCacheSize();
    readTree(tree);

    gSystem->GetProcInfo(&procinfo);
    cout << "RES memory :  " << procinfo.fMemResident - start_fMemResident << " KB" << endl;
    cout << "REAL time  :  " << stopwatch.RealTime() << endl;
    cout << "CPU time   :  " << stopwatch.CpuTime() << endl;
    printf("Read %lld bytes in %d transactions\n", file->GetBytesRead(), file->GetReadCalls());

    file->Close();
    delete file;
}