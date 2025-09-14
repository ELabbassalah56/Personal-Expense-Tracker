#compiler =g++
CXX = g++

#compiler flages 
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -g

#Target executable
TARGET = expense_tracker

#Target deleting
TARGET_DELETE = $(TARGET)

#Source files
SRCS = $(shell find src -name '*.cpp')

#Objects files 
OBJS = $(patsubst src/%.cpp, build/%.o, $(SRCS))

#Include directory
INCLUDES = -I./include

#Library directory
LIBDIRS = 
#Libraries
LIBS = 

#Phony targets
.PHONY: all clean mrproper run build

#Makefile rules
all: $(TARGET)

#linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBDIRS) $(LIBS)

#compiling
build/%.o: src/%.cpp | build
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

#cleaning
clean:
	rm -f ./build/*.o

#removing the target executable
mrproper: clean
	rm -f $(TARGET_DELETE)

# Optional: build and run the application
run: all
	./$(TARGET)