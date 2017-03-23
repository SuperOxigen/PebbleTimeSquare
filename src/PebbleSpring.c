#include "time_square.h"
#include <pebble.h>
#include <string.h>
// #include <pthread.h>


static Window *s_main_window;
static TimeSquare * s_hour, * s_minute, * s_second;

char zero[] = "00";



// **************** TIME HANDLING ***************************

static void update_time(struct tm * tick_time) {
	if (s_hour && s_minute && s_second) {
		char buffer[TS_LENGTH];

		if (clock_is_24h_style()) {
			strftime(buffer, TS_LENGTH, "%H", tick_time);
		} else {
			strftime(buffer, TS_LENGTH, "%l", tick_time);
		}
		time_square_set_time(s_hour, buffer);

		strftime(buffer, TS_LENGTH, "%M", tick_time);
		time_square_set_time(s_minute, buffer);		

		strftime(buffer, TS_LENGTH, "%S", tick_time);
		time_square_set_time(s_second, buffer);
	}
}

static void tick_handler(struct tm * tick_time, TimeUnits units_changed) {
	update_time(tick_time);
}

// ****************** ANIMATION HANDLING ***********************

#define MAX_CYCLE 5
#define CYCLE_TIME 2000

static void trigger_animation();

inline int calc_delay() {
	return CYCLE_TIME / MAX_CYCLE;
}

void animation_stopped(Animation * animation, bool finished, void * data) {
	time_square_destroy_animation(s_hour);
	time_square_destroy_animation(s_minute);
	time_square_destroy_animation(s_second);

	trigger_animation();
}

void animation_started(Animation * animation, void *data) {

}

static GRect get_to_frame(TimeSquare * ts, int cur_stage) {
	GRect to_frame;
	if (cur_stage == MAX_CYCLE) {
		to_frame = time_square_get_default_rec(ts);
	} else {
		to_frame = time_square_gen_random_rec(ts);
	}
	return to_frame;
}

static Animation * create_animation(TimeSquare * ts, int cur_stage) {
	GRect to_frame = get_to_frame(ts, cur_stage);

#ifndef PBL_PLATFORM_APLITE
	time_square_randomize_color(ts);
#endif

	Animation * anim = time_square_create_animation(ts, NULL, &to_frame);

	animation_set_duration(anim, calc_delay()); // Half second

	return anim;
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

	Animation * h_amin = create_animation(s_hour, cur_stage);
	Animation * m_amin = create_animation(s_minute, cur_stage);
	Animation * s_amin = create_animation(s_second, cur_stage);

	AnimationHandlers a_handler = { 
		.started = (AnimationStartedHandler) animation_started,
		.stopped = (AnimationStoppedHandler) animation_stopped
	};

#ifdef PBL_PLATFORM_APLITE
	animation_set_duration(h_amin, calc_delay() + 5);
	animation_set_handlers(h_amin, a_handler, NULL);
	animation_schedule(h_amin);
	animation_schedule(m_amin);
	animation_schedule(s_amin);
#else
	Animation * anim;
	anim = animation_spawn_create(
		h_amin,
		m_amin,
		s_amin,
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


static void main_window_load(Window *window) {
	time_square_add_parent(s_hour, window_get_root_layer(window));
	time_square_add_parent(s_minute, window_get_root_layer(window));
	time_square_add_parent(s_second, window_get_root_layer(window));

	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	accel_tap_service_subscribe(tap_handler);
}

static void main_window_unload(Window *window) {

}

static void init() {
	s_hour   = time_square_create(40, 10, (MAX_HEIGHT - 40) / 2);
	s_minute = time_square_create(35, 60, (MAX_HEIGHT - 35) / 2);
	s_second = time_square_create(28, 105, (MAX_HEIGHT - 28) / 2);

	s_main_window = window_create();

#ifndef COLOR_INVERSION
	window_set_background_color(s_main_window, GColorWhite);
#else
	window_set_background_color(s_main_window, GColorBlack);
#endif

	WindowHandlers main_handler = { .load   = main_window_load,
	                                .unload = main_window_unload };
	window_set_window_handlers(s_main_window, main_handler);
	window_stack_push(s_main_window, true);
}

static void deinit() {
	window_destroy(s_main_window);
	time_square_destroy(s_hour);
	time_square_destroy(s_minute);
	time_square_destroy(s_second);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
