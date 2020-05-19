all: h.o lodepng.o
	g++ h.o lodepng.o -lSDL2 -o h

h.o: h.cpp
	g++ -std=c++11 h.cpp -c
	

lodepng.o: lodepng.cpp
	g++ -std=c++11 lodepng.cpp -c
	
clean:
	rm -f *.o h
