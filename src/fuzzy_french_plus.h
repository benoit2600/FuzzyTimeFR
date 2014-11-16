/*
	Fuzzy French + SDK 2.0
	Based on the  Pebble-Fuzzy-French-Plus by mandlar, 
	ported on SDK 2.0, add some options.
 */
#ifndef FUZZY
#define FUZZY
 
#pragma once
#include <pebble.h>
#include <stdlib.h>
	
#define ANIMATION_DURATION 1000
#define LINE_BUFFER_SIZE 11
#define NB_MAX_LINE 4
#define WINDOW_NAME "fuzzy_french_plus"

enum {
	MINUTEPRECISE_KEY = 0x0,
	COULEUR_KEY = 0x1,
	BATTERIE_KEY = 0x2,
	TOPBAR_KEY = 0x3
};


typedef struct {
	TextLayer * layer[2];
	PropertyAnimation * layer_animation[2];
	bool wayAnimation; 
} TextLine;
 
typedef struct{
	TextLayer * layer;
	int line;
		
} TextLayerAnim;
	
typedef struct { /* Contient les chaines de caractères de chacun des layers*/
	char lineStr[NB_MAX_LINE][LINE_BUFFER_SIZE]; // the string of each line
	char topbar[20];
	char minutePrecise[3];

} TheTime;


typedef struct {
	TextLayer* layer;
	bool wayAnimation; 
} TextLayerBool;
#endif