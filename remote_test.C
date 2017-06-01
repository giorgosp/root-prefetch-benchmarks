R__LOAD_LIBRARY(../test/libEvent.so)

#include "../test/Event.h"

void remote_test(int async = 0)
{
    cout << "Async prefetching = " << async << endl;
    gEnv->SetValue("TFile.AsyncPrefetching", async);

    TStopwatch stopwatch;
    stopwatch.Start();

    auto file = TFile::Open("http://test-gsoc.web.cern.ch/test-gsoc/sample.root");
    auto tree = (TTree *)file->Get("T");

    Long64_t nentries = tree->GetEntries();
    Long64_t i = 0;
    //  for (i = 0; i < nentries/3; i++)
    tree->GetEntry(i);

    cout << "Finished in " << stopwatch.RealTime() << " s" << endl;
}