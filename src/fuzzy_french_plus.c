/*
	Fuzzy French +
	Inspired by Fuzzy Time and Fuzzy Time +
	With Date, 24H display and Week #
 */

#include <pebble.h>
#include "french_time.h"
#include <stdlib.h>
#define ANIMATION_DURATION 1000
#define LINE_BUFFER_SIZE 10
#define WINDOW_NAME "fuzzy_french_plus"

Window * window;

typedef struct {
	TextLayer* layer[2];
	PropertyAnimation * layer_animation[2];
} TextLine;

typedef struct { /* Contient les chaines de caractères de chacun des layers*/
	char line1[LINE_BUFFER_SIZE];
	char line2[LINE_BUFFER_SIZE];
	char line3[LINE_BUFFER_SIZE];
	char line4[LINE_BUFFER_SIZE];
	char topbar[20];
	char minutePrecise[3];

} TheTime;

static AppSync sync;
static uint8_t sync_buffer[64];
enum {
	MINUTEPRECISE_KEY = 0x0,
	COULEUR_KEY = 0x1,
	BATTERIE_KEY = 0x2,
	TOPBAR_KEY = 0x3
};
static int CminutePrecise;
static int Ccouleur;
static int Cbatterie;
static int Ctopbar;

int INTColorClear = GColorClear;
int INTColor1 = GColorBlack;
int INTColor2 = GColorWhite;

int nbLine = 0;
int nbLineNew = 0;

TextLayer * topbar; 
TextLayer * minutePrecise;
TextLayer * batterie;
TextLine * line1;
TextLine * line2;
TextLine * line3;
TextLine * line4;

static TheTime cur_time;
static TheTime new_time;

static bool busy_animating_in = false;
static bool busy_animating_out = false;
/*réglage sur 4 lignes */
const int line1_y = 7; 
const int line2_y = 46;
const int line3_y = 80;
const int line4_y = 116;
/*réglage sur 3 lignes */
const int line1_y3 = 18; 
const int line2_y3 = 56;
const int line3_y3 = 94;
const int line4_y3 = 200;



void animationInStoppedHandler(struct Animation *animation, bool finished, void *context) {
	busy_animating_in = false;
	cur_time = new_time; 	// reset cur_time
}

void animationOutStoppedHandler(struct Animation *animation, bool finished, void *context) {
	// reset out layer to x=144
	TextLayer * outside = (TextLayer *)context;
	GRect rect = layer_get_frame( (Layer *) outside);
	if (rect.origin.y == line2_y || rect.origin.y == line2_y3 ) rect.origin.x = -144;
	else rect.origin.x = 144;
	layer_set_frame((Layer *) outside, rect);

	busy_animating_out = false;
}

void updateLayer(TextLine * animating_line, int line) {

	TextLayer * inside, *outside;
	GRect rect = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));
	inside = (rect.origin.x == 0) ? animating_line->layer[0] : animating_line->layer[1];
	outside = (inside == animating_line->layer[0]) ? animating_line->layer[1] : animating_line->layer[0];
	
	GRect in_rect = layer_get_frame((Layer *) outside);
	GRect out_rect = layer_get_frame((Layer *) inside);

	if (line == 2){
		in_rect.origin.x += 144;
		out_rect.origin.x += 144;
	} else {
		in_rect.origin.x -= 144;
		out_rect.origin.x -= 144;
	}

 // animate out current layer
	busy_animating_out = true;
	animating_line->layer_animation[1] = property_animation_create_layer_frame((Layer *)inside, NULL, &out_rect);
	animation_set_duration(&(animating_line->layer_animation[1]->animation), ANIMATION_DURATION);
	animation_set_curve(&(animating_line->layer_animation[1]->animation), AnimationCurveEaseOut);
	animation_set_handlers(&(animating_line->layer_animation[1]->animation), (AnimationHandlers) {
	.stopped = (AnimationStoppedHandler)animationOutStoppedHandler
	}, (void *)inside);
	animation_schedule(&(animating_line->layer_animation[1]->animation));

	if (line==1){
		text_layer_set_text(outside, new_time.line1);
		text_layer_set_text(inside, cur_time.line1);
	}
	if (line==2){
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "On Passe dans la ligne 2 maj des texlayers");
		text_layer_set_text(outside, new_time.line2);
		text_layer_set_text(inside, cur_time.line2);
	}
	if (line==3){
		text_layer_set_text(outside, new_time.line3);
		text_layer_set_text(inside, cur_time.line3);
	}
	if (line==4){
		text_layer_set_text(outside, new_time.line4);
		text_layer_set_text(inside, cur_time.line4);
	}
	// animate in new layer
	busy_animating_in = true;
	animating_line->layer_animation[0] = property_animation_create_layer_frame((Layer *)outside, NULL, &in_rect);
	animation_set_duration(&(animating_line->layer_animation[0]->animation), ANIMATION_DURATION);
	animation_set_curve(&(animating_line->layer_animation[0]->animation), AnimationCurveEaseOut);
	animation_set_handlers(&(animating_line->layer_animation[0]->animation), (AnimationHandlers) {
	.stopped = (AnimationStoppedHandler)animationInStoppedHandler
	}, (void *)outside);
	animation_schedule(&(animating_line->layer_animation[0]->animation));
}

void update_NBlayer(int nbLayer){
	updateLayer(line1, 1);
	updateLayer(line2, 2);
	updateLayer(line3, 3);
	updateLayer(line4, 4);
}


void update_TextLayerPosition(int nbLineNew){
		if(nbLineNew == 3){
			layer_set_frame((Layer *) line1->layer[0],GRect(0, line1_y3, 144, 50));
			layer_set_frame((Layer *) line2->layer[0],GRect(0, line2_y3, 144, 50));
			layer_set_frame((Layer *) line3->layer[0],GRect(0, line3_y3, 144, 50));
			layer_set_frame((Layer *) line4->layer[0],GRect(0, line4_y3, 144, 50));
			layer_set_frame((Layer *) line1->layer[1],GRect(144, line1_y3, 144, 50));
			layer_set_frame((Layer *) line2->layer[1],GRect(-144,line2_y3, 144, 50));
			layer_set_frame((Layer *) line3->layer[1],GRect(144, line3_y3, 144, 50));
			layer_set_frame((Layer *) line4->layer[1],GRect(144, line4_y3, 144, 50));
		}
		
		else{
			layer_set_frame((Layer *) line1->layer[0],GRect(0, line1_y, 144, 50));
			layer_set_frame((Layer *) line2->layer[0],GRect(0, line2_y, 144, 50));
			layer_set_frame((Layer *) line3->layer[0],GRect(0, line3_y, 144, 50));
			layer_set_frame((Layer *) line4->layer[0],GRect(0, line4_y, 144, 50));
			layer_set_frame((Layer *) line1->layer[1],GRect(144, line1_y, 144, 50));
			layer_set_frame((Layer *) line2->layer[1],GRect(-144,line2_y, 144, 50));
			layer_set_frame((Layer *) line3->layer[1],GRect(144, line3_y, 144, 50));
			layer_set_frame((Layer *) line4->layer[1],GRect(144, line4_y, 144, 50));	
		}
}

void update_watch(void) {
	time_t test = time(NULL);
    struct tm * t = localtime(&test);

	// Let's get the new text date
	info_lines(new_time.topbar, t);
	majMinute(new_time.minutePrecise, t);
	// Let's update the top bar
	if(strcmp(new_time.topbar, cur_time.topbar) != 0) text_layer_set_text(topbar, new_time.topbar);
	if(strcmp(new_time.minutePrecise, cur_time.minutePrecise) != 0) text_layer_set_text(minutePrecise, new_time.minutePrecise);
	
	// Let's get the new text time
	nbLineNew = fuzzy_time(new_time.line1, new_time.line2, new_time.line3 ,new_time.line4, t);
	if(nbLineNew != nbLine)
	{
		update_TextLayerPosition(nbLineNew);
		update_NBlayer(nbLineNew);

	}
	else{
		// update hour only if changed
		if(strcmp(new_time.line1, cur_time.line1) != 0) updateLayer(line1, 1);
		if(strcmp(new_time.line2, cur_time.line2) != 0) updateLayer(line2, 2);
		if(strcmp(new_time.line3, cur_time.line3) != 0) updateLayer(line3, 3);
		if(strcmp(new_time.line4, cur_time.line4) != 0) updateLayer(line4, 4);
	}
	// vibrate at o'clock from 8 to 24
	if(t->tm_min == 0 && t->tm_sec == 0 && t->tm_hour >= 8 && t->tm_hour <= 24 ) vibes_double_pulse();
	if(t->tm_min == 59 && t->tm_sec == 57 && t->tm_hour >= 7 && t->tm_hour <= 23 ) vibes_short_pulse();

	nbLine = nbLineNew;
}
void change_background() {
	if(Ccouleur == 0){
		INTColor1 = GColorBlack;
		INTColor2 = GColorWhite;
	}
	else{
		 INTColor1 = GColorWhite;
		 INTColor2 = GColorBlack;
	}
	text_layer_set_text_color(line1->layer[0], INTColor2);
	text_layer_set_text_color(line1->layer[1], INTColor2);
	text_layer_set_text_color(line2->layer[0], INTColor2);
	text_layer_set_text_color(line2->layer[1], INTColor2);
	text_layer_set_text_color(line3->layer[0], INTColor2);
	text_layer_set_text_color(line3->layer[1], INTColor2);
	text_layer_set_text_color(line4->layer[0], INTColor2);
	text_layer_set_text_color(line4->layer[1], INTColor2);
	
	text_layer_set_text_color(batterie, INTColor2);
	text_layer_set_text_color(topbar,INTColor2 );
	text_layer_set_text_color(minutePrecise, INTColor2);
	window_set_background_color(window, INTColor1);
}

void  change_minutePrecise(){
	if(CminutePrecise == 0)
		layer_set_hidden((Layer *) 	minutePrecise,true);
	else
		layer_set_hidden((Layer *) 	minutePrecise,false);
}
void  change_topbar(){
	if(Ctopbar == 0)
		layer_set_hidden((Layer *) 	topbar,true);
	else
		layer_set_hidden((Layer *) 	topbar,false);
}
void  change_batterie(){
	if(Cbatterie == 0)
		layer_set_hidden((Layer *) 	batterie,true);
	else
		layer_set_hidden((Layer *) 	batterie,false);
}

void update_battery(BatteryChargeState charge_state) {
	static char batterieStr[5];
	snprintf(batterieStr,5, "%d",charge_state.charge_percent);
	text_layer_set_text(batterie,batterieStr);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case MINUTEPRECISE_KEY:
      CminutePrecise = new_tuple->value->uint8;
	  change_minutePrecise();
      break;
    case COULEUR_KEY:
      Ccouleur = new_tuple->value->uint8;
      change_background();
      break;
    case BATTERIE_KEY:
      Cbatterie = new_tuple->value->uint8;
	  change_batterie();
      break;      
    case TOPBAR_KEY:
      Ctopbar = new_tuple->value->uint8;
	  change_topbar();
      break;
  }
}
// Called once per second
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (busy_animating_out || busy_animating_in) return;
	update_watch();
}


// Handle the start-up of the app
void handle_init(void) {

	// Init the text layers used to show the time
	line1 = malloc(sizeof(TextLine));
	line2 = malloc(sizeof(TextLine));
	line3 = malloc(sizeof(TextLine));
	line4 = malloc(sizeof(TextLine));

 	
	// Create our app's base window
	window = window_create();
 	window_stack_push(window, true);
	window_set_background_color(window, INTColor1);

	
	Tuplet initial_values[] = {
		TupletInteger(MINUTEPRECISE_KEY, 0),
		TupletInteger(COULEUR_KEY, 0),
		TupletInteger(BATTERIE_KEY, 0),
		TupletInteger(TOPBAR_KEY, 0)
    };
	
	line1->layer[0] = text_layer_create(GRect(00, line1_y, 144, 50));
	text_layer_set_text_color(line1->layer[0], INTColor2);
	text_layer_set_background_color(line1->layer[0], INTColorClear);
	text_layer_set_font(line1->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(line1->layer[0], GTextAlignmentLeft);

	line1->layer[1] = text_layer_create(GRect(144, line1_y, 144, 50));
	text_layer_set_text_color(line1->layer[1], INTColor2);
	text_layer_set_background_color(line1->layer[1], INTColorClear);
	text_layer_set_font(line1->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(line1->layer[1], GTextAlignmentLeft);

	line2->layer[0] = text_layer_create(GRect(0, line2_y, 144, 50));
	text_layer_set_text_color(line2->layer[0], INTColor2);
	text_layer_set_background_color(line2->layer[0], INTColorClear);
	text_layer_set_font(line2->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line2->layer[0], GTextAlignmentLeft);

	line2->layer[1] = text_layer_create(GRect(-144, line2_y, 144, 50));
	text_layer_set_text_color(line2->layer[1], INTColor2);
	text_layer_set_background_color(line2->layer[1], INTColorClear);
	text_layer_set_font(line2->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line2->layer[1], GTextAlignmentLeft);

	line3->layer[0] = text_layer_create(GRect(0, line3_y, 144, 50));
	text_layer_set_text_color(line3->layer[0], INTColor2);
	text_layer_set_background_color(line3->layer[0], INTColorClear);
	text_layer_set_font(line3->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line3->layer[0], GTextAlignmentLeft);
	
	
	line3->layer[1] = text_layer_create(GRect(144, line3_y, 144, 50));
	text_layer_set_text_color(line3->layer[1], INTColor2);
	text_layer_set_background_color(line3->layer[1], INTColorClear);
	text_layer_set_font(line3->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line3->layer[1], GTextAlignmentLeft);


	line4->layer[0] = text_layer_create(GRect(0, line4_y, 144, 50));
	text_layer_set_text_color(line4->layer[0], INTColor2);
	text_layer_set_background_color(line4->layer[0], INTColorClear);
	text_layer_set_font(line4->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line4->layer[0], GTextAlignmentLeft);

	line4->layer[1] = text_layer_create(GRect(144, line4_y, 144, 50));
	text_layer_set_text_color(line4->layer[1], INTColor2);
	text_layer_set_background_color(line4->layer[1], INTColorClear);
	text_layer_set_font(line4->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line4->layer[1], GTextAlignmentLeft);


	topbar = text_layer_create(GRect(14, 0, 116, 15));
	text_layer_set_text_color(topbar,INTColor2 );
	text_layer_set_background_color(topbar, INTColorClear);
	text_layer_set_font(topbar, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(topbar, GTextAlignmentCenter);

	minutePrecise = text_layer_create(GRect(130, 0, 14, 18));
	text_layer_set_text_color(minutePrecise, INTColor2);
	text_layer_set_background_color(minutePrecise, INTColorClear);
	text_layer_set_font(minutePrecise, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(minutePrecise, GTextAlignmentRight);
	
	batterie = text_layer_create(GRect(0, 0, 50, 18));
	text_layer_set_text_color(batterie, INTColor2);
	text_layer_set_background_color(batterie, INTColorClear);
	text_layer_set_font(batterie, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(batterie, GTextAlignmentLeft);
	

	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line2->layer[0]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line2->layer[1]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line3->layer[0]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line3->layer[1]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line4->layer[0]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line4->layer[1]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line1->layer[0]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line1->layer[1]));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(topbar));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(minutePrecise));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(batterie));

	const int inbound_size = 64;
 	const int outbound_size = 64;
  	app_message_open(inbound_size, outbound_size);  

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick); // la fonction handle_minute_tick s'executera auto. toute les minutes
	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
    sync_tuple_changed_callback, NULL, NULL);
 	update_battery(battery_state_service_peek());
	battery_state_service_subscribe(&update_battery);
	

}


void handle_deinit(void) {
	
	tick_timer_service_unsubscribe();	
  	app_sync_deinit(&sync);
	battery_state_service_unsubscribe();

	text_layer_destroy(line1->layer[0]);
	text_layer_destroy(line2->layer[0]);
	text_layer_destroy(line3->layer[0]);
	text_layer_destroy(line4->layer[0]);
	
	text_layer_destroy(line1->layer[1]);
	text_layer_destroy(line2->layer[1]);
	text_layer_destroy(line3->layer[1]);
	text_layer_destroy(line4->layer[1]);	
	
	free(line1);
	free(line2);
	free(line3);
	free(line4);
	
	text_layer_destroy(batterie);
	text_layer_destroy(topbar);
	text_layer_destroy(minutePrecise);
	
	battery_state_service_unsubscribe();

	window_destroy(window);
}
// The main event/run loop for our app
int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit(); 
}
