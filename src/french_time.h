#pragma once

#include <pebble.h>
#include <time.h>
#include <string.h>	
#include "fuzzy_french_plus.h"
	
int fuzzy_time(TheTime * timeStr, struct tm * t);

void info_lines(char* str_line1, struct tm * t);

void majMinute(char * str, struct tm * t);