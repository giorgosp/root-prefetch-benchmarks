R__LOAD_LIBRARY(../test/libEvent.so)

#include <iostream>
#include "../test/Event.h"

#define TREE_KEY "T"


void fillSimpleBranches(TTree *tree, int nbranches, int nentries){
    cout << "Writing simple branches with " << nbranches << " branches" << " of " << nentries << " entries each..." << endl;

    TString baseBranchName = "SimpleBranch";

    Event *event = new Event();

    for (int b = 0; b < nbranches; b++) {
        // Create branches SimpleBranch0.  SimpleBranch1. etc
        TString branchName = baseBranchName + (Long_t)b + ".";
        auto branch = tree->Branch(branchName, &event, "fTemperature/F:fNtrack/I:fNseg:fFlag/i");

        for (int i = 0; i < nentries; i++) {
            Float_t sigmat, sigmas;
            gRandom->Rannor(sigmat, sigmas);
            Int_t ntrack = Int_t(600 + 600 * sigmat / 120.);
            
            auto random = gRandom->Rndm();
            event->SetTemperature(random + 20.);
            event->SetNtrack(ntrack);
            event->SetNseg(Int_t(10 * ntrack + 20 * sigmas));
            event->SetFlag(UInt_t(random + 0.5));

            branch->Fill();
            event->Clear();
        }
    }
    delete event;
}

void fillArrayBranches(TTree *tree, int nbranches, int nentries){
    cout << "Writing array branches with " << nbranches << " branches" << " of " << nentries << " entries each..." << endl;

    // Create  branches whose leaves hold arrays
    TString baseBranchName = "ArrayBranch";

    int array_len = 1000;
    Float_t  f[array_len];

    for (int b = 0; b < nbranches; b++) {
        // Create branches ArrayBranch.  ArrayBranch. etc
        TString branchName = baseBranchName + (Long_t)b + ".";
        auto branch = tree->Branch(branchName, f, "f[10]/F");
        
        for (int i = 0; i < nentries; i++) {
            for (int i = 0; i < array_len; i++)
                f[i] = gRandom->Rndm();
            
            branch->Fill();
        }
    }
}

void fillComplexBranches(TTree *tree, int nbranches, int nentries, int splitlevel){
    // Create  branches whose leaves hold Event objects
    cout << "Writing complex branches with " << nbranches << " branches" << " of " << nentries << " entries each..." << endl;
    
    TString baseBranchName = "EventBranch" +  std::to_string(splitlevel) + "_";

    char etype[20];
    Event *event = new Event();

    for (int b = 0; b < nbranches; b++) {
        TString branchName = baseBranchName + std::to_string(b) + ".";
        auto branch = tree->Branch(branchName, "Event", &event, 32000, splitlevel);
        
        for (Int_t i = 0; i < nentries; i++)
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
                event->SetMeasure(m, Int_t(gRandom->Gaus(m, m + 1)));
            
            // fill the matrix
            for (UChar_t i0 = 0; i0 < 4; i0++)
                for (UChar_t i1 = 0; i1 < 4; i1++)
                    event->SetMatrix(i0, i1, gRandom->Gaus(i0 * i1, 1));
            
            // create and fill the Track objects
            for (Int_t t = 0; t < ntrack; t++)
                event->AddTrack(random);

            branch->Fill();
            event->Clear();
        }
    }
    delete event;
}

void writeTree(TTree *tree, int nevents, int nbranches)
{
    // Fill a TTree with different types of branches

    int types = 4;
    int nbranches_per_type = ceil(nbranches/(float)types);
    int nentries_per_type = ceil(nevents/(float)types);
    int nentries_per_branch = ceil(nentries_per_type / (float)nbranches_per_type);

    fillSimpleBranches(tree, nbranches_per_type, nentries_per_branch);
    fillArrayBranches(tree, nbranches_per_type, nentries_per_branch);
    fillComplexBranches(tree, nbranches_per_type, nentries_per_branch, 99);
    fillComplexBranches(tree, nbranches_per_type, nentries_per_branch, 1);

    tree->Write();
}

// Write a tree to a local file
void create_sample_file(std::string filename, int nevents, int nbranches)
{
    if (nevents == 0 || nbranches == 0)
    {
        std::cout << "Error: Number of events and branches cannot be 0" << std::endl;
        return;
    }

    TFile *file = new TFile(filename.c_str(), "RECREATE");
    if(file == 0){
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    TTree *tree = new TTree(TREE_KEY, "A tree with events for benchmarking");
    writeTree(tree, nevents, nbranches);
    file->Close();
    cout << "Written " << nevents << " events in " << file->GetBytesWritten() << " bytes" << endl;
    delete file;
}