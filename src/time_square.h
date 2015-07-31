#pragma once
#include <pebble.h>

#define TS_LENGTH 3
#define MAX_WIDTH 144
#define MAX_HEIGHT 168

typedef struct TimeSquare {
	TextLayer * text_layer;
	PropertyAnimation * prop_anim;
	char * time_str;
	int dim;
	int ix;
	int iy;
} TimeSquare;

TimeSquare * time_square_create(int dim, int ix, int iy);
void time_square_destroy(TimeSquare *);

void time_square_add_parent(TimeSquare *, Layer *);

#ifdef PBL_PLATFORM_BASALT
void time_square_randomize_color(TimeSquare *);
#endif
void time_square_reset_color(TimeSquare *);


void time_square_set_time(TimeSquare *, char * time_str);
char * time_square_get_time(TimeSquare *, char * dest);


Layer * time_square_get_layer(TimeSquare *);
GRect time_square_get_frame(TimeSquare *);
GRect time_square_get_default_rec(TimeSquare *);
GRect time_square_gen_random_rec(TimeSquare *);

Animation * time_square_create_animation(TimeSquare *, GRect * from_frame, GRect * to_frame);
void time_square_destroy_animation(TimeSquare *);

Animation * time_square_get_animation(TimeSquare *);
