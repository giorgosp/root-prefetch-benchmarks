R__LOAD_LIBRARY(libEvent.so)

#include <iostream>
#include "Event.h"

#define TREE_KEY "T"


void fillArrayBranches(TTree *tree, int nbranches, int nentries){
    cout << "Writing array branches with " << nbranches << " branches" << " of " << nentries << " entries each..." << endl;

    int array_len = 100;
    // Float_t  f[nbranches][array_len];
    Float_t **f = new Float_t *[nbranches];
    for (int i = 0; i < nbranches; ++i)
        f[i] = new Float_t[array_len];

    // keep all needed branches in memory
    std::string base_branch_name = "ArrayBranch";
    TBranch *branches[nbranches];
    for (Long64_t b = 0; b < nbranches; b++)
    {
        std::string branch_name = base_branch_name + std::to_string(b) + ".";
        branches[b] = tree->Branch(branch_name.c_str(), &(f[b][0]), "f[100]/F");
    }

    for (int i = 0; i < nentries; i++)
    {
        for (int b = 0; b < nbranches; b++)
        {
            for (int j = 0; j < array_len / 2; j++)
                f[b][j] = gRandom->Rndm();

            for (int j = array_len / 2; j < array_len; j++)
                f[b][j] = 0;
        }
        tree->Fill();
    }

    for (int i = 0; i < nbranches; ++i)
        delete[] f[i];
    delete[] f;
}

void fillComplexBranches(TTree *tree, int nbranches, int nentries, int splitlevel){
    // Create  branches whose leaves hold Event objects
    cout << "Writing complex branches with " << nbranches << " branches" << " of " << nentries << " entries each and split level " << splitlevel << " ..." << endl;
    
    TString baseBranchName = "EventBranch" +  std::to_string(splitlevel) + "_";

    char etype[10];
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

            tree->Fill();
            event->Clear();
        }
    }
    delete event;
}

void writeTree(TTree *tree, int nentries, int nbranches)
{
    // Fill a TTree with different types of branches

    tree->SetAutoFlush(100);

    int nbranches_per_type = nbranches / 2;
    fillArrayBranches(tree, nbranches_per_type, nentries);
    fillComplexBranches(tree, nbranches_per_type, nentries, 1);

    // int types = 3;
    // int nbranches_per_type = ceil(nbranches/(float)types);
    // int nentries_per_type = ceil(nevents/(float)types);
    // int nentries_per_branch = ceil(nentries_per_type / (float)nbranches_per_type);


    // fillSimpleBranches(tree, nbranches_per_type, nentries_per_branch);
    // fillArrayBranches(tree, nbranches_per_type, nentries_per_branch);

    // fillComplexBranches(tree, nbranches_per_type, nevents, 99);
    // fillComplexBranches(tree, nbranches_per_type, nentries_per_branch, 1);

    tree->Write();
}

// Write a tree to a local file
void create_sample_file(std::string filename, int nentries, int nbranches)
{
    if (nentries == 0 || nbranches == 0)
    {
        std::cout << "Error: Number of events and branches cannot be 0" << std::endl;
        return;
    }

    TFile *file = new TFile(filename.c_str(), "RECREATE");
    if(file == 0){
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    file->SetCompressionLevel(3);

    TTree *tree = new TTree(TREE_KEY, "A tree with events for benchmarking");
    writeTree(tree, nentries, nbranches);
    file->Close();
    cout << "Written " << nentries << " events in " << file->GetBytesWritten() << " bytes" << endl;
    delete file;
}