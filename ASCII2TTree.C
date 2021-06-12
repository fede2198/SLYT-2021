#include <iostream>
#include <fstream>
#include <sstream>
#include <TFile.h>
#include <TTree.h>

using namespace std;

/*
 * macro di root per convertire i dati da formato ascii a formato root
 *
 * inputFileName:	nome del file di input
 * outputFileName:	nome del file di output
 */
void ASCII2TTree(const char* inputFileName, const char* outputFileName, int n_qdc, int n_tdc)
{
    cout<<"Converting "<<inputFileName<<" into "<<outputFileName<<endl;

    // apre il file di input con l'oggetto ifstream
    // che appartiene alle librerie standard C++
    ifstream inFile;
    inFile.open(inputFileName);

    // apre il file di output in formato root
    TFile* outFile = new TFile(outputFileName,"RECREATE");
    // crea un TTree che verra' poi scritto in outFile.
    // al tree viene assegnato un nome (datatree) e una descrizione.
    TTree* tree = new TTree("datatree","data from my experiment");

    // crea le variabili in cui mettere il contenuto del file di input
    // e le assegna al TTree di output
    int nevents;
    int channels[n_qdc+ n_tdc];
    tree->Branch("NEventi",&nevents,"NEventi/I");
    // i canali sono disposti nel file di input nell'ordine 0 16 1 17 ...etc
    for(int i=0;i< n_qdc;i+=2){
	    stringstream temp;
    	    temp << "CH" << i / 2;
	    tree->Branch(temp.str().c_str(), channels+i, (temp.str()+"/I").c_str());
	    temp.str("");
	    temp << "CH" << i / 2 + n_qdc / 2;
	    tree->Branch(temp.str().c_str(), channels+i+1, (temp.str()+"/I").c_str());
    }

    for(int i=0;i< n_tdc;i+=2){
	    stringstream temp;
    	    temp << "TCH" << i / 2;
	    tree->Branch(temp.str().c_str(), channels + n_qdc + i, (temp.str()+"/I").c_str());
	    temp.str("");
	    temp << "TCH" << i / 2 + n_tdc / 2;
	    tree->Branch(temp.str().c_str(), channels+ n_qdc + i + 1, (temp.str()+"/I").c_str());
    }


    // legge il file riga per riga e riempie il TTree.
    string line;
    while(getline(inFile, line)){
	    // prende in considerazione solo le righe che iniziano con un numero (ignora commenti)
	    if(!isdigit(line[0]))
		    continue;

	    stringstream str(line);
	    str >> nevents;
	    for (int i = 0; i < n_qdc + n_tdc; ++i)
	    {
		    str >> channels[i];
	    }
	    tree->Fill();
    }

    // scrive il numero di eventi (= numero righe del file) nel TTree.
    cout<<"Number of entries: "<<tree->GetEntries()<<endl;

    // scrive il tree nel file e chiude il file
    tree->Write();
    outFile->Close();
}
