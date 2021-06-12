/*root macro to analyze data without pedestal
 
*******************************DESCRIPTION*************************************

From source file, it 
1)Subtracts average pedestal value (gaussian fit) on the x axis;
2)Makes the gaussian fit with root;
3)Prints source hist, pedestal hist, pedestal-background-source hist and its gaussian fit on the highest peak;
4)Prints fit parameters.
5)Computes and prints mean value, error and chi square
6)Creates png image of hists if "print" input variable is = 1 

******************************* HOW TO USE IT *************************************

1)Launch it as:
	shell> root -l
	root[0] .L analyze.C
	root[1] analyze(<input>)
2)Make sure you insert the right path to the input file or put this file in its folder

**********************************CODE*************************************
*/



 void analyze(const char* fileName,const char* filePedestal,int n,int bins, int n_peak,int left,int right,int leftfitped, int rightfitped, bool print) {

 // output print
    ofstream myoutput_main;
    myoutput_main.open("fit_ped.dat");
    myoutput_main << "#mean	  mean_err   p_value " << endl;

  // initial variable declaration
    int counts[32];
    int counts2[32];
    stringstream tmp_str;
  
  // opens files and takes "datatree" TTree
    TFile* file = new TFile(fileName);
    TTree* tree = (TTree*)file->Get("datatree");
    TFile* Pedestal = new TFile(filePedestal);
    TTree* treePedestal = (TTree*)Pedestal->Get("datatree");
 
  // prints general info
    Long64_t nentries = tree->GetEntries(); 
    cout<<"Histogram with "<<nentries<<" events and "<<bins<<" bins, "<<" using channel # "<<n<<". Background not included!"<<endl;
  
  // sets variables addresses
    for(int k=0 ; k<32 ; k++){
      tmp_str.str("");
      tmp_str << "CH" << k;
      tree->SetBranchAddress(tmp_str.str().c_str(), (counts+k));
      treePedestal->SetBranchAddress(tmp_str.str().c_str(), (counts2+k));
     }
 
  // creates empty hists  
    TH1D* source = new TH1D("Source without pedestal", TString::Format("QDC histogram of channel %i",n), bins, left, right); 
    TH1D* pedestal = new TH1D("Pedestal",TString::Format("Pedestal histogram of channel %i",n), 50, 200, 250);
    TH1D* source_ped = new TH1D("Source with pedestal", TString::Format("QDC histogram of channel %i with pedestal",n), bins, left, right);
 
  // fills pedestal hist
    nentries = treePedestal->GetEntries();
    for(Long64_t entry = 0; entry < nentries; entry++) {
      treePedestal->GetEntry(entry);
      pedestal->Fill(counts2[n]);
    }
    
  // creates and splits in four parts the canvas
    TCanvas* c1 = new TCanvas();
    c1->Divide(2,2);
    gStyle->SetOptStat("ne");

  // draws pedestal hists
    c1->cd(2);
    pedestal->SetLineColor(1);
    pedestal->Draw();

  // pedestal gaussian fit
    TF1* fitPed = new TF1("fitPed","gaus",leftfitped,rightfitped); 
    TFitResultPtr s = pedestal->Fit(fitPed,"SQ", "R");  
    gStyle->SetOptFit(101);
  
  // fit results
    cout << "\nPEDESTAL FIT RESULTS:" << endl;
    s->Print();
    cout << "\n";
    myoutput_main << s->Parameter(1) << "\t" << s->ParError(1) << "\t" << s->Prob() << endl;


  // fills source and source_ped hist, last one minus pedestal average
    nentries = tree->GetEntries();
    for(Long64_t entry = 0; entry < nentries; entry++) {
      tree->GetEntry(entry);
      source->Fill(counts[n]);
      source_ped->Fill(counts[n]- (s->Parameter(1)) );
     }

      //draws source & source_ped hists
    c1->cd(1);
    source->SetLineColor(3);
    source->Draw();
    c1->cd(3);
    source_ped->SetLineColor(4);
    source_ped->Draw();

    // renames axes differently
    pedestal->GetXaxis()->SetTitle("QDC Counts");
    pedestal->GetYaxis()->SetTitle("Counts");
    pedestal->GetYaxis()->SetTitleOffset(1.5);
    source->GetXaxis()->SetTitle("QDC Counts");
    source->GetYaxis()->SetTitle("Counts");
    source->GetYaxis()->SetTitleOffset(1.5);
    source_ped->GetXaxis()->SetTitle("QDC Counts");
    source_ped->GetYaxis()->SetTitle("Counts");
    source_ped->GetYaxis()->SetTitleOffset(1.5);

    //saves hists
    if (print == 1) c1->SaveAs("ris_analyze.png");
    
    // looks for n_peak peaks and fits them, creating an output file with all data    
    myoutput_main.close();

    find_peaks1(source_ped, n_peak);
   }
