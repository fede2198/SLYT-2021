#include <vector>
#include <fstream>

/* GENERAL SETTING  */
#define V1718 1
#define debug verbose
#define ADC792 1
// in use:
#define TDCdd00 1

/* 1 modulo attivo*/

#define WAIT_TIMEOUT 10000
#define HEADER_SIZE 7
#define EVT_SIZE HEADER_SIZE
short TOTAL_value[EVT_SIZE];
int verbose;
volatile bool exit_signal;

void print_usage(FILE* stream, char *name, int exit_status);
void int_handler(int sig);
