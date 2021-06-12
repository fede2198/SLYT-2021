// V792N -> Vdd00N , adc792 -> tdcdd00 , a792 -> tdd00 , V792N_IPED -> Vdd00N_FSR
#include <vector>
#ifndef _Vdd00N_HEADER_
#define _Vdd00N_HEADER_

#define Vdd00N_ADDRESS               0xdd000000
#define Vdd00N_CHANNEL               16

#define Vdd00N_REG1_STATUS           0x100e
#define Vdd00N_REG2_STATUS           0x1022
#define Vdd00N_REG1_CONTROL          0x1010
#define Vdd00N_BIT_SET1              0x1006
#define Vdd00N_BIT_CLEAR1            0x1008
#define Vdd00N_BIT_SET2              0x1032
#define Vdd00N_BIT_CLEAR2            0x1034
#define Vdd00N_OUTPUT_BUFFER         0x0000
#define Vdd00N_MCST_CBLT_ADDRESS     0x1004
#define Vdd00N_INTERRUPT_LEVEL       0x100a
#define Vdd00N_INTERRUPT_VECTOR      0x100c
#define Vdd00N_ADER_HIGH             0x1012
#define Vdd00N_ADER_LOW              0x1014
#define Vdd00N_MCST_CBLT_CTRL        0x101a
#define Vdd00N_EVENT_TRIGGER_REG     0x1020
#define Vdd00N_EVENT_COUNTER_L       0x1024
#define Vdd00N_EVENT_COUNTER_H       0x1026
#define Vdd00N_FCLR_WINDOW           0x102e
#define Vdd00N_W_MEM_TEST_ADDRESS    0x1036
#define Vdd00N_MEM_TEST_WORD_HIGH    0x1038
#define Vdd00N_CRATE_SELECT          0x103c
#define Vdd00N_FSR                   0x1060
#define Vdd00N_R_TEST_ADDRESS        0x1064
#define Vdd00N_SS_RESET_REG          0x1016
#define Vdd00N_FIRMWARE_REV          0x1000

//#define tdcdd00_OVFSUP    0 // overflow suppression ON
#define tdcdd00_OVFSUP    0x8 // overflow suppression OFF

//#define tdcdd00_ZROSUP    0 // zero suppression ON
#define tdcdd00_ZROSUP    0x10 // zero suppression OFF

#define tdcdd00_STASTOP    0 // Common START
//#define tdcdd00_STASTOP    0x400 // Common STOP

#define tdcdd00_LSB    280 // LSB=ps per count, roughly LSB=8.9*1000/tdcdd00_LSB, it can go from 35 to 300 ps 
#define tdcdd00_THR    0 // minimum value to be stored -SET to 0 

#define tdcdd00_debug   1

int init_tdcdd00(int32_t BHandle);
unsigned short read_tdcdd00_simple(int32_t BHandle, int *pDataTdc);
std::vector<int> read_tdcdd00(int32_t BHandle, int pstatus);
std::vector<int> readFasttdcdd00(int32_t BHandle, short int* pstatus);
std::vector<unsigned long> read_block_tdcdd00(int32_t BHandle, int& status);
int data_reset_tdcdd00(int32_t BHandle);

struct tdd00_shift{
  int datareg;
  int statusreg1;
  int statusreg2;
} ;
static const struct tdd00_shift tdcdd00_shift=
{0x0000,0x100e,0x1022};

struct tdd00_bitmask{
  unsigned long rdy;
  unsigned long busy;
  unsigned long full;
  unsigned long empty;
};
static const struct tdd00_bitmask tdcdd00_bitmask=
{0x1,0x4,0x4,0x2};
#endif