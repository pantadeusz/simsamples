
CPP=g++

SDLLIB=/home/PJWSTK/pantadeusz/public_html/sdl/SDL2-2.0.4/build/.libs
LIBS=-L$(SDLLIB) -lSDL2 -lGL
CARGS=-std=c++17 -I/home/PJWSTK/pantadeusz/public_html/sdl/SDL2-2.0.4/include/SDL -I/usr/include/SDL2 -I../


APP=game

all: $(APP) 

$(APP): game.cpp
	$(CPP) $(CARGS) $(LIBS) game.cpp -o $(APP) $(LIBS)

run: all
	@LD_LIBRARY_PATH=$(SDLLIB) $(APP)

clean:
	rm -f $(APP)

