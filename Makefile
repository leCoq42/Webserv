#Directories and File Names
NAME        := webserv
SRC_DIR     := src
CGI_DIR     := cgi_src
CGI_BIN_DIR := html/cgi-bin
BUILD_DIR   := obj
MAIN        := main.cpp
RM          := rm -rf
HEADERS     := inc/Webserv.hpp
CC          := c++ --std=c++20

# Include Paths
INCLUDES    := $(shell find inc -type d -exec echo -I {} \;)

#Compiler Flags
CFLAGS := -Wall -Wextra -Werror -Wunreachable-code#-Ofast -march=native -flto

#Debug Flags
ifdef DEBUG
	CFLAGS += -g #-fsanitize=address,undefined #-D DEBUG
endif

#Source files
SRC         := $(wildcard $(SRC_DIR)/**/*.cpp)
CGI_SRC     := $(wildcard $(CGI_DIR)/*.cpp)

# Object files
MAIN_OBJ    := $(addprefix $(BUILD_DIR)/, $(MAIN:%.cpp=%.o))
OBJS        := $(addprefix $(BUILD_DIR)/, $(SRC:$(SRC_DIR)/%.cpp=%.o))
CGI_OBJS	:= $(addprefix $(BUILD_DIR)/, $(CGI_SRC:$(CGI_DIR)/%.cpp=%.o))

# CGI targets
CGI_TARGETS := $(patsubst $(CGI_DIR)/%.cpp,$(CGI_BIN_DIR)/%.cgi,$(CGI_SRC))

# Colors
RED			:= \033[31m
BLUE		:= \033[34m
YELLOW		:= \033[33m
GREEN    	:= \033[32m
RESET_COLOR	:= \033[0m

# Targets
all: $(NAME) $(CGI_TARGETS)

debug:
	$(MAKE) DEBUG=1

rebug: fclean debug

cgi:
	$(MAKE) $(CGI_TARGETS)

$(NAME): $(OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $(NAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CGI_BIN_DIR)/%.cgi: $(CGI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@
	@echo "$(GREEN)CGI script $@ created$(RESET_COLOR)"
	@echo "$(GREEN)./$(NAME) executable created$(RESET_COLOR)"

# Cleaning Targets
clean:
	@find ./obj -name "*.o" -type f -delete 
	@$(RM) ./logDir/*
	@echo "$(YELLOW)Object files deleted$(RESET_COLOR)"
	@echo "$(YELLOW)Log deleted$(RESET_COLOR)"
	
fclean: clean
	@$(RM) $(NAME)
	@$(RM) $(CGI_BIN_DIR)/*.cgi
	@echo "$(YELLOW)./$(NAME) executable deleted$(RESET_COLOR)"

re: fclean all

.PHONY: all clean fclean re debug cgi
