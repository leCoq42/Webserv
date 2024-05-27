#pragma once

#define BUFFER_SIZE 32

#include <iostream>
#include <fstream>
#include <list>
#include "../ServerStruct.hpp"
#include "../Parser.hpp"

//file_handler.cpp
int	load_file_to_buff(char *file_name, char **buffer, int *file_len);
