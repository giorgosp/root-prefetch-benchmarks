// This part is available only in ROOT6. For ROOT 5, use something like
// root -l -q -e "gSystem->Load(\"../test/libEvent.so\");" mybenchmarks.C
R__LOAD_LIBRARY(../test/libEvent.so)

#include <iostream>

#include "Event.h" 

using namespace std;

void writeTree(TTree* tree, int eventsNum = 100,  int bufferSize = 16000, int splitLevel = 99)
{
    Event *event = new Event();

    tree->Branch("EventBranch", "Event", &event, bufferSize, splitLevel);

    char etype[20];
    for (Int_t i = 0; i < eventsNum; i++) {
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
        for (UChar_t m = 0; m < 10; m++) {
            event->SetMeasure(m, Int_t(gRandom->Gaus(m, m + 1)));
        }

        // fill the matrix
        for(UChar_t i0 = 0; i0 < 4; i0++) {
            for (UChar_t i1 = 0; i1 < 4; i1++) {
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
}

void readFile(bool async){}

void mybenchmarks(){
    char filename[] = "sample.root";
    char treeKey[] = "T";
    Event *event;
    int eventsNum = 100;

    // Write file
    TFile *file = new TFile(filename, "RECREATE");
    TTree *tree = new TTree(treeKey, "A tree with events for benchmarking");
    writeTree(tree, eventsNum);
    file->Close();
    delete file;
    printf("File written\n");

    // Read with standard prefetch
    printf("Read local file with standard prefetch\n");
    file = new TFile(filename, "READ");
    tree = (TTree*)file->Get(treeKey);
    tree->SetAutoFlush(0);
    tree->SetCacheSize(-1);
    event = 0;
    tree->SetBranchAddress("EventBranch", &event);
    for (int i = 0; i < eventsNum; i++) 
        tree->GetEntry(i);
    file->Close();
    printf("Read %lld bytes in %d transactions\n", file->GetBytesRead(), file->GetReadCalls());

    // Read with asynchronous prefetch
    printf("Read local file with asynchronous prefetch\n");
    file = new TFile(filename, "READ");
    tree = (TTree*)file->Get(treeKey);
    event = 0;
    tree->SetBranchAddress("EventBranch", &event);
    for (int i = 0; i < eventsNum; i++) 
        tree->GetEntry(i);
    file->Close();
    printf("Read %lld bytes in %d transactions\n", file->GetBytesRead(), file->GetReadCalls());
}