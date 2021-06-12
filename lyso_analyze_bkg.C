//YES BACKGROUND; YES FILTER
//lyso_analyze.c performs the sum of the channels that are nearest neighbours of the given trigger (after subtracting the pedestal fit and dealing with the background) and plots it.
//TO USE CENTRAL CHANNEL: last entry with number of central channel (original LYSO)
//TO USE TDC FILTER WITH N CHANNELS: 4th entry != 0, last entry 0

//*************************** HOW TO USE IT **********************************

//root[0] .L lyso_analyze_bkg.C+ 
//root[1] lyso_analyze_bkg(filename, filepedestal, filebackground, tdc channel numbers to consider, central channel)

#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <iostream>
#include <TTree.h>
#include <TFitResult.h>
#include <TF1.h>
#include <TStyle.h>
#include <sstream>
#include "stdio.h"

#define TDC_SIZE 16
#define QDC_SIZE 32
#define TOT_SIZE 32 + 16 

using namespace std;

void lyso_analyze_bkg(const char* fileName, const char* filePedestal, const char* fileBackground, int nbins, int tdcCh2check, int centralCh){

	// initial variable declaration
        int counts[QDC_SIZE], counts_tdc[TDC_SIZE], counts2[QDC_SIZE], counts3[QDC_SIZE];
	stringstream tmp_str;

	const int SIZE = 5;
	vector<int> ch, ch_orig, tch;
	
	//channels selected
	int n_orig[SIZE] = {36,28,37,44,35};
	int n[SIZE] = {14,5,9,2,1};
	
	ch = vector<int> (n,n + SIZE);
	ch_orig = vector<int> (n_orig,n_orig + SIZE);

  	int index;
	if(centralCh != 0){
		cout << "Working with TDC filter set on central channel " << centralCh << endl;
	        tdcCh2check = 1;
		auto it = find(ch_orig.begin(), ch_orig.end(), centralCh);	 
		
		// If element was found, assigns it to index. Otherwise, it prints an error
	       	if (it != ch_orig.end()) index = it - ch_orig.begin();
		else fprintf(stderr,"Please insert a correct central channel!\n");
	
		tch.push_back(index);	
		
	}else{
		cout << "Working with TDC filter set on selecting at least " << tdcCh2check << " active channels per event" << endl; 
		int n_tdc[SIZE] = {0,1,2,3,4};
		tch = vector<int> (n_tdc, n_tdc + SIZE);
	}

	// opens files and takes "datatree" TTree
	TFile* file_lyso = new TFile(fileName);
	TTree* tree = (TTree*)file_lyso->Get("datatree");
	TFile* Pedestal_lyso = new TFile(filePedestal);
	TTree* treePedestal = (TTree*)Pedestal_lyso->Get("datatree");
	TFile* Background_lyso = new TFile(fileBackground);
	TTree* treeBackground = (TTree*)Background_lyso->Get("datatree"); 
	
	// sets variables addresses
	for(int k = 0 ; k < 32 ; k++){
		tmp_str.str("");
		tmp_str << "CH" << k;
		tree->SetBranchAddress(tmp_str.str().c_str(), (counts+k));
		treePedestal->SetBranchAddress(tmp_str.str().c_str(), (counts2+k));
		treeBackground->SetBranchAddress(tmp_str.str().c_str(), (counts3+k));
     	}

	// sets variables addresses for tdc
	for(int k = 0; k < TDC_SIZE; k++){
		tmp_str.str("");
		tmp_str << "TCH" << k;
		tree->SetBranchAddress(tmp_str.str().c_str(), (counts_tdc + k));
	}
	
	//creates canvases
	TCanvas* c_ped = new TCanvas("c_ped", "c_ped", 10, 10, 1000, 900);
	TCanvas* c_source_ped = new TCanvas("c_source", "c_source", 10, 10, 1000, 900);

	c_ped->DivideSquare(ch.size());
	c_source_ped->DivideSquare(ch.size());
	
	//creates empy hists vectors
	vector<TH1D*> pedestal_lyso;
	vector<TH1D*> background_lyso;
	vector<TH1D*> source_ped_lyso;
	vector<TFitResultPtr> s;
	TF1* fitPed = new TF1("fitPed","gaus",0,600); 
	TH1D* tot = new TH1D("Parameters", "Sum of the selected channels", nbins, 0, 4096);
	TH1D* tot_back = new TH1D("Parameters with background", "Sum of the selected channels with background", nbins, 0, 4096);

	//get entries
	Long64_t nentries_ped = treePedestal->GetEntries();
	Long64_t nentries_sig = tree->GetEntries();
   	Long64_t nentries_bkg = treeBackground->GetEntries();
  

	for(int k = 0; k < (int) ch.size(); k++){
		// creates empty hists   
		pedestal_lyso.push_back(new TH1D(TString::Format("Pedestal LYSO %d",ch_orig[k]),TString::Format("Pedestal histogram of channel %i; QDC Counts; Counts;",ch_orig[k]), 100, 0, 1000));
		source_ped_lyso.push_back(new TH1D(TString::Format("Source with pedestal LYSO %d", ch_orig[k]), TString::Format("QDC histogram of channel %i with pedestal; QDC Counts; Counts;",ch_orig[k]), nbins, 0, 4096));
		background_lyso.push_back(new TH1D(TString::Format("Background LYSO %d",ch_orig[k]), TString::Format("Background of channel %i; QDC Counts; Counts;",ch_orig[k]), nbins, 0, 4096)); 
	}
	
	//get pedestal entries
	for(Long64_t entry = 0; entry < nentries_ped; entry++) {
		treePedestal->GetEntry(entry);
		for(unsigned int k = 0; k < ch.size(); k++){
			pedestal_lyso[k]->Fill(counts2[ch[k]]); 		
		}
	}

	//fit & pedestal drawings
	for(unsigned int k = 0; k < ch.size(); k++){
		c_ped->cd(k + 1);
		pedestal_lyso[k]->Draw();
			
		// pedestal gaussian fit
		s.push_back(pedestal_lyso[k]->Fit(fitPed,"SQ", "R"));  	
	}

	//get qdc & tdc entries
	int counter = 0, thr = 100, n_counts;

  	for(Long64_t entry = 0; entry < nentries_sig; entry++) {
		counter = 0;
		tree->GetEntry(entry);

		//use tdc as a filter: must be below 4090, due to C discharge
		/*for(unsigned int j = 0; j < tch.size(); j++){
			n_counts = counts_tdc[j];
			if(n_counts> thr && n_counts < 4090) counter++;
		}*/
		if(counts_tdc[0] > 100 && counts_tdc[2] > 100) counter = 2;

		//signal is accepted only if counter> tdcCh2check
		if(counter >=  tdcCh2check){
			for(unsigned int k = 0; k < ch.size(); k++){
				source_ped_lyso[k]->Fill(counts[ch[k]]- (s[k]->Parameter(1)));		
			}
		}
	}

 	        
	for(Long64_t entry = 0; entry < nentries_bkg; entry++) {
		treeBackground->GetEntry(entry);
		for(unsigned int k = 0; k < ch.size(); k++){
			background_lyso[k]->Fill(counts3[ch[k]] - (s[k]->Parameter(1)));
		}
	}	
	
	for(unsigned int k = 0; k < ch.size(); k++){
		c_source_ped->cd(k+1);
		source_ped_lyso[k]->Draw();
	
		tot->Add(source_ped_lyso[k]);
		tot_back->Add(source_ped_lyso[k]);
		tot->Add(background_lyso[k], -1);
	}
	
	gStyle->SetOptFit(101);
	
	TCanvas* res = new TCanvas("res", "res", 10, 10, 500, 450); 	
	res->Divide(2,1);
	
	res->cd(1);
	tot->Draw();
	res->cd(2);
	tot_back->Draw();	

	res->SaveAs("plot_tot_bkg.pdf");
	c_source_ped->SaveAs("plot_source_ped_bkg.pdf");
	c_ped->SaveAs("plot_ped_bkg.pdf");
}













