NAME = serverstruct
GCC = c++ --std=c++11
FLAGS = -Wall -Wextra -Werror #-fsanitize=address
SRCS = main.cpp file_handler.cpp server_struct/ProtoStruct.cpp server_struct/ServerStruct.cpp server_struct/ConfigContent.cpp server_struct/LocationStruct.cpp text_file_parser/ParserStruct.cpp text_file_parser/ParserItem.cpp text_file_parser/Parser.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(GCC) $(FLAGS) $(OBJS) -o $(NAME)

%.o:%.cpp
	$(GCC) $(FLAGS) $^ -c -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all 

.PHONY: all clean fclean re