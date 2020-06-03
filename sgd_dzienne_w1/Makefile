all: h.o lodepng.o client.o
	g++ h.o lodepng.o client.o -lSDL2 -o h

h.o: h.cpp
	g++ -std=c++14 h.cpp -c
	
client.o: net/client.cpp
	g++ -std=c++14 net/client.cpp -c

lodepng.o: lodepng.cpp
	g++ -std=c++14 lodepng.cpp -c
	
clean:
	rm -f *.o h
