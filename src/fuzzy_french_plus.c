/*
	Fuzzy French +
	Inspired by Fuzzy Time and Fuzzy Time +
	With Date, 24H display and Week #
w */
// test github
#include <pebble.h>
	
#include "french_time.h"
#include <stdlib.h>
#define ANIMATION_DURATION 800
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

int INTColorClear = GColorClear;
int INTColorBlack = GColorWhite;
int INTColorWhite = GColorBlack;

TextLayer * topbar; 
TextLayer * minutePrecise;
TextLine * line1;
TextLine * line2;
TextLine * line3;
TextLine * line4;

static TheTime cur_time;
static TheTime new_time;

static bool busy_animating_in = false;
static bool busy_animating_out = false;

const int line1_y = 6; 
const int line2_y = 46;
const int line3_y = 80;
const int line4_y = 116;

const int line1_y3 = 6; 
const int line2_y3 = 46;
const int line3_y3 = 80;
const int line4_y3 = 116;


void animationInStoppedHandler(struct Animation *animation, bool finished, void *context) {
	busy_animating_in = false;
	cur_time = new_time; 	// reset cur_time
}

void animationOutStoppedHandler(struct Animation *animation, bool finished, void *context) {
	// reset out layer to x=144
	TextLayer * outside = (TextLayer *)context;
	GRect rect = layer_get_frame( (Layer *) outside);
	if (rect.origin.y == line2_y) rect.origin.x = -144;
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

	if (line == 2) {
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
	fuzzy_time(new_time.line1, new_time.line2, new_time.line3 ,new_time.line4, t);
	// update hour only if changed
	if(strcmp(new_time.line1, cur_time.line1) != 0) updateLayer(line1, 1);
	// update min1 only isf changed
	if(strcmp(new_time.line2, cur_time.line2) != 0) updateLayer(line2, 2);
	// update min2 only if changed happens on
	if(strcmp(new_time.line3, cur_time.line3) != 0) updateLayer(line3, 3);
	if(strcmp(new_time.line4, cur_time.line4) != 0) updateLayer(line4, 4);
	// vibrate at o'clock from 8 to 24
	if(t->tm_min == 0 && t->tm_sec == 0 && t->tm_hour >= 8 && t->tm_hour <= 24 ) vibes_double_pulse();
	if(t->tm_min == 59 && t->tm_sec == 57 && t->tm_hour >= 7 && t->tm_hour <= 23 ) vibes_short_pulse();
}

// Called once per second
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	if (busy_animating_out || busy_animating_in) return;
	update_watch();
}

// Handle the start-up of the app
void handle_init(void) {

	// Init the text layers used to show the time
	line1 = malloc(sizeof(TextLine ));
	line2 = malloc(sizeof(TextLine ));
	line3 = malloc(sizeof(TextLine ));
	line4 = malloc(sizeof(TextLine ));

	// Create our app's base window
	window = window_create();
 	window_stack_push(window, true);
	window_set_background_color(window, INTColorBlack);

	line1->layer[0] = text_layer_create(GRect(0, line1_y, 144, 50));
	text_layer_set_text_color(line1->layer[0], INTColorWhite);
	text_layer_set_background_color(line1->layer[0], INTColorClear);
	text_layer_set_font(line1->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(line1->layer[0], GTextAlignmentLeft);

	line1->layer[1] = text_layer_create(GRect(144, line1_y, 144, 50));
	text_layer_set_text_color(line1->layer[1], INTColorWhite);
	text_layer_set_background_color(line1->layer[1], 3);
	text_layer_set_font(line1->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(line1->layer[1], GTextAlignmentLeft);

	line2->layer[0] = text_layer_create(GRect(0, line2_y, 144, 50));
	text_layer_set_text_color(line2->layer[0], INTColorWhite);
	text_layer_set_background_color(line2->layer[0], INTColorClear);
	text_layer_set_font(line2->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line2->layer[0], GTextAlignmentLeft);

	line2->layer[1] = text_layer_create(GRect(-144, line2_y, 144, 50));
	text_layer_set_text_color(line2->layer[1], INTColorWhite);
	text_layer_set_background_color(line2->layer[1], INTColorClear);
	text_layer_set_font(line2->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line2->layer[1], GTextAlignmentLeft);

	line3->layer[0] = text_layer_create(GRect(0, line3_y, 144, 50));
	text_layer_set_text_color(line3->layer[0], INTColorWhite);
	text_layer_set_background_color(line3->layer[0], INTColorClear);
	text_layer_set_font(line3->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line3->layer[0], GTextAlignmentLeft);
	
	
	line3->layer[1] = text_layer_create(GRect(144, line3_y, 144, 50));
	text_layer_set_text_color(line3->layer[1], INTColorWhite);
	text_layer_set_background_color(line3->layer[1], INTColorClear);
	text_layer_set_font(line3->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line3->layer[1], GTextAlignmentLeft);


	line4->layer[0] = text_layer_create(GRect(0, line4_y, 144, 50));
	text_layer_set_text_color(line4->layer[0], INTColorWhite);
	text_layer_set_background_color(line4->layer[0], INTColorClear);
	text_layer_set_font(line4->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line4->layer[0], GTextAlignmentLeft);

	line4->layer[1] = text_layer_create(GRect(144, line4_y, 144, 50));
	text_layer_set_text_color(line4->layer[1], INTColorWhite);
	text_layer_set_background_color(line4->layer[1], INTColorClear);
	text_layer_set_font(line4->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(line4->layer[1], GTextAlignmentLeft);


	topbar = text_layer_create(GRect(14, 0, 116, 15));
	text_layer_set_text_color(topbar,INTColorWhite );
	text_layer_set_background_color(topbar, INTColorClear);
	text_layer_set_font(topbar, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(topbar, GTextAlignmentCenter);

	minutePrecise = text_layer_create(GRect(130, 0, 14, 18));
	text_layer_set_text_color(minutePrecise, INTColorWhite);
	text_layer_set_background_color(minutePrecise, INTColorClear);
	text_layer_set_font(minutePrecise, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(minutePrecise, GTextAlignmentRight);
	
	//update_watch();
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick); // la fonction handle_minute_tick s'executera auto. toute les minutes

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

	
}

void handle_deinit(void) {
	
	tick_timer_service_unsubscribe();	

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
	
	text_layer_destroy(topbar);
	text_layer_destroy(minutePrecise);

	window_destroy(window);
}
// The main event/run loop for our app
int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit(); 
}
