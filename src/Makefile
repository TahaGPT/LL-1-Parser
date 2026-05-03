CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
 
SRCS   = main.cpp grammar.cpp first_follow.cpp parser.cpp \
         stack.cpp tree.cpp error_handler.cpp
OBJS   = $(SRCS:.cpp=.o)
TARGET = ll1_parser
 
# only match grammar files - those whose name has NO underscore
# shell find is more reliable than wildcard for this
GRAMMARS := $(shell find input -maxdepth 1 -name '*.txt' \
              | grep -v '_' | sort)
 
all: $(TARGET)
 
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
 
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
 
run: $(TARGET)
	@mkdir -p output
	./$(TARGET) $(GRAMMARS)
 
# run on one grammar: make run-one FILE=input/grammar1.txt
run-one: $(TARGET)
	@mkdir -p output
	./$(TARGET) $(FILE)
 
clean:
	rm -f $(OBJS) $(TARGET)
 
clean-output:
	rm -rf output/
 
.PHONY: all run run-one clean clean-output