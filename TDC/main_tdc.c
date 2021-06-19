//file nuovo

/* ---------------------------------------------------------------------------
     * ** main_tdc.c

     Data acquisition program for Physics Laboratory II exam, MSc Physics, Sapienza Università di Roma
 * 
 * Options: 
 * 
 * -o <filename> saves the output in a file called filename
 *  (obbligatorio)
 *
 * -n <nevents> sets the number of events to acquire
 * 
 * -p <IPED> sets the pedestal current
 *
 * -t <time> sets acquisition time
 *
 * -r generates automatically a root file called <filename>.root
 *
 * -v increases the verbosity of the program, giving more information useful for debug
 *  ATTENTION verbosity decreases max acquisition rate, use it just for debug
 *  
 * The program needs or the number of events or the acquisition time. If both are set, 
 * than the program stops whne one of the conditions is satisfied.To stop it correctly, please
 * press Ctr+c.
 *
 * NOTE: clear_veto assures the veto signal corresponds to 0 (logic NIM);
 *      set_veto assures the veto signal corresponds to 1 (logic NIM);
 *      Both return 1 if they are working correctly.
 *
 * ** Authors: Nicola Alborè, Maria Adriana Sabia, Federica Troni and Alberto Tubito


     * -------------------------------------------------------------------------*/
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <getopt.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>
    #include <time.h>
    #include <signal.h>
    #include "my_vmeio.h"
    #include "my_vmeint.h"

    //Bridge!
    #include "vme_bridge.h"

    #include "main_daqpet.h"
    #include "v1718_lib.h"
    #include <fstream>
    #include <iostream>
    #include <vector>
    #include <string>
    #include "adc792_lib.h"
    #include "tdcdd00_lib.h"


    // root
    #include "root.h"

    using namespace std;

    void print_usage(FILE* stream, char* name, int exit_status){
	    fprintf(stream, "Usage: %s -o filename  -n nevents -t time -p ped [-r/--root]\n\n"
			    "\t-o FILENAME\tfile di output in cui vengono scritti gli eventi del qdc e del tdc." 
			    "In particolare vengono creati due file rinominati FILENAME_qdc e FILENAME_tdc.\n"
			    "\t-n NEVENTS\timposta il numero di eventi da acquisire\n"
			    "\t-t TIME\t\timposta il tempo di acquisizione\n"
			    "\n\t\t\tdeve essere specificata almeno una di queste due opzioni, se si sceglie solo -n l'acquisizione viene fermata"
			    "con CTRL + C (miraccomando a non chiudere il terminale...polletti degli anni successivi)\n"
			    "\t\t\tse vengono specificate sia -t che -n il programma terminera' quando una delle due condizioni viene raggiunta per prima"
			    "\n\n"
			    "\t-r, --root\tgenera automaticamente i due file root chiamandoli FILENAME_qdc.root e FILENAME_tdc.root\n"
			    "\t-v\t\taumenta la verbosita' (e lentezza) del programma (da usare in caso di debug)\n"
			    "\t-p\t\timposta la corrente di piedistallo (default 95)\n",
			    name);
	    exit(exit_status);
    }

    void int_handler(int sig){
	    fprintf(stderr, "\nCaught signal %d\n", sig);
	    exit_signal = true;
    }

   int main(int argc, char** argv)
    {
	    /************************* PARAMETERS PARSING **************************
	     */
	    int opt;
	    unsigned long EvMax = 0;
	    unsigned long TMax = 0;
	    unsigned long Pmax=IPED_VAL;
	    unsigned long time_curr = 0;
	    char* output_filename_basic = NULL;
	    char* output_filename = (char *)malloc(25); 
	    
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
		    opt = getopt_long(argc, argv, "hvo:n:r:t:p:", long_options, NULL);
		    switch(opt){
			    case 'h':
				    print_usage(stdout, argv[0], 0);
				    break;
			    case 'v':
				    // questo permette di fare piu' livelli di verbosita' a seconda di quante -v vengono passate
				    ++verbose;
				    break;
			    case 'o':
				    output_filename_basic = optarg;
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

	    if(output_filename_basic == NULL || (EvMax == 0 && TMax == 0)){
		    print_usage(stderr, argv[0], EXIT_FAILURE);
	    }
	    /********************************* PARMETERS FINE PARSING *********************************/
	    printf("Selected ped=%lu \n", Pmax);	

	    int status_init,status;
	    int32_t BHandle(0);
	    vector<unsigned long> DataAdc(V792N_CHANNEL+2); //34
	    vector<unsigned long> DataTdc(Vdd00N_CHANNEL * 2 + 2);//18 x2

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
		    /*we put veto signal to 0 because we want a 0 from the output of the Logic Unit: 
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

	    /*ADC792 initialization*/

	    if(ADC792)
	      {
		printf("Initialization of ADC792\n");
		status_init *= init_adc792(BHandle,Pmax);
		if (status_init!=1)
		  {
		    printf("Error in adc792 initialization....STOP! \n");
		    return(EXIT_FAILURE);
		  }
	      }

	    /*tdc DD00 initialization*/

	    if(TDCdd00)
	      {
		printf("Initialization of TDCDD00\n");
		status_init *= init_tdcdd00(BHandle);
		if (status_init!=1)
		  {
		    printf("Error in TDCDD00 initialization....STOP! \n");
		    return(EXIT_FAILURE);
		  }
	      }

	    //	 status_read *= read_tdcdd00_simple(BHandle, pDataTdc);


	/*-------------- HANDLING INTERRUPT SIGNAL ------------------*/

	    struct sigaction sigIntHandler;
	    sigIntHandler.sa_handler = int_handler;
	    sigemptyset(&sigIntHandler.sa_mask);
	    sigIntHandler.sa_flags = 0;
	    sigaction(SIGINT, &sigIntHandler, NULL);



	/*------------- ACQUISITION------------------*/

	    if(access(output_filename_basic, F_OK) == 0){
		    fprintf(stderr, "Warning %s already exists overwite <Y/n>:", output_filename_basic);
		    if(getchar() == 'n'){
			    return EXIT_SUCCESS;
		    }
	    }
	    strcpy(output_filename,output_filename_basic);
	    strcat(output_filename,"_qdc");
	    if((output_file = fopen(output_filename,"w")) == NULL){
		    perror(output_filename);
	    }

      /*---------------Setting the output file----------------*/
      /*QDC output file*/

	    fprintf(output_file,"#Ev.\t");
	    for(int i=0;i<V792N_CHANNEL/2;i++){
		    fprintf(output_file,"CH %d\t", i);
		    fprintf(output_file,"CH %d\t", i + V792N_CHANNEL/2);
	    };

      /*TDC output file*/
	    for(int i=0;i<Vdd00N_CHANNEL/2;i++){
		    fprintf(output_file,"TCH %d\t", i);
		    fprintf(output_file,"TCH %d\t", i + Vdd00N_CHANNEL/2);
	    }	

	    /*------------------------------------------------------*/

	    /* resetting adc buffer and releasing veto ACQUIRING EVENTS BEYOND THIS POINT*/
	    data_reset_adc792(BHandle);
	    data_reset_tdcdd00(BHandle);
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
	    unsigned long EvNum = 0,EvNumTdc =0;

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
		    DataTdc = read_block_tdcdd00(BHandle, status);

		    if(status!=1){
			    fprintf(stderr, "ERRORE!!!! %d\n", status);
		    }

		    // vedere pagina 45 del qdc per la conformazione del buffer
		    int valid, validtdc;
		    valid = (DataAdc[0] >> 24) & 0x7; //Bit di controllo 26-24 
		    validtdc = (DataTdc[0] >> 24) & 0x7; //Bit di controllo 26-24 
		    
        int k=0, j=0;

		    /*-------UNPACKING DATA FROM BUFFER AND DISCARTING NON VALID EVENTS--------*/
		    while( valid == 2 ){ // events with control bit 010
	//start from  33
			    EvNum =DataAdc[k+V792N_CHANNEL+1] & 0xffffff; //Bit 0-24
			    fprintf(output_file, "\n%lu\t", EvNum);

			    for(int i=1;i<V792N_CHANNEL+1;i++){ //from1 to 32 channels
				    fprintf(output_file,"%lu\t",DataAdc[k+i] & 0xfff);//Bit 0-11 (dato)
			    }
	  //if k = 0 , 0+33+2 = 35 
			    //printf("\n k = %d\n",k);
			    k += V792N_CHANNEL+2;
	 
			    valid = (DataAdc[k] >> 24) & 0x7; //Bit di controllo 26-24
		    }

		    while( validtdc == 2 ){ // events with control bit 010
			    //start from  17
		      EvNumTdc =DataTdc[j+Vdd00N_CHANNEL+1] & 0xffffff; //x2
			    
			    for(int i=1;i<Vdd00N_CHANNEL+1;i++){
			        fprintf(output_file,"%lu\t",DataTdc[j+i] & 0xfff);
			    }
			    //if k = 0 , 0+16+2 = 18
			    //printf("\n j = %d\n",j);
			    // j += Vdd00N_CHANNEL+2;
			    j += 18;
			    validtdc = (DataTdc[j] >> 24) & 0x7;
			    //printf("valid dopo run è %d\n",validtdc);
			    //printf("k = %d j = %d \n",k,j);

			    //for(int y = 0; y < 18; y++){
			    //  printf("DataTdc[%d] = %lu \n",y, DataTdc[y]);
			    //    }
		              }
		    
	/*-----WRITING DATA TO DISK------------*/
		    fflush(output_file);
		    if(verbose>0){
			    printf("ev num %lu\n", EvNum);
		    }
	
		    //if(verbose>0){
			  //  printf("ev num %lu\n", EvNumTdc);
		    //}
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
	    printf("\nTime: %lf s\tRate: %lf Hz\n", time_elapsed, (adc_scaler+1)/time_elapsed);

	    fprintf(output_file, "\n# Time: %lf s\tRate: %lf Hz", time_elapsed, (adc_scaler+1)/time_elapsed);
      fclose(output_file);

	    /* VME deinitialization */
	    if(bridge_deinit(BHandle)==1){
		    printf("\n VME and modules deinitialization completed \n\n");
	    }

	    /*-------------GENERATING ROOT FILE-----------------*/
	    if(rootc){
		    printf("Generating ROOT file\n");
		    string root_filename = output_filename;
		    root_filename += ".root";
		    ASCII2TTree(output_filename,root_filename.c_str(), V792N_CHANNEL, Vdd00N_CHANNEL);
	    }
	free(output_filename);
	    return(EXIT_SUCCESS);
    }
