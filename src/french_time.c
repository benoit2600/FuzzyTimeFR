#include "french_time.h"

static const char* STR_HEURE = "heure";
static const char* STR_H = " h.";
static const char* STR_MOINS = "moins";

static const char* const HEURES[] = {
	"minuit","une","deux","trois","quatre","cinq","six","sept","huit","neuf","dix","onze","midi"};

static const char* const MINS[] = {
	"pile","cinq","dix","et quart","vingt","et demi","le quart","environ","trente"};

static const char* const JOURS[] = {
	"Dimanche","Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi"};

static const char* const MOIS[] = {
	"Jan.","Fév.","Mars","Avril","Mai","Juin","Juil.","Août","Sep.","Oct.","Nov.","Déc."};

int fuzzy_time(char* line1, char* line2, char* line3, char* line4, struct tm * t) {

	int hours = t->tm_hour;
	int minutes = t->tm_min;
	int nbLine = 3;
	strncpy(line1, "",LINE_BUFFER_SIZE -1);
	strncpy(line2, "",LINE_BUFFER_SIZE -1);
	strncpy(line3, "",LINE_BUFFER_SIZE -1);
	strncpy(line4, "",LINE_BUFFER_SIZE -1);	
	
	if (minutes >= 38) hours++;
	if (hours >= 24) hours = 0;
	if (hours > 12) hours -= 12;
	
	strcat(line1, HEURES[hours]); // on affiche l'heure
	
	if ((minutes >= 0 && minutes < 33) || (minutes >= 58 && minutes < 60) ) 
	{    // on rajoute la ligne de l'heure si inferieur a 33min
		if (hours > 0 && hours < 12) 
		{
			strncpy(line2, STR_HEURE,LINE_BUFFER_SIZE -1);
			if (hours > 1) strcat(line2, "s");
		}
		else
		{
			strncpy(line1, "",LINE_BUFFER_SIZE -1);
			strcat(line2, HEURES[hours]);
		}
	}
	else if(minutes >= 33 && minutes < 58)
	{
		if(hours > 0 && hours < 12) strcat(line1, STR_H);
	}
	if (minutes >= 38 && minutes < 58) strcat(line2, STR_MOINS);
	
	
	if (minutes >= 0 && minutes < 3) 
	{
		if (minutes == 0) strcat(line3, MINS[0]); // pile !
	}
	else if (minutes < 8)  strcat(line3, MINS[1]); // cinq
	else if (minutes < 13) strcat(line3, MINS[2]); // dix
	else if (minutes < 18) strcat(line3, MINS[3]); // et quart
	else if (minutes < 23) strcat(line3, MINS[4]); // vingt
	else if (minutes < 28) // vingt-cinq
	{
		strcat(line3, MINS[4]); 
		strcat(line4, MINS[1]); 
		nbLine = 4;
	}
	else if (minutes < 33)  strcat(line3, MINS[5]); // et demi
	else if (minutes < 38) // trente cinq
	{
		strcat(line2, MINS[8]);
		strcat(line3, MINS[1]);
	}
	else if (minutes < 43) strcat(line3, MINS[4]); // moins vingt
	else if (minutes < 48) strcat(line3, MINS[6]); // moins le quart
	else if (minutes < 53) strcat(line3, MINS[2]); // moins dix
	else if (minutes < 58) strcat(line3, MINS[1]); // moins cinq
	else if (minutes >= 58) strncpy(line3, MINS[7],LINE_BUFFER_SIZE -1); // presque (ici environ)

	
	if (hours == 4){
		nbLine = 4;
		strncpy(line2, STR_HEURE,LINE_BUFFER_SIZE -1);
		if (minutes >= 33 && minutes < 38) // moins vingt cinq
		{
			strncpy(line3, MINS[8],LINE_BUFFER_SIZE -1); // -- vingt
			strncpy(line4, MINS[1],LINE_BUFFER_SIZE -1); // cinq
		}
		else if (minutes >= 38 && minutes < 58)
		{
			strncpy(line3, STR_MOINS,LINE_BUFFER_SIZE -1);
			if (minutes < 43) strcat(line4, MINS[4]); // moins vingt
			else if (minutes < 48) strncpy(line4, MINS[6],LINE_BUFFER_SIZE -1); // moins le quart
			else if (minutes < 53) strncpy(line4, MINS[2],LINE_BUFFER_SIZE -1); // moins dix
			else if (minutes < 58) strncpy(line4, MINS[1],LINE_BUFFER_SIZE -1); // moins cinq
		}
		else if (minutes >= 58) strncpy(line3, MINS[7],LINE_BUFFER_SIZE -1); // presque
	}
	return nbLine;
}

void info_lines(char* line1,struct tm * t) {
	
	char wday[LINE_BUFFER_SIZE];
	char mday[LINE_BUFFER_SIZE];
	char mon[LINE_BUFFER_SIZE];
	strncpy(line1, "",LINE_BUFFER_SIZE -1);
	snprintf(wday, LINE_BUFFER_SIZE, "%s", JOURS[t->tm_wday]);
	snprintf(mday, LINE_BUFFER_SIZE, "%d", t->tm_mday);
	snprintf(mon, LINE_BUFFER_SIZE, "%s", MOIS[t->tm_mon]);
	strcat(line1, wday);
	strcat(line1, " ");
	strcat(line1, mday);
	strcat(line1, " ");
	strcat(line1, mon);


}

void majMinute(char * str, struct tm * t){
	//str[0] = 'a';
	//strcpy(str, "a");
	snprintf(str, 3, "%d", t->tm_min);
}
