CXX = g++ 
CXXFLAGS = -Wall -Wextra -std=c++17 -Isfml/include -Isteamaudio/include
LDFLAGS = -Lsfml/lib -Lsteamaudio/lib/windows-x64
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lphonon
TARGET = sfml_steamaudio_test

SRCS = main.cpp steamaudiomanager.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run