CXXFLAGS = -O2 -std=c++11 -Wall

LibObj = avr.o instr.o atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o
EmuObj = main.o $(LibObj)
TstObj = test.o $(LibObj)
AllObj = main.o test.o $(LibObj)

.PHONY:	ALL Clean

ALL:	AVRemu AVRtest

Clean:
	rm -f $(AllObj) AVRemu AVRtest

$(AllObj): avr.h instr.h

AVRemu:	$(EmuObj)
	g++ -o AVRemu $(EmuObj)

AVRtest: $(TstObj)
	g++ -o AVRtest $(TstObj)
