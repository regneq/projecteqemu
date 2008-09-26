
all: cleanipc spell_explorer
	gmake -C azone

cleanipc: cleanipc.cpp
	g++ cleanipc.cpp -o cleanipc

spell_explorer: spell_explorer.cpp
	g++ -g spell_explorer.cpp -o spell_explorer

clean:
	rm -f cleanipc
	gmake -C azone clean

