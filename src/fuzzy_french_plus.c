#include "fuzzy_french_plus.h"
#include "french_time.h"

Window * window;

static AppSync sync;
static 	uint8_t sync_buffer[64];


static int offset = 0;

static TextLayer * topbar; 
static TextLayer * minutePrecise;
static TextLayer * batterie;
static TextLine ** Txl_line; // tab of TextLine. Contains line0, line1, line2, line3.
 TheTime cur_time;
 TheTime new_time;
static int nbLineNew ;

static bool busy_animating_in = false;
static bool busy_animating_out = false;
static bool setWayAnimation;
const int line_x_layer1[4] = {144,-144,144,-144};

const int line_y[2][4] = 	{{18,56,94,200}, // 3 lines
							{7,46,80,116}}; // 4 lines

void animationInStoppedHandler(struct Animation *animation, bool finished, void *context) {
	busy_animating_in = false;
	cur_time = new_time; 	// reset cur_time
}

void animationOutStoppedHandler(struct Animation *animation, bool finished, void * context) {
	TextLayerAnim * anim = (TextLayerAnim * ) context;
	TextLayer * outside = anim->layer;
	GRect rect = layer_get_frame( (Layer *) text_layer_get_layer(outside));
	
	if ( (anim->line %2)!=0 ) 
		rect.origin.x = -144;
	else 
		rect.origin.x = 144;
	
	layer_set_frame(text_layer_get_layer(outside), rect);
	busy_animating_out = false;
	free(anim);
}

void updateLayer(TextLine * animating_line, int line) {
	TextLayerAnim * anim;
	anim = (TextLayerAnim *) malloc(sizeof(TextLayerAnim ));
	anim->line = line;
	setWayAnimation = animating_line->wayAnimation;
	TextLayer *inside, *outside;
  	GRect rect = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));

  	inside = (rect.origin.x == 0) ? animating_line->layer[0] : animating_line->layer[1];
  	outside = (inside == animating_line->layer[0]) ? animating_line->layer[1] : animating_line->layer[0];
  	anim->layer =  inside;

  	GRect in_rect = layer_get_frame(text_layer_get_layer(outside));
  	GRect out_rect = layer_get_frame(text_layer_get_layer(inside));

	if (line % 2 !=0){

		in_rect.origin.x += 144;
		out_rect.origin.x += 144;
	} else {
		in_rect.origin.x -= 144;
		out_rect.origin.x -= 144;
	}

 // animate out current layer
	 busy_animating_out = true;
  	PropertyAnimation *animate_out;
  	animate_out = property_animation_create_layer_frame(text_layer_get_layer(inside), NULL, &out_rect);
  	animation_set_duration((Animation*)animate_out, ANIMATION_DURATION);
  	animation_set_curve((Animation*)animate_out, AnimationCurveEaseOut);
  	animation_set_handlers((Animation*)animate_out, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler)animationOutStoppedHandler
  }, (void *)  anim);
  animation_schedule((Animation*)animate_out);

	text_layer_set_text(outside, new_time.lineStr[line]);
	text_layer_set_text(inside, cur_time.lineStr[line]);

	// animate in new layer
	busy_animating_in = true;
  PropertyAnimation *animate_in;
  animate_in = property_animation_create_layer_frame(text_layer_get_layer(outside), NULL, &in_rect);
  animation_set_duration((Animation*)animate_in, ANIMATION_DURATION);
  animation_set_curve((Animation*)animate_in, AnimationCurveEaseOut);
  animation_set_handlers((Animation*)animate_in, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler)animationInStoppedHandler
  }, (void *)outside);
  animation_schedule((Animation*)animate_in);


}

void update_NBlayer(int nbLayer){
	for(int i=0; i<4;i++)
		updateLayer(Txl_line[i], i);
}


void update_TextLayerPosition(int nbLineNew, int offset){
	int i;
		if(nbLineNew == 3){ // impair
			for(i = 0; i< NB_MAX_LINE; i++)
			{
				layer_set_frame((Layer *) Txl_line[i]->layer[0],GRect(0, line_y[0][i] - ( offset - (offset/3)*(i+1)), 144, 50));
				layer_set_frame((Layer *) Txl_line[i]->layer[1],GRect(line_x_layer1[i] , line_y[0][i]  - ( offset -(offset/3)*(i+1)), 144, 50));
			}
		}
		else{ //  pair (2 or 4)
			for(i = 0; i< NB_MAX_LINE; i++)
			{
				layer_set_frame((Layer *) Txl_line[i]->layer[0],GRect(0, line_y[1][i] - ( offset - (offset/4)*(i+1)), 144, 50));
				layer_set_frame((Layer *) Txl_line[i]->layer[1],GRect(line_x_layer1[i]  , line_y[1][i] - ( offset - (offset/4)*(i+1)), 144, 50));
			}
		}
}

void update_watch(struct tm * t) {
	static int nbLine ;
	

	info_lines(new_time.topbar, t);	// Let's get the new text date
	majMinute(new_time.minutePrecise, t); 

	if(strcmp(new_time.topbar, cur_time.topbar) != 0) text_layer_set_text(topbar, new_time.topbar);	// Let's update the top bar
	if(strcmp(new_time.minutePrecise, cur_time.minutePrecise) != 0) text_layer_set_text(minutePrecise, new_time.minutePrecise);
	
	nbLineNew = fuzzy_time(&new_time, t);	// Let's get the new text time

	if(nbLineNew != nbLine)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "update_watch() offset : '%d'", offset);
		update_TextLayerPosition(nbLineNew, offset);
		update_NBlayer(nbLineNew);

	}
	else{
		if(strcmp(new_time.lineStr[0], cur_time.lineStr[0]) != 0) updateLayer(Txl_line[0], 0);		// update hour only if changed
		if(strcmp(new_time.lineStr[1], cur_time.lineStr[1]) != 0) updateLayer(Txl_line[1], 1);
		if(strcmp(new_time.lineStr[2], cur_time.lineStr[2]) != 0) updateLayer(Txl_line[2], 2);
		if(strcmp(new_time.lineStr[3], cur_time.lineStr[3]) != 0) updateLayer(Txl_line[3], 3);
	}
	// vibrate at o'clock from 8 to 24
	if(t->tm_min == 0 && t->tm_sec == 0 && t->tm_hour >= 8 && t->tm_hour <= 24 ) vibes_double_pulse();
	if(t->tm_min == 59 && t->tm_sec == 57 && t->tm_hour >= 7 && t->tm_hour <= 23 ) vibes_short_pulse();

	nbLine = nbLineNew;
}

void change_background(int param) {
	int INTColor1 = GColorWhite;
	int	INTColor2 = GColorBlack;

	if(param == 0){
		INTColor1 = GColorBlack;
		INTColor2 = GColorWhite;
	}
	for(int i = 0; i < NB_MAX_LINE; i++){
		text_layer_set_text_color(Txl_line[i]->layer[0], INTColor2);
		text_layer_set_text_color(Txl_line[i]->layer[1], INTColor2);
	}
	text_layer_set_text_color(batterie, INTColor2);
	text_layer_set_text_color(topbar,INTColor2 );
	text_layer_set_text_color(minutePrecise, INTColor2);
	window_set_background_color(window, INTColor1);
}

void  change_minutePrecise(int param){
	if(param == 0)
		layer_set_hidden((Layer *) 	minutePrecise,true);
	else
		layer_set_hidden((Layer *) 	minutePrecise,false);
}
void  change_topbar(int param){
	if(param == 0)
		layer_set_hidden((Layer *) 	topbar,true);
	else
		layer_set_hidden((Layer *) 	topbar,false);
}
void  change_batterie(int param){
	if(param == 0)
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
	time_t test = time(NULL);
	struct tm * t = localtime(&test);
	int nb = fuzzy_time(&new_time, t);	// Let's get the new text time
	static int CminutePrecise, Ccouleur,Cbatterie,Ctopbar;

  switch (key) {
	case MINUTEPRECISE_KEY:
    	CminutePrecise = new_tuple->value->uint8;
    	change_minutePrecise(CminutePrecise);
    	break;
    case COULEUR_KEY:
    	Ccouleur = new_tuple->value->uint8;
    	change_background(Ccouleur);
    	break;
    case BATTERIE_KEY:
    	Cbatterie = new_tuple->value->uint8;
        change_batterie(Cbatterie);
    	break;      
    case TOPBAR_KEY:
		Ctopbar = new_tuple->value->uint8;
		change_topbar(Ctopbar);

		//APP_LOG(APP_LOG_LEVEL_DEBUG, "CminutePrecise = %d && Cbatterie =%d &&  Ctopbar = '%d'",CminutePrecise, Cbatterie,Ctopbar);

		if(CminutePrecise == 0 && Cbatterie == 0 &&  Ctopbar == 0)
			offset = 14;
		else
			offset = 0;
	  
	  	//APP_LOG(APP_LOG_LEVEL_DEBUG, "sync_tuple_changed_callback() offset : '%d'", offset);
		update_TextLayerPosition(nb, offset);
    	break;
	

  }

	


}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {// Called once per minute
	if (busy_animating_out || busy_animating_in) 
		return;
	update_watch(tick_time);
}

void handle_init(void) {// Handle the start-up of the app

	int i, j;
	int INTColor1 = GColorBlack;
	int INTColor2 = GColorWhite;

	//Init the text layers used to show the time
	Txl_line = (TextLine **) malloc(sizeof(TextLine *) * 4);
	for(i = 0; i < NB_MAX_LINE; i++){
		Txl_line[i] = (TextLine *)malloc(sizeof(TextLine));
		if(Txl_line[i] == NULL)
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Alloc Txl_line[%d] impossible", i);

	}
	
	window = window_create();	// Create our app's base window
	window_stack_push(window, true);
	window_set_background_color(window, INTColor1);
	
	Tuplet initial_values[] = {
		TupletInteger(MINUTEPRECISE_KEY, 1),
		TupletInteger(COULEUR_KEY, 0),
		TupletInteger(BATTERIE_KEY, 0),
		TupletInteger(TOPBAR_KEY, 1)
	};


	for(i =0; i< NB_MAX_LINE; i++){ // set parameters of each 2 layers of the 4 lines
		Txl_line[i]->layer[0] = text_layer_create(GRect(0, line_y[0][i], 144, 50));
		Txl_line[i]->layer[1] = text_layer_create(GRect(line_x_layer1[i], line_y[0][i], 144, 50));

		for(j = 0; j < 2; j++){
			text_layer_set_text_color(Txl_line[i]->layer[j], INTColor2);
			text_layer_set_background_color(Txl_line[i]->layer[j], GColorClear);
			text_layer_set_text_alignment(Txl_line[i]->layer[j], GTextAlignmentLeft);
			text_layer_set_font(Txl_line[i]->layer[j], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
			layer_add_child(window_get_root_layer(window), text_layer_get_layer(Txl_line[i]->layer[j]));
		}	
	}
	Txl_line[0]->wayAnimation = false; // animation right to left
	Txl_line[1]->wayAnimation = true; // animation left to right
	Txl_line[2]->wayAnimation = false;
	Txl_line[3]->wayAnimation = true;

	text_layer_set_font(Txl_line[0]->layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD)); //set the hour in bold
	text_layer_set_font(Txl_line[0]->layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

	topbar = text_layer_create(GRect(14, 0, 116, 15));
	text_layer_set_text_color(topbar,INTColor2 );
	text_layer_set_background_color(topbar, GColorClear);
	text_layer_set_font(topbar, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(topbar, GTextAlignmentCenter);

	minutePrecise = text_layer_create(GRect(130, 0, 14, 18));
	text_layer_set_text_color(minutePrecise, INTColor2);
	text_layer_set_background_color(minutePrecise, GColorClear);
	text_layer_set_font(minutePrecise, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(minutePrecise, GTextAlignmentRight);
	
	batterie = text_layer_create(GRect(0, 0, 50, 18));
	text_layer_set_text_color(batterie, INTColor2);
	text_layer_set_background_color(batterie, GColorClear);
	text_layer_set_font(batterie, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(batterie, GTextAlignmentLeft);
	

	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(topbar));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(minutePrecise));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(batterie));

	const int inbound_size = 64;
	const int outbound_size = 64;
	app_message_open(inbound_size, outbound_size); 


	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick); // la fonction handle_minute_tick s'executera auto. toute les minutes
	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),sync_tuple_changed_callback, NULL, NULL);

//	update_battery(battery_state_service_peek());
	battery_state_service_subscribe(&update_battery);
}


void handle_deinit(void) {

	tick_timer_service_unsubscribe();	
	app_sync_deinit(&sync);
	battery_state_service_unsubscribe();

	for(int i=0; i<NB_MAX_LINE; i++){ // destroy the lines
		text_layer_destroy(Txl_line[i]->layer[0]);
		text_layer_destroy(Txl_line[i]->layer[1]);
		free(Txl_line[i]);
	}
	free(Txl_line);
	text_layer_destroy(batterie);
	text_layer_destroy(topbar);
	text_layer_destroy(minutePrecise);
	layer_remove_child_layers(window_get_root_layer(window));
	battery_state_service_unsubscribe();
	window_stack_remove(window, false);
	window_destroy(window);
}
// The main event/run loop for our app
int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit(); 
}

