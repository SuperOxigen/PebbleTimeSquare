#include <pebble.h>
#include <string.h>

typedef struct TimeSquare {
	TextLayer * text_layer;
	char * time_str;
	int dim;
	int ix;
} TimeSquare;

static Window *s_main_window;
static TimeSquare s_hour   = { .text_layer = 0, .time_str = 0, .dim = 40, .ix = 5 },
                  s_minute = { .text_layer = 0, .time_str = 0, .dim = 35, .ix = 50 },
                  s_second = { .text_layer = 0, .time_str = 0, .dim = 30, .ix = 90 };
char zero[] = "00";

#define MAX_WIDTH 144
#define MAX_HEIGHT 168
#define TS_LENGTH 5

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

static void tap_handler(AccelAxisType axis, int32_t direction) {

}

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

	ts->text_layer = text_layer_create(GRect(ts->ix, MAX_HEIGHT / 2, ts->dim, ts->dim));
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
