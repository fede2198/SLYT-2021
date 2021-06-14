// adc792 -> tdcdd00 , V792N -> Vdd00N , adc -> tdc
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <time.h> 
#include "CAENVMElib.h"
#include "CAENVMEtypes.h" 
#include "CAENVMEoslib.h"
#include "tdcdd00_lib.h"
#include <iostream>

using namespace std;

/*-------------------------------------------------------------*/

int init_tdcdd00(int32_t BHandle) {

  int status=1;
  unsigned long address;
  unsigned long  DataLong;
  //  int nZS = 1;
  int caenst;

  /*Firmware Revision */
  address = Vdd00N_ADDRESS + Vdd00N_FIRMWARE_REV;
  //  status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Error READING Vdd00N firmware -> address=%lx \n", address);
    return status;
  }
  else {
    if(tdcdd00_debug) printf("Vdd00N firmware is version:  %lx \n", DataLong);
  }
  
  //Bit set register
  address = Vdd00N_ADDRESS +  Vdd00N_BIT_SET1;
  DataLong = 0x80; //Issue a software reset. To be cleared with bit clear register access
  //  status = vme_write_dt(address,&DataLong, AD32, D16);
  //status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Bit Set Register read: %li\n", DataLong);
  }

  //Control Register: enable BLK_end
  address = Vdd00N_ADDRESS +  Vdd00N_REG1_CONTROL;
  DataLong = 0x4; //Sets bit 2 to 1 [enable blkend]
  //  status = vme_write_dt(address,&DataLong, AD32, D16);
  //status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Bit Set Register read: %li\n", DataLong);
  }

  //Bit clear register
  address = Vdd00N_ADDRESS +  Vdd00N_BIT_CLEAR1;
  DataLong = 0x80; //Release the software reset. 
  //  status = vme_write_dt(address,&DataLong, AD32, D16);
  //status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("Bit Clear Register read: %li\n", DataLong);
  }

  //Bit set register 2 according to header file values
  address = Vdd00N_ADDRESS + Vdd00N_BIT_SET2;
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  if(tdcdd00_debug) printf("Register 2 of TDC Vdd00N before writing is at %lx\n", DataLong);
  // DataLong = 0x5880 + tdcdd00_OVFSUP + tdcdd00_ZROSUP + tdcdd00_STASTOP;
  DataLong = 0x58b8;
    caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(status != 1) {
    printf("ERROR setting register 2 as %lx", DataLong);
  }
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  if(tdcdd00_debug) printf("Register 2 of TDC Vdd00N after writing is at %lx\n", DataLong);

  //Set the thresholds.
  for(int i=0; i<16; i++) {
    address = Vdd00N_ADDRESS + 0x1080 +4*i;
    if(tdcdd00_debug) printf(
	   "Address of the thr register %i : address = %lx\n",i,address);
    DataLong = tdcdd00_THR; //Threshold
    //status = vme_write_dt(address, &DataLong, AD32, D16);
    caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
    status *= (1-caenst); 
    if(tdcdd00_debug) printf("Threshold set: %li\n", DataLong);
    if(status != 1) {
      printf("Threshold register read: %li\n", DataLong);
    }
  }

  //Set the LSB value according to tdcdd00_LSB in header file
  address = Vdd00N_ADDRESS + Vdd00N_FSR;
  DataLong = 8.9*1000/(double)tdcdd00_LSB; 
  printf("DataLong %lu\n",DataLong);
  //  status = vme_read_dt(address, &DataLong, AD32, D16);
  caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  caenst = CAENVME_ReadCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdcdd00_debug) printf("LSB register read: %li\n", DataLong);
  if(status != 1) {
    printf("LSB register read: %li\n", DataLong);
  }
  return status;
 }



/*------------------------------------------------------------------*/

int data_reset_tdcdd00(int32_t BHandle){
	int ret_val;
	unsigned long DataLong;
	unsigned long address;

	DataLong = 0x4;
	/* asserting data reset */
	address = Vdd00N_ADDRESS + Vdd00N_BIT_SET2;
	ret_val = CAENVME_WriteCycle(BHandle,address,
			&DataLong,cvA32_U_DATA,cvD16);
	ret_val = CAENVME_ReadCycle(BHandle,address,
			&DataLong,cvA32_U_DATA,cvD16);
	if(!DataLong & 0x4){
		ret_val = 1;
		fprintf(stderr, "ERROR data reset not asserted\n");
		
	}


	DataLong = 0x4;
	address = Vdd00N_ADDRESS + Vdd00N_BIT_CLEAR2;
	/* releasing data reset */
	ret_val = CAENVME_WriteCycle(BHandle,address,
			&DataLong,cvA32_U_DATA,cvD16);
	ret_val = CAENVME_ReadCycle(BHandle, address,
			&DataLong,cvA32_U_DATA,cvD16);
	if(DataLong & 0x4){
		ret_val = 1;
			fprintf(stderr, "ERROR data reset not released\n");
	}

//	printf("DATALONG AT THE END IS: %lu\n", DataLong);

	return ret_val;
}

/*-------------------------------------------------------------*/


vector<unsigned long> read_block_tdcdd00(int32_t BHandle, int& status)
{
  
  int nbytes_tran = 0;
  unsigned long dataV[(Vdd00N_CHANNEL+2)*2*Vdd00N_CHANNEL]={0};
 
  // int wr= (Vdd00N_CHANNEL+2)*Vdd00N_CHANNEL*4*2; //pag 42 tdc manual: output buffer format
  int wr= (Vdd00N_CHANNEL+2)*Vdd00N_CHANNEL/4; //pag 42 tdc manual: output buffer format in bytes
  int caenst, caenst2;
  unsigned long address, address2, data, data2;
  unsigned long tdcdd00_rdy;

  /*
    check if the fifo has something inside: use status register 1
   */  
  //in adc792 : address = adcaddrs.at(idB) + V792N_REG1_STATUS; //i-th board address + 0x100e , here we only have one address

  address = Vdd00N_ADDRESS + Vdd00N_REG1_STATUS;
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status = (1-caenst); 

  if(tdcdd00_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdcdd00_rdy = data & tdcdd00_bitmask.rdy;

  address2 = Vdd00N_ADDRESS + Vdd00N_BIT_SET2;
  caenst2 = CAENVME_ReadCycle(BHandle,address2, &data2,cvA32_U_DATA,cvD16);
  //printf("REGISTRO 2 IN READ BLOCK  = %lu \n",data2);


  if(tdcdd00_rdy){
		address = Vdd00N_ADDRESS + Vdd00N_OUTPUT_BUFFER;	    

		status *= 1 - CAENVME_BLTReadCycle(BHandle,address,&dataV,wr, cvA32_U_BLT,cvD32,&nbytes_tran); //  wr = size of the transfer in byte and &nbytes_tran = number of transferred bits
		//status *= 1 -  CAENVME_ReadCycle(BHandle,address,&dataV, cvA32_U_DATA,cvD32);
 		 if(tdcdd00_debug && nbytes_tran != wr){
			fprintf(stderr, "warning different block size\n");
		}
	}
  //dataV is unsigned char *Buffer
   vector<unsigned long> outD(dataV, dataV + (Vdd00N_CHANNEL+2)*Vdd00N_CHANNEL*2);

  	return outD;
 
}
