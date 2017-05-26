// This part is available only in ROOT6. For ROOT 5, use something like
// root -l -q -e "gSystem->Load(\"../test/libEvent.so\");" mybenchmarks.C
R__LOAD_LIBRARY(../test/libEvent.so)

#include <iostream>

#include "Event.h"

#define TREE_KEY "T"

#define LOCAL_FILE "data/sample.root"
// A file with 1000 events ~30mb
#define REMOTE_FILE "https://drive.google.com/uc?export=download&id=0B5eWmoo5R47zck1WMUFaTjRWVE0"

enum Locality
{
    LOCAL,
    REMOTE
};

enum Prefetching
{
    STANDARD,
    ASYNC
};

using namespace std;


struct ProcInfo_t procinfo;

void writeTree(TTree *tree, int eventsNum = 100, int bufferSize = 16000, int splitLevel = 99)
{
    Event *event = new Event();

    tree->Branch("EventBranch", "Event", &event, bufferSize, splitLevel);

    char etype[20];
    for (Int_t i = 0; i < eventsNum; i++)
    {
        Float_t sigmat, sigmas;
        gRandom->Rannor(sigmat, sigmas);
        Int_t ntrack = Int_t(600 + 600 * sigmat / 120.);
        Float_t random = gRandom->Rndm(1);
        sprintf(etype, "type%d", i % 5);
        event->SetType(etype);
        event->SetHeader(i, 200, 960312, random);
        event->SetNseg(Int_t(10 * ntrack + 20 * sigmas));
        event->SetNvertex(Int_t(1 + 20 * gRandom->Rndm()));
        event->SetFlag(UInt_t(random + 0.5));
        event->SetTemperature(random + 20.);
        for (UChar_t m = 0; m < 10; m++)
        {
            event->SetMeasure(m, Int_t(gRandom->Gaus(m, m + 1)));
        }

        // fill the matrix
        for (UChar_t i0 = 0; i0 < 4; i0++)
        {
            for (UChar_t i1 = 0; i1 < 4; i1++)
            {
                event->SetMatrix(i0, i1, gRandom->Gaus(i0 * i1, 1));
            }
        }
        // create and fill the Track objects
        for (Int_t t = 0; t < ntrack; t++)
            event->AddTrack(random);

        // Fill the tree
        tree->Fill();

        // Clear before reloading event
        event->Clear();
    }

    // Save tree header
    tree->Write();

    delete event;
}

// Write a tree to a local file
void createFileLocal(int eventsNum)
{
    TFile *file = new TFile(LOCAL_FILE, "RECREATE");
    TTree *tree = new TTree(TREE_KEY, "A tree with events for benchmarking");
    writeTree(tree, eventsNum);
    file->Close();
    cout << "Written " << eventsNum << " events in " << file->GetBytesWritten() << " bytes" << endl;
    delete file;
}

// Read events from the tree into memory
void readTree(TTree *tree)
{
    Event *event = 0;
    tree->SetBranchAddress("EventBranch", &event);
    auto eventsNum = tree->GetEntries();
    for (int i = 0; i < eventsNum; i++)
        tree->GetEntry(i);
}

void benchmark(Locality locality, Prefetching prefetching, string filename)
{
    string localityLabel;
    if (locality == LOCAL)
        localityLabel = "LOCAL";
    else
        localityLabel = "REMOTE";

    string prefetchLabel;
    if (prefetching == STANDARD)
        prefetchLabel = "STANDARD";
    else
        prefetchLabel = "ASYNC";

    cout << "\nRead " << localityLabel << " file with " << prefetchLabel << " prefetch" << endl;
    printf("------------------------------------------------\n");

    gSystem->GetProcInfo(&procinfo);
    cout << "RES memory before: " << procinfo.fMemResident << " KB" << endl;

    auto file = TFile::Open(filename.c_str());
    auto tree = (TTree *)file->Get(TREE_KEY);

    if (prefetching == STANDARD){
        tree->SetAutoFlush(0);
        tree->SetCacheSize(-1);
    }

    readTree(tree);

    gSystem->GetProcInfo(&procinfo);
    cout << "RES memory after:  " << procinfo.fMemResident << " KB" << endl;
    printf("Read %lld bytes in %d transactions\n", file->GetBytesRead(), file->GetReadCalls());

    file->Close();
    delete file;
}

void mybenchmarks(int eventsNum = 100)
{
    // todo measure cpu time
    gSystem->GetProcInfo(&procinfo);
    cout << "RES memory at start of script " << procinfo.fMemResident << " KB" << endl;

    createFileLocal(eventsNum);
    benchmark(LOCAL, STANDARD, LOCAL_FILE);
    benchmark(LOCAL, ASYNC, LOCAL_FILE);
    
    benchmark(REMOTE, STANDARD, REMOTE_FILE);
    benchmark(REMOTE, ASYNC, REMOTE_FILE);
}