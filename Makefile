# Compiler
C++ = g++

# Compiler flags
C++FLAGS = -Wall -Wextra -Werror -std=c++11 

# Include directories
INCLUDES = -I./inc

# Directories
SRC_DIRS = src/sockets src/cgi src/parser src/request src/response
OBJ_DIR = ./obj
BIN_DIR = ./bin

# Source files
SRC_FILES = $(wildcard main.cpp $(addsuffix /*.cpp, $(SRC_DIRS)))
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC_FILES:.cpp=.o)))

# Target executable
TARGET = $(BIN_DIR)/webserv

# Default target
all: $(TARGET)

# Linking target
$(TARGET): $(OBJ_FILES)
	$(C++) $(C++FLAGS) -o $@ $^

# Compiling source files to object files
$(OBJ_DIR)/%.o: %.cpp
	$(C++) $(C++FLAGS) -c -o $@ $<

# Cleaning up generated files
re: fclean all

clean:
	rm ./obj/*

fclean: clean
	rm ./dir/*

# Phony targets
.PHONY: all clean fclean