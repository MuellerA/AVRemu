CXX	= /usr/bin/g++
CXXFLAGS = -O2 -std=c++11 -Wall


LibObj = avr.o instr.o io.o filter.o atmegaXX8.o atmega8.o attinyX5.o attinyX4.o atxmegaAU.o
EmuObj = main.o execute.o $(LibObj)
MatchObj = match.o $(LibObj)
TstObj = test.o $(LibObj)
AllObj = main.o execute.o test.o $(LibObj)

.PHONY:	ALL Clean tags

ALL:	AVRemu AVRtest AVRmatch tags

tags:
	ctags -R . || true
Clean:
	rm -f $(AllObj) AVRemu AVRtest

$(AllObj): avr.h instr.h io.h filter.h

execute.o main.o: execute.h

AVRemu:	$(EmuObj)
	$(CXX) -o AVRemu $(EmuObj)

AVRmatch: $(MatchObj)
	$(CXX) -o AVRmatch $(MatchObj)

AVRtest: $(TstObj)
	$(CXX) -o AVRtest $(TstObj)

