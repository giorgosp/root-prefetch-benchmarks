#include <iostream>

#define TREE_KEY "T"

#define READ_QUOTA 1

#define LOCAL_FILE "sample.root"
#define REMOTE_FILE "http://test-gsoc.web.cern.ch/test-gsoc/sample.root"

enum Prefetching { STANDARD, ASYNC };

void mybenchmarks(string filename, Prefetching prefetching, int cachesize, int maxEvents=-1, string treeName="")
{
    // Extract some labels
    bool isLocalFile = filename.find("https:") == 0 || filename.find("http:") == 0 || filename.find("root:") == 0;
    std::string localityLabel = isLocalFile ? "REMOTE" : "LOCAL";
    std::string prefetchLabel = (prefetching == Prefetching::STANDARD) ? "STANDARD" : "ASYNC";
    printf("\nRead %s file with %s prefetch and CACHE SIZE %d\n", localityLabel.c_str(), prefetchLabel.c_str(), cachesize);
    printf("------------------------------------------------\n");
    
    // Measure time
    TStopwatch stopwatch;
    stopwatch.Start();

    gEnv->SetValue("TFile.AsyncPrefetching", (int)(prefetching == Prefetching::ASYNC));

    auto file = TFile::Open(filename.c_str());
    if (file == 0) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }

    if(treeName == "")
        treeName = TREE_KEY;
    auto tree = (TTree *)file->Get(treeName.c_str());
    tree->SetCacheSize(cachesize);
    
    // Read
    int entries = maxEvents != -1 ? maxEvents : tree->GetEntries();
    for(int i = 0 ; i < entries;i++)
        tree->GetEntry(i);

    file->Close();

    cout << "REAL time  :  " << stopwatch.RealTime() << endl;
    cout << "CPU time   :  " << stopwatch.CpuTime() << endl;
    printf("Read %lld bytes in %d transactions\n", file->GetBytesRead(), file->GetReadCalls());

    delete file;

    // Print memory stats from /proc/pid/status
    std::stringstream ss;
    ss << "cat /proc/" << gSystem->GetPid() << "/status | grep -i vm";
    system(ss.str().c_str());
}