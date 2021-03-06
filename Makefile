

SOURCES = src/main.cpp src/game.cpp src/render.cpp src/letvezi.cpp src/persistent.cpp src/binary_serial.cpp
HEADERS = src/game.h src/render.h src/letvezi.h src/persistent.h src/binary_serial.h src/timer.h src/conc.h src/vect.h
OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE = build/letvezi

LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lpthread

INCLUDES = -I src/ -I libs/

ifdef $(RELEASE)
OPTS = -Ofast -s
else
OPTS = -O0 -g
endif

ifdef $(WINDOWS)
WARNS = -Wall
else
WARNS = -Wall -Wextra -Werror
endif

CXXFLAGS = -std=c++14 $(OPTS) $(WARNS) $(INCLUDES)

CXX = clang++

all : $(OBJECTS) $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	mkdir -p build/
	$(CXX) ${LDFLAGS} ${CXXFLAGS} ${OBJECTS} -o $(EXECUTABLE)

src/main.o: src/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/game.o: src/game.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/render.o: src/render.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/letvezi.o: src/letvezi.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/persistent.o: src/persistent.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/binary_serial.o: src/binary_serial.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f ${EXECUTABLE} ${OBJECTS}
