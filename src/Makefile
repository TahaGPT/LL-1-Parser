CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

SRCS   = main.cpp grammar.cpp first_follow.cpp parser.cpp
OBJS   = $(SRCS:.cpp=.o)
TARGET = ll1_parser

# Auto-discover all grammar files in input/, or override on the command line:
#   make GRAMMARS="input/grammar1.txt input/grammar2.txt"
GRAMMARS ?= $(wildcard input/*.txt)

# ----------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# Create dirs, then run the parser on every discovered grammar file
run: $(TARGET)
	@mkdir -p output
	./$(TARGET) $(GRAMMARS)

# Run on a single specific file, e.g.:  make run-one FILE=input/grammar2.txt
run-one: $(TARGET)
	@mkdir -p output
	./$(TARGET) $(FILE)

clean:
	rm -f $(OBJS) $(TARGET)

clean-output:
	rm -rf output/

.PHONY: all run run-one clean clean-output
