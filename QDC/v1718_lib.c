#include "CAENVMElib.h"
#include "CAENVMEtypes.h" 
#include "CAENVMEoslib.h"

#include "v1718_lib.h"
#include <iostream>

using namespace std;

/*----------------------------------------------------------------------*/
int init_1718(int32_t BHandle) {

  int status,caenst;
  unsigned Mask=0, DataShort=0;

  /*----------   setting the IO reg section of the V1718  -----------*/
  
  
  status = 1;
  DataShort = 0x200; /* timeout=50us - FIFO mode */
  caenst = CAENVME_WriteRegister(BHandle,cvVMEControlReg,DataShort);
  status *= (1-caenst); 
  caenst = CAENVME_ReadRegister(BHandle,cvVMEControlReg,&DataShort);
  status *= (1-caenst); 
  printf("Coltrol Reg config = %x \n",DataShort);

  /*  setting the output lines */

  caenst = CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,
				 cvActiveHigh,cvManualSW);
  status *= (1-caenst); 
  caenst = CAENVME_SetOutputConf(BHandle,cvOutput1,cvDirect,
				 cvActiveHigh,cvMiscSignals);
  status *= (1-caenst); 
  
  
  caenst *= CAENVME_SetOutputConf(BHandle,cvOutput2,cvDirect,
				  cvActiveHigh,cvMiscSignals);
  status *= (1-caenst); 

  caenst *= CAENVME_SetOutputConf(BHandle,cvOutput3,cvDirect,
				  cvActiveHigh,cvMiscSignals);
  status *= (1-caenst); 

  caenst *= CAENVME_SetOutputConf(BHandle,cvOutput4,cvDirect,
				  cvActiveHigh,cvMiscSignals);

  
  status *= (1-caenst); 
  /* setting which output line must be pulsed  */
  

  Mask = cvOut0Bit + cvOut1Bit + cvOut2Bit + cvOut3Bit + cvOut4Bit;
  //Mask = cvOut0Bit + cvOut1Bit;
  caenst = CAENVME_SetOutputRegister(BHandle,Mask); 
  status *= (1-caenst); 

  /*  setting the input lines */
  caenst = CAENVME_SetInputConf(BHandle,cvInput0,cvDirect,cvActiveHigh);
  status *= (1-caenst); 
  caenst = CAENVME_SetInputConf(BHandle,cvInput1,cvDirect,cvActiveHigh);
  status *= (1-caenst);

  return status;

}
  /*---------------   end setting IO reg section of V1718 ----------------*/

int init_scaler_1718(int32_t BHandle) {
  unsigned DataShort=0; 
  int status, caenst;
  unsigned short limit=1023, AutoReset=1;
  printf("Init SCALER v1718\n");
  caenst = CAENVME_SetScalerConf(BHandle,limit,AutoReset,
				 cvInputSrc0,cvManualSW,cvManualSW);
  status = 1-caenst;

  caenst = CAENVME_ReadRegister(BHandle, cvScaler0, &DataShort);
  status *= 1-caenst;
  printf("Scaler Reg config = %x \n",DataShort);

  caenst = CAENVME_EnableScalerGate(BHandle);
  status *= 1-caenst;
  return status;
}
  /*---------------   end setting IO reg section of V1718 ----------------*/

int init_pulser_1718(int32_t BHandle) {
  unsigned char Period=30, Width=4,PulseNo=1;
  int status, caenst;
  CVTimeUnits Unit;
  CVIOSources Start, Reset;

  printf("Init PULSER v1718\n");
  Unit = cvUnit25ns;
  Start = cvManualSW;
  Reset = cvManualSW;
  
  /*init pulserA*/
  caenst = CAENVME_SetPulserConf(BHandle,cvPulserA,Period,Width,
				Unit ,PulseNo,Start,Reset);
  status = 1-caenst;
  caenst = CAENVME_GetPulserConf(BHandle,cvPulserA,&Period,&Width,
				 &Unit,&PulseNo,&Start,&Reset);
  status *= 1-caenst;
  printf("Pulser config: Pulser= %d Period %d Width %d Units %d Npulse %d\n", 
	 cvPulserA,Period,Width,Unit,PulseNo);
  status *= 1-caenst;
  
  /*init PulserB*/
 
  Period=150;
  Width=10;
  
  caenst = CAENVME_SetPulserConf(BHandle,cvPulserB,Period,Width,
				Unit ,PulseNo,Start,Reset);
  status = 1-caenst;
  caenst = CAENVME_GetPulserConf(BHandle,cvPulserB,&Period,&Width,
				 &Unit,&PulseNo,&Start,&Reset);

  status *= 1-caenst;
  printf("Pulser  config: Pulser= %d Period %d Width %d Units %d Npulse %d\n", 
	 cvPulserB,Period,Width,Unit,PulseNo);
  status *= 1-caenst;

  return status;
}

/*****************************************************************************/

/* SET OUT0 TO LOGIC LEVEL 1*/
int set_veto_1718(int32_t BHandle){
    return 1 - CAENVME_SetOutputRegister(BHandle, cvOut0Bit);
}

/*****************************************************************************/

/* SET OUT0 TO LOGIC LEVEL 0*/

int clear_veto_1718(int32_t BHandle){
    return 1 - CAENVME_ClearOutputRegister(BHandle, cvOut0Bit);
}


 /*------------------------------------------------------------------------*/

int read_scaler_1718(int32_t BHandle) {
  
  unsigned DataShort=0;
  int status, caenst;
  caenst = CAENVME_ReadRegister(BHandle, cvScaler1, &DataShort);
  status = (1-caenst); 
  caenst = CAENVME_ResetScalerCount(BHandle);
  status *= (1-caenst); 
  return DataShort;
}
 /*------------------------------------------------------------------------*/

/*CHECK FOR INTERRUPTS ON THE VME BUS ptrig IS TRUE IF AN ITERRUPT IS FOUND*/
int trigger_interrupt(int32_t BHandle, bool *ptrig) {
    *ptrig = false;
    CAEN_BYTE mask;

    int status = CAENVME_IRQCheck(BHandle, &mask);
    if(mask != 0){
        *ptrig = true;
    }

    return 1 - status;
}

/**************************************************************************/
int trigger_scaler_1718(int32_t BHandle, bool *ptrig) {
  
  unsigned DataShort=0;
  int status, caenst;
  bool trigger = false;
  caenst = CAENVME_ReadRegister(BHandle, cvScaler1, &DataShort);
  status = (1-caenst); 
  if(DataShort>=1){ /*ma se ci metti maggiore o uguale a 1 invece che ==?Solo per evitare l'incastro inizale..funziona.. lasciamo cosi' .. 4aprile2011*/
    if(DataShort>1){
      cout<<"Warning: scaler > 1"<<endl;
    }
    printf("status=%d    %u\n", caenst, DataShort); // DEBUG
    trigger = true;
    caenst = CAENVME_ResetScalerCount(BHandle);
    status *= (1-caenst); 
  }
  *ptrig = trigger;
  return status;
  
}

 /*------------------------------------------------------------------------*/


int clearbusy_1718(int32_t BHandle) {

  int status = 1; int caenst;

  unsigned short Mask=0;
  Mask = cvOut0Bit + cvOut1Bit;
  caenst = CAENVME_PulseOutputRegister(BHandle,Mask);
  status *= (1-caenst); 
  return status;
}

 /*------------------------------------------------------------------------*/


int clearbusy_new_1718(int32_t BHandle) {

  int status = 1; int caenst=0;

  caenst = CAENVME_StartPulser(BHandle,cvPulserA);
  status = (1-caenst); 
  return status;
}

/*---------------------------------------------------------------*/
int read_trig_1718(int32_t BHandle, bool *ptrig) {

  int status = 1;
  int caenst;
  unsigned DataShort=0;
  int ch0 =0, ch1=0;

  unsigned short Mask=0;
  Mask = cvOut0Bit + cvOut1Bit;

  bool trig =false;

  int ncy=0,ncycles=1000000000;
  while(!trig && ncy<ncycles){ 
    
    caenst = CAENVME_ReadRegister(BHandle,cvInputReg, &DataShort);
    status *= (1-caenst); 
    //    int_to_binary(DataShort&0xff);
    ch0= (int) (DataShort & cvIn0Bit);
    ch1= (int) (DataShort & cvIn1Bit)>1;
    //    printf("Input reg :  %i  \n",(DataShort&0xff));
    if(ch0 || ch1) trig = true;
    ncy++;     
  }
  if(ncy == ncycles) {printf("TIMEOUT of V1718!!!!\n");}
  /*
    reset of the input register after finding the trigger
   */
  DataShort = 0;
  caenst = CAENVME_WriteRegister(BHandle,cvInputReg,DataShort);
  status *= (1-caenst); 

  caenst = CAENVME_ReadRegister(BHandle,cvInputReg, &DataShort);
  status *= (1-caenst); 
  //    int_to_binary(DataShort&0xff);
  ch0= (int) (DataShort & cvIn0Bit);
  ch1= (int) (DataShort & cvIn1Bit)>1;
  printf("ch0= %i  ch1 = %i \n",ch0,ch1);

  *ptrig = trig;
  return status;
}
