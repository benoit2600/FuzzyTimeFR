#pragma once

#include <pebble.h>
#include "mini-printf.h"
#include <time.h>
#include <string.h>	

#define LINE_BUFFER_SIZE 10

void fuzzy_time(char* str_line1, char* str_line2, char* str_line3,char* str_line4, struct tm * t);

void info_lines(char* str_line1, struct tm * t);

void majMinute(char * str, struct tm * t);