#include "root/test/Event.h"

void mybenchmarks (){
    
    // write a tree of Events to a file

    TFile *file = new TFile("big_sample.root", "RECREATE");
    TTree *tree = new TTree("T", "A tree with events for benchmarking");

    Event *event = new Event();

    int bufferSize = 16000; // bytes
    int splitLevel = 99;
    tree->Branch("EventBranch", "Event", &event, bufferSize, splitLevel);

    char etype[20];

    int eventsNum = 100;
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
    
    file->Write();
    tree->Print();
}