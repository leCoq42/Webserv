include makerc/colors.mk

# Directories and File Names
NAME		:= webserv
SRC_DIR		:= src
BUILD_DIR	:= build
MAIN		:= main.c
RM			:= rm -rvf
HEADERS		= inc/Webserver.hpp
CC			= g++




# Include Paths
INCLUDES	= -I ./inc


# Compiler Flags
# CFLAGS			= -Wall -Wextra -Werror -Wunreachable-code -Ofast
CFLAGS			= 
INCLUDE_FLAGS	:= $(addprefix -I, $(sort $(dir $(HEADERS))))

# Source files
SRC =	


# Object files
OBJS        = $(addprefix $(BUILD_DIR)/, $(SRC:$(SRC_DIR)/%.c=%.o))
MAIN_OBJ    = $(addprefix $(BUILD_DIR)/, $(MAIN:%.c=%.o))


# Targets
all: $(NAME)


$(NAME): $(OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ $(INCLUDE_FLAGS) $(ARCHIVE_LIBFT) $(ARCHIVE_PRINTF) $(ARCHIVE_MLX_MAC) -o $(NAME)
	@printf "$(BLUE_FG)$(NAME)$(RESET_COLOR) created_archive\n"

$(MAIN_OBJ) $(OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@


# Cleaning Targets
clean:
	@$(RM) $(OBJS) $(MAIN_OBJ)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re debug rebug fsan resan