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




/*   //Enable/disable zero suppression, overflow sup., common start-stop */
/*   if(nZS) { */
/*     address = Vdd00N_ADDRESS + 0x1032; */
/*     //    DataLong = 0x18; //Disable Zero Suppression + disable overfl */
/*     DataLong = 0x218; //Disable Zero Suppression + disable overfl + com stop */
/*     //    status = vme_write_dt(address,&DataLong, AD32, D16); */
/*     caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16); */
/*     status *= (1-caenst);  */
/*     if(status != 1) { */
/*       printf("Could not disable ZS: %li\n", DataLong); */
/*     } */
/*   } else { */
/*     address = Vdd00N_ADDRESS + 0x1032; */
/*     //    DataLong = 0x88; //Enable Zero Suppression + disable overfl */
/*     DataLong = 0x88; //Enable Zero Suppression + disable overfl + com stop */
/*     //  status = vme_write_dt(address,&DataLong, AD32, D16); */
/*     caenst = CAENVME_WriteCycle(BHandle,address,&DataLong,cvA32_U_DATA,cvD16); */
/*     status *= (1-caenst);  */
/*     if(status != 1) { */
/*       printf("Could not disable ZS: %li\n", DataLong); */
/*     } */
/*   } */

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



/*------------------------------------------------------------------*/

vector<int> read_tdcdd00(int32_t BHandle, int pstatus)
{
  /*
    reading of the Vdd00N 
    returns vector with output
  */
  vector<int> tdc_val; tdc_val.clear(); tdc_val.resize(Vdd00N_CHANNEL);
  vector<int> outD;
  int caenst;
  unsigned long address, data, dt_type;
  unsigned long tdcdd00_rdy, tdcdd00_busy, evtnum;
  unsigned long full,empty, evc_lsb, evc_msb, tdc_value, tdc_chan;
  unsigned int ncha;
  int status = 1; 

  /*
    check if the fifo has something inside: use status register 1
   */  
  address = Vdd00N_ADDRESS + tdcdd00_shift.statusreg1;



  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status = (1-caenst); 
  printf("status is: %d\n", status);

  if(tdcdd00_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdcdd00_rdy = data & tdcdd00_bitmask.rdy;
  //  tdcdd00_rdy = 1; se commento la riga di prima e inserisco questa, nell'if entra arrivando fino alla fine
  tdcdd00_busy = data & tdcdd00_bitmask.busy;

  //Trigger status
  address = Vdd00N_ADDRESS + 0x1020; //0x1020 = event trigger register
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdcdd00_debug) printf("ST (trg1) :: %i, %lx, %lx \n",status,address,data);


  //Event counter register
  address = Vdd00N_ADDRESS + 0x1024; //0x1024 = event counter register
  //  status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 

  evc_lsb = data;
  printf("data is: %lu\n", data);

  address = Vdd00N_ADDRESS + 0x1026;
  // status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_msb = data & 0xFF; //seleziono 0-8 di data
  //  printf("evc_msb is: %lu\n",evc_msb);
  //  printf("tdcdd00_rdy = %lu\ntdcdd00_busy = %lu\n", tdcdd00_rdy, tdcdd00_busy); 
  if(tdcdd00_rdy == 1 && tdcdd00_busy == 0) {
  
  /*
      Data Ready and board not busy
    */
    
    //Read the Event Header
    address = Vdd00N_ADDRESS;
    //status = vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status *= (1-caenst); 
    if(tdcdd00_debug) printf("Data Header :: %i, %lx, %lx \n",status,address,data);
    ncha =  data>>8 & 0x3F;
    if(tdcdd00_debug) cout<<"Going to Read "<<ncha<<" channels!"<<endl;
    full = 0; empty = 0;
    while(!full && !empty) {

      //Read MEB for each channel and get the TDC value
      address = Vdd00N_ADDRESS;
      //status = vme_read_dt(address,&data,AD32,D32);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
      status *= (1-caenst); 
      if(tdcdd00_debug) printf("Reading and got:: %i %lx %lx\n", status, data, address);

      dt_type = data>>24 & 0x7;
      if(tdcdd00_debug) printf("typ:: %lx\n", dt_type);

      //if(!(dt_type & 0x7)) {
      if(! dt_type){
	tdc_value = data & 0xFFF;
	tdc_chan = data>>17 & 0xF;
	if(tdcdd00_debug) cout<<tdc_value<<" "<<tdc_chan<<" "<<endl;
	tdc_val[tdc_chan] = tdc_value;
	if(data>>12 & 0x1) cout<<" Overflow, my dear !! "<<tdc_value<<endl;
	} else if(dt_type & 0x4) {
	// }else if (data & 0x4){
	//EOB
	evtnum = data & 0xFFFFFF;
	if(tdcdd00_debug) cout<<"EvtNum "<<evtnum<<endl;
	} else if(dt_type & 0x2) {
	// } else if(data & 0x2) {
	//Header
	if(tdcdd00_debug) cout<<" ERROR:: THIS SHOULD NOT HAPPEN!!!"<<endl;
      }

      //Check the status register to check the MEB status
      address = Vdd00N_ADDRESS + tdcdd00_shift.statusreg2;
      //status = vme_read_dt(address,&data,AD32,D16);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
      status *= (1-caenst); 
  
      full = data &  tdcdd00_bitmask.full;
      
      empty = data &  tdcdd00_bitmask.empty;
      if(full) {
	cout<<"MEB Full:: "<<full<<" !!!"<<endl;
      }
    }
    //Write event in output vector
    for(int iC = 0; iC<(int)tdc_val.size(); iC++) {
      outD.push_back(tdc_val[iC]);
    }

       }
  pstatus = status;
  return outD;
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




    


/*-------------------------------------------------------------*/


vector<int> readFasttdcdd00(int32_t BHandle, short int* pstatus)
{
  /*
    Implements Block Transfer Readout of Vdd00N 
    returns vector with output
  */
  int nbytes_tran = 0;
  vector<int> outD; outD.clear();
  int status = 1, caenst=0;
  unsigned long data,address;
  unsigned long dataV[34]; int wr;
  unsigned long tdcdd00_rdy, tdcdd00_busy;
  unsigned int ncha, idV;
  struct timeval tv;
  time_t curt, curt2, tdiff, tdiff1, curt3;
  /*
    check if the fifo has something inside: use status register 1
  */  
  
  if(tdcdd00_debug) {
    gettimeofday(&tv, NULL); 
    curt3=tv.tv_usec;
  }

  address = Vdd00N_ADDRESS + tdcdd00_shift.statusreg1;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdcdd00_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdcdd00_rdy = data & tdcdd00_bitmask.rdy;
  tdcdd00_busy = data & tdcdd00_bitmask.busy;

  //Event counter register
  if(tdcdd00_debug) {
    gettimeofday(&tv, NULL); 
    curt=tv.tv_usec;
  }

  if(tdcdd00_rdy == 1 && tdcdd00_busy == 0) {
    /*
      Data Ready and board not busy
    */

    //Read the Event Header
    address = Vdd00N_ADDRESS;
    //status = vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status *= (1-caenst); 
    //I put the header in
    outD.push_back((int)data);

    if(tdcdd00_debug) printf("Data Header :: %i, %lx, %lx \n",status,address,data);
    ncha =  data>>8 & 0x3F;
    if(tdcdd00_debug) cout<<"Going to Read "<<ncha<<" channels!"<<endl;

    //Vector reset
    idV = 0; while(idV<34) { dataV[idV] = 0; idV++; }
    wr = (ncha+1)*4;
    //    printf("numchann %d \n",ncha);

    //status *= vme_read_blk(address,dataV,wr,AD32,D32);    
    caenst = CAENVME_BLTReadCycle(BHandle,address,dataV,wr,
				  cvA32_U_DATA,cvD32,&nbytes_tran);
    status *= (1-caenst); 

    //Vector dump into output
    idV = 0; while(idV<ncha+1) { outD.push_back((int)dataV[idV]); idV++;  }
  }

  if(tdcdd00_debug) {
    gettimeofday(&tv, NULL); 
    curt2=tv.tv_usec;
    tdiff = (curt2-curt);
    tdiff1 = (curt-curt3);
    cout<<"Just BLT "<<curt<<" "<<curt2<<" "<<tdiff<<" "<<tdiff1<<endl;
  }
  *pstatus = (short int) status; 
  return outD;
}
/*-------------------------------------------------------------*/


unsigned short read_tdcdd00_simple(int32_t BHandle, int *pDataTdc)
{

  /*
    reading of the Vdd00N returns pointer to vector with output
  */
  int tdc_val[Vdd00N_CHANNEL]={0};
  int status=0, caenst=0;
  unsigned long address, data, dt_type;
  unsigned long tdcdd00_rdy, tdcdd00_busy, evtnum;
  unsigned long full,empty, evc_lsb, evc_msb, tdc_value, tdc_chan;
  unsigned int ncha;
  status = 1; 

  /*
    check if the fifo has something inside: use status register 1
   */  
  address = Vdd00N_ADDRESS + tdcdd00_shift.statusreg1;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status = (1-caenst); 

  if(tdcdd00_debug) printf("ST (str1) :: %i, %lx, %lx \n",status,address,data);
  tdcdd00_rdy = data & tdcdd00_bitmask.rdy;
  tdcdd00_busy = data & tdcdd00_bitmask.busy;

  //Trigger status
  address = Vdd00N_ADDRESS + 0x1020;
  //status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  if(tdcdd00_debug) printf("ST (trg1) :: %i, %lx, %lx \n",status,address,data);


  //Event counter register
  address = Vdd00N_ADDRESS + 0x1024;
  //  status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_lsb = data;
  address = Vdd00N_ADDRESS + 0x1026;
  // status *= vme_read_dt(address,&data,AD32,D16);
  caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
  status *= (1-caenst); 
  evc_msb = data & 0xFF;
  
  if(tdcdd00_rdy == 1 && tdcdd00_busy == 0) {
    /*
      Data Ready and board not busy
    */
    
    //Read the Event Header
    address = Vdd00N_ADDRESS;
    //status = vme_read_dt(address,&data,AD32,D32);
    caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
    status *= (1-caenst); 
    if(tdcdd00_debug) printf("Data Header :: %i, %lx, %lx \n",status,address,data);
    ncha =  data>>8 & 0x3F;
    if(tdcdd00_debug) cout<<"Going to Read "<<ncha<<" channels!"<<endl;
    full = 0; empty = 0;
    while(!full && !empty) {

      //Read MEB for each channel and get the TDC value
      address = Vdd00N_ADDRESS;
      //status = vme_read_dt(address,&data,AD32,D32);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD32);
      status *= (1-caenst); 
      if(tdcdd00_debug) printf("Reading and got:: %i %lx %lx\n", status, data, address);

      dt_type = data>>24 & 0x7;
      if(tdcdd00_debug) printf("typ:: %lx\n", dt_type);

      if(!(dt_type & 0x7)) {
	tdc_value = data & 0xFFF;
	tdc_chan = data>>17 & 0xF;
	if(tdcdd00_debug) cout<<tdc_value<<" "<<tdc_chan<<" "<<endl;
	tdc_val[tdc_chan] = tdc_value;
	if(data>>12 & 0x1) cout<<"TDC Overflow, my dear !! "<<tdc_value<<endl;
      } else if(dt_type & 0x4) {
	//EOB
	evtnum = data & 0xFFFFFF;
	if(tdcdd00_debug) cout<<"EvtNum "<<evtnum<<endl;
      } else if(dt_type & 0x2) {
	//Header
	if(tdcdd00_debug) cout<<" ERROR:: THIS SHOULD NOT HAPPEN!!!"<<endl;
      }

      //Check the status register to check the MEB status
      address = Vdd00N_ADDRESS + tdcdd00_shift.statusreg2;
      //status = vme_read_dt(address,&data,AD32,D16);
      caenst = CAENVME_ReadCycle(BHandle,address,&data,cvA32_U_DATA,cvD16);
      status *= (1-caenst); 
  
      full = data &  tdcdd00_bitmask.full;
      empty = data &  tdcdd00_bitmask.empty;
      if(full) {
	cout<<"MEB Full:: "<<full<<" !!!"<<endl;
      }
    }

    //Write event in output vector
    for(int iC = 0; iC<Vdd00N_CHANNEL; iC++) {
      *(pDataTdc+iC)=tdc_val[iC];
    }
  }
  
  return status;
}