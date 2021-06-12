#include "vme_bridge.h"

#include <unistd.h>
#include "CAENVMElib.h"
#include "CAENVMEtypes.h" 
#include "CAENVMEoslib.h"

int bridge_init(int32_t &BHandle) {

  int status, caenst;
  CVBoardTypes   VMEBoard;
  short          Link, Device;
  BHandle = 0;
  unsigned DataShort=0;
  VMEBoard = cvV1718;
  Device = 0;
  Link = 0;
 
  /*Assegna il numero BHandle al bridge*/
  if( CAENVME_Init(VMEBoard, Link, Device, &BHandle) != cvSuccess ) 
    {
      printf("\n\n Error opening the device\n");
      return 0;
    }
  else{
    printf("VME initialized \n");
  }
  
  //Resetta il bridge
  CAENVME_SystemReset(BHandle);
  usleep(10000);

  //status e caenst definite sopra
  status = 1;
  caenst = CAENVME_ReadRegister(BHandle, cvStatusReg, &DataShort);
  status *= (1-caenst); 
  if(status==1){
    printf("V1718 Status reg \n Bridge is Controlled = %i, USB speed = %i, ",
	   (DataShort & 0x2)>>1,(DataShort & 0x8000)>>16);
    //    int_to_binary((DataShort&0xff));
  }

  return status;

}

int bridge_deinit(int32_t BHandle) {

  int caenst, status;
  status = 1;
  caenst = CAENVME_SystemReset(BHandle);
  status *= (1-caenst); 
  caenst = CAENVME_End(BHandle);
  status *= (1-caenst); 

  return status;

}
