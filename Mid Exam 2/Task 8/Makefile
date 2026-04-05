# ---------------------------------------------------------
# mingw32-make         <- Կոմպիլյացիա անելու համար
# mingw32-make run     <- Կոմպիլյացիա և միանգամից միացնելու համար
# mingw32-make clean   <- Ավելորդ ֆայլերը ջնջելու համար
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
