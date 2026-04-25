# ---------------------------------------------------------
# mingw32-make
# mingw32-make run
# mingw32-make clean 
# ---------------------------------------------------------

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
TARGET = calculator.exe
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)
