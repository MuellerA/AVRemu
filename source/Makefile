CXXFLAGS = -g -std=c++11 -Wall

ALL:	AVRemu AVRtest

Clean:
	rm -f *.o AVRemu

avr.o: avr.h

atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o: avr.h instr.h

instr.o: instr.cpp avr.h instr.h

test.o main.o:	avr.h instr.h

AVRemu:	avr.o instr.o main.o atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o
	g++ avr.o instr.o main.o atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o -o AVRemu

AVRtest:	avr.o instr.o test.o atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o
	g++ avr.o instr.o test.o atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o -o AVRtest
