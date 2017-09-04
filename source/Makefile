CXXFLAGS = -g -std=c++11 -Wall

ALL:	AVRemu

Clean:
	rm -f *.o AVRemu

avr.o: avr.cpp avr.h

instr.o: instr.cpp avr.h instr.h

main.o:	avr.h instr.h

AVRemu:	avr.o instr.o main.o
	g++ avr.o instr.o main.o -o AVRemu
