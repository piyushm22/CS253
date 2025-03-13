CXX = g++
CXXFLAGS = -std=c++14 -Wall
TARGET = final
SRCS = final.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)