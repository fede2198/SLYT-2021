//find_peaks_update
/*
find_peaks_update.C finds the greatest n peaks (n_p in input) on the given hist and returns their fit results

********************HOW TO USE**************************
Insert function find_peaks_update(<input>) into the function you want to use it in
  shell> root -l
	root[0] .L find_peaks_update.C
  root[1] .L fileyouuse.C
	root[2] fileyouuse(<input>)
*/

#include "TCanvas.h"
#include "TMath.h"
#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
 
// Comment out the line below, if you want "peaks' heights".
// Uncomment the line below, if you want "peaks' areas".

// #define __PEAKS_C_FIT_AREAS__ 1 /* fit peaks' areas 
 
Int_t npeaks = 3;

Double_t fpeaks(Double_t *x, Double_t *par) {
   Double_t result = par[0] + par[1] * x[0];
   for (Int_t p = 0;p < npeaks; p++) {
      Double_t norm  = par[3 * p + 2]; // "height" or "area"
      Double_t mean  = par[3 * p + 3];
      Double_t sigma = par[3 * p + 4];
#if defined(__PEAKS_C_FIT_AREAS__)
      norm /= sigma * (TMath::Sqrt(TMath::TwoPi())); // "area"
#endif // defined(__PEAKS_C_FIT_AREAS);
      result += norm * TMath::Gaus(x[0],mean,sigma);
   }
   return result;
}


void peaks(TH1* hist, Int_t np=3) {
   npeaks = TMath::Abs(np);
   Double_t par[3000];

   //output file for maxima mean, error and P value
   ofstream myoutput;
   myoutput.open("fit_res.dat");
   myoutput << "#mean\tmean_err" << endl;	

   TCanvas *c2 = new TCanvas("c2","c2",10,10,1000,900);
   c2->Divide(1,2);
   c2->cd(1);
   hist->Draw();

   TH1F *hist2 = (TH1F*)hist->Clone("hist2");
  
   // Use TSpectrum to find the peak candidates
   TSpectrum *s = new TSpectrum(2 * npeaks);
   Int_t nfound = s->Search(hist, 2, "", 0.10);
   printf("Found %d candidate peaks to fit\n",nfound);
   
   // Estimate background using TSpectrum::Background
   TH1 *histb = s->Background(hist, 20, "same");
   if (histb) c2->Update();
   if (np < 0) return;
 
   //estimate linear background using a fitting methistod
   c2->cd(2);
   TF1 *fline = new TF1("fline","pol1",0,1000);
   hist->Fit("fline","qn");

   // Loop on all found peaks. Eliminate peaks at the background level
   par[0] = fline->GetParameter(0);
   par[1] = fline->GetParameter(1);
   npeaks = 0;
   Double_t *xpeaks;
   xpeaks = s->GetPositionX();
   
   for (Int_t p=0;p<nfound; p++) {
      Double_t xp = xpeaks[p];
      Int_t bin = hist->GetXaxis()->FindBin(xp);
      Double_t yp = hist->GetBinContent(bin);
      
     // if (yp-TMath::Sqrt(yp) < fline->Eval(xp)) continue;
      par[3 * npeaks + 2] = yp; // "height"
      par[3 * npeaks + 3] = xp; // "mean"
      par[3 * npeaks + 4] = 3; // "sigma"
      npeaks++;
   }

   printf("Found %d useful peaks to fit\n", npeaks);
   printf("Now fitting: Be patient\n");
   TF1 *fit = new TF1("fit", fpeaks, 0, 1000,2 + 3 * npeaks);
   
   // We may have more than the default 25 parameters
   TVirtualFitter::Fitter(hist2, 10 + 3 * npeaks);
   fit->SetParameters(par);
   fit->SetNpx(1000);
   hist2->Fit("fit");
   
   //TPaveStats *ps = (TPaveStats*)c2->GetPrimitive("stats");
  //gStyle->SetStatY(0.9);
  //gStyle->SetStatX(0.9);
  //gStyle->SetStatW(0.4);
  //gStyle->SetStatH(0.5); 

   TF1 *my_fit_f = hist2->GetFunction("fpeaks");
   Double_t fit_par[npeaks * 3];
	
   //get fit parameters and cut the original hist
   int n_events, bin_min, bin_max;
   double mean, sigma, prob;
   double bin_width = hist->GetXaxis()->GetBinWidth(1);  
   vector<TH1*> hists, hist_clones;
   int i, k, n;
   TRandom3 rndgen;

   //prepare where to draw them
   TCanvas *c3 = new TCanvas("c3","c3",10,10,1000,900);
   c3->Divide(npeaks,2);
   
   for(i = 0; i < npeaks; i++){   
   	n_events = 0;
	mean =  fit->GetParameter(3 * i + 3);
        sigma = fit->GetParameter(3 * i + 4);
	myoutput << mean << "\t" << fit->GetParError(3 * i + 3) << "\t" << endl;
   	
	bin_min = (mean - 2 * sigma) / bin_width;
	bin_max = (mean + 2 * sigma) / bin_width;
	
	hist_clones.push_back(new TH1D(TString::Format("Copies of only max %d",i + 1), TString::Format("Copies of only max %d",i + 1), bin_max - bin_min, mean - 2 * sigma, mean + 2 * sigma));
	hists.push_back(new TH1D(TString::Format("hist max %d",i + 1), TString::Format("hist max %d",i + 1), bin_max - bin_min ,mean - 2 * sigma, mean + 2 * sigma));	
	
	for(k = bin_min; k < bin_max; k++){    
	       	       n = hist->GetBinContent(k);
	 	       n_events += n;	
       		       hist_clones[i]->SetBinContent(k - bin_min, (double) n);
	}

	for(k = 0; k < n_events; k++) hists[i]->Fill(rndgen.Gaus(mean, sigma));
	
	hists[i]->Rebin(4);
	hist_clones[i]->Rebin(4);
	prob = hist_clones[i]->KolmogorovTest(hists[i]);
  	cout << "prob = " << prob << endl;

	c3->cd(i+1);
	hists[i]->Draw();
	hists[i]->SetLineColor(3);
	hist_clones[i]->Draw("Same");
   }
}
