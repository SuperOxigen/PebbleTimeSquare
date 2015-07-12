#include <pebble.h>
#include <string.h>
// #include <pthread.h>

typedef struct TimeSquare {
	TextLayer * text_layer;
	PropertyAnimation * prop_anim;
	char * time_str;
	int dim;
	int ix;
} TimeSquare;

static Window *s_main_window;
static TimeSquare s_hour   = { .text_layer = 0, .prop_anim = 0, .time_str = 0, .dim = 40, .ix = 5 },
                  s_minute = { .text_layer = 0, .prop_anim = 0, .time_str = 0, .dim = 35, .ix = 50 },
                  s_second = { .text_layer = 0, .prop_anim = 0, .time_str = 0, .dim = 30, .ix = 90 };
char zero[] = "00";

#define MAX_WIDTH 144
#define MAX_HEIGHT 168
#define TS_LENGTH 5

static GRect time_square_get_default_rec(TimeSquare * ts) {
	return GRect(ts->ix, (MAX_HEIGHT - ts->dim) / 2, ts->dim, ts->dim);	
}

// **************** TIME HANDLING ***************************

static void update_time(struct tm * tick_time) {
	if (!s_hour.text_layer || !s_minute.text_layer || !s_second.text_layer) return;

	if (clock_is_24h_style()) {
		strftime(s_hour.time_str, TS_LENGTH, "%H", tick_time);
	} else {
		strftime(s_hour.time_str, TS_LENGTH, "%I", tick_time);
	}
	text_layer_set_text(s_hour.text_layer, s_hour.time_str);

	strftime(s_minute.time_str, TS_LENGTH, "%M", tick_time);
	text_layer_set_text(s_minute.text_layer, s_minute.time_str);

	strftime(s_second.time_str, TS_LENGTH, "%S", tick_time);
	text_layer_set_text(s_second.text_layer, s_second.time_str);
}

static void tick_handler(struct tm * tick_time, TimeUnits units_changed) {
	update_time(tick_time);
}

// ****************** ANIMATION HANDLING ***********************

#define MAX_CYCLE 5

static void trigger_animation();

inline int calc_delay() {
	return 2000 / MAX_CYCLE;
}

void animation_stopped(Animation * animation, bool finished, void * data) {
#ifdef PBL_PLATFORM_APLITE
	// Automatically GB with V3
	property_animation_destroy(s_hour.prop_anim);
	property_animation_destroy(s_minute.prop_anim);
	property_animation_destroy(s_second.prop_anim);
#endif
	s_hour.prop_anim = 0;
	s_minute.prop_anim = 0;
	s_second.prop_anim = 0;

	trigger_animation();
}

void animation_started(Animation * animation, void *data) {

}

static void time_square_set_animation(TimeSquare * ts, int cur_stage) {
	if (ts->prop_anim) return;

	Layer * layer = text_layer_get_layer(ts->text_layer);	

	GRect from_frame = layer_get_frame(layer);
	GRect to_frame;
	if (cur_stage == MAX_CYCLE) {
		to_frame = time_square_get_default_rec(ts);
	} else {
		to_frame = GRect(rand() % (MAX_WIDTH - ts->dim), rand() % (MAX_HEIGHT - ts->dim), ts->dim, ts->dim);
	}

	ts->prop_anim = property_animation_create_layer_frame(layer, &from_frame, &to_frame);

	Animation * anim = property_animation_get_animation(ts->prop_anim);


	animation_set_duration(anim, calc_delay()); // Half second
}

static int stage = 0;  // DO NOT DIRECTLY REFERENCE
// static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int get_stage() {
	// pthread_mutex_lock(&mutex);
	int temp = stage;
	// pthread_mutex_unlock(&mutex);
	return temp;
}

int inc_stage() {
	// pthread_mutex_lock(&mutex);
	stage = (stage + 1) % (MAX_CYCLE + 1);
	int temp = stage;
	// pthread_mutex_unlock(&mutex);
	return temp;
}

int clear_stage() {
	// pthread_mutex_lock(&mutex);
	int temp = stage = 0;
	// pthread_mutex_unlock(&mutex);
	return temp;	
}

static void trigger_animation() {
	int cur_stage = inc_stage();
	if (!cur_stage) return;

	time_square_set_animation(&s_hour, cur_stage);
	time_square_set_animation(&s_minute, cur_stage);
	time_square_set_animation(&s_second, cur_stage);

	AnimationHandlers a_handler = { 
		.started = (AnimationStartedHandler) animation_started,
		.stopped = (AnimationStoppedHandler) animation_stopped
	};

#ifdef PBL_PLATFORM_APLITE
	Animation * h_amin, * m_amin, * s_amin;
	h_amin = property_animation_get_animation(s_hour.prop_anim);
	m_amin = property_animation_get_animation(s_minute.prop_anim);
	s_amin = property_animation_get_animation(s_second.prop_anim);
	animation_set_duration(h_amin, calc_delay() + 5);
	animation_set_handlers(h_amin, a_handler, NULL);
	animation_schedule(h_amin);
	animation_schedule(m_amin);
	animation_schedule(s_amin);
#else
	Animation * anim;
	anim = animation_spawn_create(
		property_animation_get_animation(s_hour.prop_anim),
		property_animation_get_animation(s_minute.prop_anim),
		property_animation_get_animation(s_second.prop_anim),
		NULL // Signifies end of animations
	);
	animation_set_handlers(anim, a_handler, NULL);
	animation_set_duration(anim, calc_delay()); // Half second
	animation_schedule(anim);
#endif
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	static int tap_counter = 0;

	if (get_stage()) {
		tap_counter++;
		if (tap_counter > 5) {
			clear_stage();
		}
		return;
	}

	trigger_animation();
}

// ****************** LAYER DRAWYING ***************************

static void destroy_time_square(TimeSquare * ts) {
	if (ts->text_layer) {
		text_layer_destroy(ts->text_layer);
		ts->text_layer = 0;
	}
	if (ts->time_str) {
		free(ts->time_str);
		ts->time_str = 0;
	}
}

static void add_time_square(Window * window, TimeSquare * ts) {
	if (ts->text_layer || ts->time_str) {
		destroy_time_square(ts);
	}

	ts->text_layer = text_layer_create(time_square_get_default_rec(ts));
	text_layer_set_background_color(ts->text_layer, GColorBlack);
	text_layer_set_text_color(ts->text_layer, GColorWhite);
	ts->time_str = malloc(TS_LENGTH);
	strcpy(ts->time_str, zero);
	text_layer_set_text(ts->text_layer, ts->time_str);
	text_layer_set_font(ts->text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	text_layer_set_text_alignment(ts->text_layer, GTextAlignmentCenter);

	layer_add_child(window_get_root_layer(window), text_layer_get_layer(ts->text_layer));
}

static void main_window_load(Window *window) {
	add_time_square(window, &s_hour);
	add_time_square(window, &s_minute);
	add_time_square(window, &s_second);

	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	accel_tap_service_subscribe(tap_handler);
}

static void main_window_unload(Window *window) {
	destroy_time_square(&s_hour);
	destroy_time_square(&s_minute);
	destroy_time_square(&s_second);
}

static void init() {
	s_main_window = window_create();

	WindowHandlers main_handler = { .load   = main_window_load,
	                                .unload = main_window_unload };
	window_set_window_handlers(s_main_window, main_handler);

	window_stack_push(s_main_window, true);
}

static void deinit() {
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
