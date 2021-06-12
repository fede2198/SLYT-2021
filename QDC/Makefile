#------------------------------------------------------------------------------ 
# Title:        Makefile for the SLYT acquisition
# Date:         2012                                                    
# Author:	S.Fiore
# Platform:	Linux 2.4.x
# Language:	GCC 2.95 and 3.0
#------------------------------------------------------------------------------  
# daq for the CAEN bridge v1718


OBJS = main_daqpet_init.o v1718_lib.o vme_bridge.o adc792_lib.o tdc775_lib.o root.o
HEAD = main_daqpet.h v1718_lib.h vme_bridge.h adc792_lib.h tdc775_lib.h

INCLUDE_DIR = .

COPTS = -g -Wall -DLINUX -fPIC -I$(INCLUDE_DIR)

acquire_init: $(OBJS)
	g++ -m32 -g -DLINUX -o acquire_init $(OBJS) -lncurses -lm -lrt -l CAENVME $(shell root-config --glibs)


main_daqpet_init.o: main_daqpet_init.c main_daqpet.h v1718_lib.h vme_bridge.h adc792_lib.h tdc775_lib.h root.h
	g++ -m32 $(COPTS) -c main_daqpet_init.c

v1718_lib.o: v1718_lib.c v1718_lib.h 
	g++ -m32 $(COPTS) -c v1718_lib.c

vme_bridge.o: vme_bridge.c $(INCLUDE_DIR)/vme_bridge.h 	
	g++ -m32 $(COPTS) -c vme_bridge.c	

tdc775_lib.o: tdc775_lib.c tdc775_lib.h  
	g++ -m32 $(COPTS) -c tdc775_lib.c

adc792_lib.o: adc792_lib.c adc792_lib.h 
	g++ -m32 $(COPTS) -c adc792_lib.c

root.o: root.cpp root.h
	g++ -m32 $(COPTS) -c root.cpp -I$(shell root-config --incdir)

clean:
	-rm -f acquire
	-rm -f #*#
	-rm -f *~
	-rm -f core
	-rm -f $(OBJS)

