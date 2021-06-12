/* ---------------------------------------------------------------------------
 * ** main_daqpet_init.c
 * ** Programma di aquisitione per l'esperienza della pet del laboratorio di
 * fisica nucleare e subnucleare della laurea magistrale in fisica universita'
 * la sapienza di Roma.
 * Questo programma permette di comunicare con l'elettronica VME (V792, V1718)
 * della CAEN utilizzando le CaenVMELIB per inizializzare e acquisire i dati.
 *
 * Opzioni 
 * 
 * -o <filename> salva l'output in formato testuale nel file filename
 *  (obbligatorio)
 *
 * -n <nevents> imposta il numero di eventi da acquisire
 * 
 * -p <IPED> imposta la corrente di piedistallo
 *
 * -t <time> imposta il tempo di acquisizione
 *
 * -r genera automaticamente il file di root con nome <filename>.root
 *
 * -v aumenta la verbosità del programma fornendo informazioni utili per i debug
 *  ATTENZIONE l'aumento della verbosità diminuisce la rate massima
 *  d'acquisizione non utilizzare -v durante la presa dati ma solo per
 *  effettuare debug
 *
 * il programma dovra' ricevere almeno o il numero di eventi o il tempo di
 * acquisizione se vengono forniti tutti e due il programma termina quando una
 * delle due condizioni viene raggiunta ignorando l'altra. Per arrestare il
 * programma manualmente premere Ctrl+c per deinizializzare correttamente
 * l'elettronica e salvare i dati acquisiti sul disco.
 *
 * Il presente programma e' rilasciato "as is" gli autori non si assumono
 * nessuna responsabilita' per aventuali malfunzionamenti o bug
 * 
 *NOTA: clear_veto fa in modo che il segnale di veto in uscita corrisponda allo zero NIM logico;
 *      set_veto fa in modo che il segnale di veto in uscita corrisponda all'1 NIM logico.
 *      Le funzioni clear_veto e set_veto restituiscono un valore int pari ad 1 se le rispettive operazioni sono andate a buon fine,
 *      altro in caso contrario.
 *
 * **
 * ** Author: Daniele Racanelli, Flavio Pisani, Paolo Cretaro
 * **Coauthor: Gaia Franciosini, Luca Ceccarelli, Pippo Franco, Alessandro Santoni
 * -------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //classificazione dei caratteri 
#include <fcntl.h> //file control options
#include <unistd.h> //standard symbolic constants and types
#include <getopt.h> //parsing of the command-line arguments
#include <sys/ioctl.h> //I/O Control operations
#include <sys/time.h> //definition of time types
#include <time.h>
#include <signal.h> //signal handling
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

//I'd like to get rid of those!!!
#include "my_vmeio.h"
#include "my_vmeint.h"

//Bridge!
#include "vme_bridge.h"

#include "main_daqpet.h"

//Modules
#include "v1718_lib.h"
#include "tdc775_lib.h"
#include "adc792_lib.h"

// root
#include "root.h"

using namespace std;
// printa il funzionamento e i flag su terminale
void print_usage(FILE* stream, char* name, int exit_status){
	fprintf(stream, "Usage: %s -o filename  -n nevents -t time -p ped [-r/--root]\n\n"
			"\t-o FILENAME\tfile di output in cui vengono scritti gli eventi\n"
			"\t-n NEVENTS\timposta il numero di eventi da acquisire\n"
			"\t-t TIME\t\timposta il tempo di acquisizione\n"
			"\n\t\t\tdeve essere specificata almeno una di queste due opzioni,\n"
			"\t\t\tse vengono specificate entrambe il programma terminera' quando una delle due condizioni viene raggiunta per prima\n\n"
			"\t-r, --root\tgenera automaticamente il file di root chiamandolo FILENAME.root\n"
			"\t-v\t\taumenta la verbosita' (e lentezza) del programma\n"
			"\t-p\t\timposta la corrente di piedistallo\n",
			name);
	exit(exit_status);
}

void int_handler(int sig){
  //stderr è solo un modo alternativo per printare qualcosa
	fprintf(stderr, "\nCaught signal %d\n", sig);
	exit_signal = true;
}

int main(int argc, char** argv)
{
	/************************* PARSING DEI PARAMETRI **************************
	 * inutile commentare cio' che viene fatto di seguito
	 * e' piu' che intuitivo direi e in caso di dubbio vedere su google come funziona getopt */
	
  int opt;
	unsigned long EvMax = 0;
	unsigned long TMax = 0;
	unsigned long Pmax=IPED_VAL;
	unsigned long time_curr = 0;
	char* output_filename = NULL;

	verbose = 0;
	bool rootc = false;

	struct option long_options[] = {
		{ "help", 0, NULL, 'h'},
		{ "verbose", 0, NULL, 'v'},
		{ "output", 1, NULL, 'o'},
		{ "nevents", 1, NULL, 'n'},
		{ "root", 0, NULL, 'r'},
		{ "time", 1, NULL, 't'},
		{ "ped", 0, NULL, 'p'},
		{ NULL, 0, NULL, 0}
	};

	do{
		opt = getopt_long(argc, argv, "hvo:n:rt:p:", long_options, NULL);
    //argc significa argument counter, argv argument values
    //getopt prende in input i flag inseriti da terminale ("-h -v -o" eccetera)
    //getopt_long prende in input anche argomenti "long" che credo si riferisca alla possibilità di inserire più lettere in uno stesso flag (esempio: "tar -xvf")
		switch(opt){
			case 'h':
				print_usage(stdout, argv[0], 0);
				break;
			case 'v':
				// questo permette di fare piu' livelli di verbosita' a seconda di quante -v vengono passate
				++verbose;
				break;
			case 'o':
				output_filename = optarg;
				break;
			case 'n':
				EvMax = atoi(optarg);
				break;
			case 'r':
				rootc = true;
				break;
			case 't':
				TMax = atoi(optarg);
				break;
			case 'p':
				Pmax = atoi(optarg);
				break;
			case -1:
				break;
			default:
				print_usage(stderr, argv[0], EXIT_FAILURE);
		}
	} while(opt != -1);

	if(output_filename == NULL || (EvMax == 0 && TMax == 0)){
		print_usage(stderr, argv[0], EXIT_FAILURE);
	}


	/********************************* FINE PARSING PARAMETRI *********************************/
	
  
  printf("Ped=%lu \n", Pmax);	

	int status_init,status;
	int32_t BHandle(0);
	vector<unsigned long> DataAdc(V792N_CHANNEL+2);
	FILE* output_file;


	/* Bridge VME initialization */
	status_init = bridge_init(BHandle);  
	bridge_deinit(BHandle);
	status_init = bridge_init(BHandle);

	printf("\n\n VME initialization\n");
	if (status_init != 1){


		printf("VME Initialization error ... STOP!\n");
		return(EXIT_FAILURE);
	}

	/* Modules initialization */
	if(V1718)
	{
		printf("\n Bridge initialization and trigger vetoed\n");
		status_init *= init_1718(BHandle);
		status_init *= clear_veto_1718(BHandle);
		/*we put veto signal to 0 becouse we want a 0 from the output of the Logic Unit: 
		  this operation stops the generation of the gate during the initialization.*/
		status_init *= init_scaler_1718(BHandle) ;
		status_init *= init_pulser_1718(BHandle) ;
		status_init *= clearbusy_1718(BHandle);
		if (status_init != 1) { return(EXIT_FAILURE); }

	}
	else {
		printf("\n No TRIGGER module is present:: EXIT!\n");
		return(EXIT_FAILURE);
	}

	/* adc 792 initialization */
	if(ADC792)
	{
		printf("\n Initialization of ADC792 \n");
		status_init *= init_adc792(BHandle,Pmax);
		if(status_init!=1)
		{
			printf("Error in adc792 initialization.... STOP!\n");
			return(EXIT_FAILURE);
		}

	}

	/* tdc 775 initialization */
	if(TDC775)
	{
		printf("\n Initialization of TDC775 \n");
		status_init *= init_tdc775(BHandle);
		if(status_init!=1)
		{
			printf("Error in tdc775 initialization.... STOP!\n");
			return(EXIT_FAILURE);
		}

	}
	printf("\nVME and modules initialization completed \n\n");

	/*-------------- HANDLING INTERRUPT SIGNAL ------------------*/
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = int_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	/*------------- ACQUISITION------------------*/

	if(access(output_filename, F_OK) == 0){
		fprintf(stderr, "warning %s already exists overwite <Y/n>:", output_filename);
		if(getchar() == 'n'){
			return EXIT_SUCCESS;
		}
	}
	if((output_file=fopen(output_filename,"w")) == NULL){
		perror(output_filename);
	}
	/*---------------Setting the output file----------------*/
	fprintf(output_file,"#Ev.\t");
	for(int i=0;i<V792N_CHANNEL/2;i++){
		fprintf(output_file,"CH %d\t", i);
		fprintf(output_file,"CH %d\t", i + V792N_CHANNEL/2);

	}

	/*------------------------------------------------------*/

	/* resetting adc buffer and releasing veto ACQUIRING EVENTS BEYOND THIS POINT*/

	data_reset_adc792(BHandle);

	/*We put the veto signal at 1 in order to have coincidence in the Logic Unit. This operation permits the gate generation. */

	if(set_veto_1718(BHandle) != 1){
		fprintf(stderr, "ERROR veto not released aborting\n");
		return EXIT_FAILURE;
	}
	fprintf(stderr, "Veto released starting acquisition\n");
	
  /* acquisition starts */

	exit_signal = false;
	bool trigger;
	struct timespec start, end;

	/*---------STARTING ACQUISITION TIME--------------*/
	clock_gettime(CLOCK_MONOTONIC, &start);
	unsigned long EvNum = 0;

	/*---------MAIN ACQUISITION CYCLE--------------*/
	while((((EvMax != 0) && (EvNum < EvMax)) || ((TMax != 0) && (time_curr < TMax)))
			&& (exit_signal==false)){
		status=1;
		trigger = false;

		/*---------WAITING FOR TRIGGER--------------*/
		while (trigger==false && status==1 && exit_signal == false){
			status = trigger_interrupt(BHandle, &trigger);
		}
		if(verbose>0){
			printf("TRIGGER GONE\n");
		}  

		/*We put the veto to 0 to start the reading-writing operations*/
		if(clear_veto_1718(BHandle) != 1){      
			fprintf(stderr, "ERROR VETO NOT RELEASED\n");
			return EXIT_FAILURE;
		}

		/*---------READING THE WHOLE BUFFER--------------*/
		DataAdc = read_block_adc792(BHandle, 0, status);

		if(status!=1){
			fprintf(stderr, "ERRORE!!!! %d\n", status);
		}

		// vedere pagina 45 del qdc per la conformazione del buffer
		int valid;
		valid = (DataAdc[0] >> 24) & 0x7;
		int k=0;
		
		/*-------UNPACKING DATA FROM BUFFER AND DISCARTING NON VALID EVENTS--------*/
		while( valid == 2 ){
			EvNum =DataAdc[k+V792N_CHANNEL+1] & 0xffffff;
			fprintf(output_file, "\n%lu\t", EvNum);
			for(int i=1;i<V792N_CHANNEL+1;i++){
				fprintf(output_file,"%lu\t",DataAdc[k+i] & 0xfff);
			}
			k += V792N_CHANNEL+2;
			valid = (DataAdc[k] >> 24) & 0x7;
		}

		/*-----WRITING DATA TO DISK------------*/
		fflush(output_file);
		if(verbose>0){
			printf("ev num %lu\n", EvNum);
		}
		/*After the reading-writing operations, we are ready for another event, so we put veto signal to 1 */
		if(set_veto_1718(BHandle) != 1){
			fprintf(stderr, "ERROR VETO NOT RELEASED.\n");
			return EXIT_FAILURE;
		}

		/*----UPDATING ACQUISITION TIME--------------*/
		clock_gettime(CLOCK_MONOTONIC, &end);
		time_curr = end.tv_sec - start.tv_sec;

	}

	/*------------CALCULATING TOTAL ACQUISITION TIME------------------*/
	clock_gettime(CLOCK_MONOTONIC, &end);

	/*------------CLEAR VETO SIGNAL--------------------*/
	/*After the acquisition cycle we put veto signal to 0 */
	status_init *= clear_veto_1718(BHandle);
	printf("Done\n");

	/*------------CALCULATING NUMBER OF EVENTS AND ACQUISITION RATE--------------*/
	unsigned adc_scaler;
	/*
	 * FIXME: senza questo sleep la read_scaler_adc792 ritorna un numero a caso
	 * forse si tratta di un problema di timing (il qdc non e' pronto a rispondere)?
	 */
	sleep(1);
	adc_scaler = read_scaler_adc792(BHandle);
	double time_elapsed = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec)*1e-9;
	printf("Time: %lf s\tRate: %lf Hz\n", time_elapsed, (adc_scaler+1)/time_elapsed);

	fprintf(output_file, "\n# Time: %lf s\tRate: %lf Hz", time_elapsed, (adc_scaler+1)/time_elapsed);
	fclose(output_file);


	/* VME deinitialization */
	if(bridge_deinit(BHandle)==1){
		printf("\n VME and modules deinitialization completed \n\n");
	}

	/*-------------GENERATING ROOT FILE-----------------*/
	if(rootc){
		printf("Generating ROOT file\n");
		string root_filename(output_filename);
		root_filename += ".root";
		ASCII2TTree(output_filename,root_filename.c_str());
	}

	return(EXIT_SUCCESS);
}

