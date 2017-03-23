#include "time_square.h"
#include <string.h>

GRect time_square_get_default_rec(TimeSquare * ts) {
	return GRect(ts->ix, ts->iy, ts->dim, ts->dim);	
}

static GFont time_square_default_font(TimeSquare * ts) {
	if (ts->dim <= 32 ) {
		return fonts_get_system_font(FONT_KEY_GOTHIC_18);
	} else if (ts->dim <= 37) {
		return fonts_get_system_font(FONT_KEY_GOTHIC_24);
	} else {
		return fonts_get_system_font(FONT_KEY_GOTHIC_28);
	}
}

GRect time_square_gen_random_rec(TimeSquare * ts) {
	if (ts) {
		return GRect(rand() % (MAX_WIDTH - ts->dim), rand() % (MAX_HEIGHT - ts->dim), ts->dim, ts->dim);
	} else {
		return GRect(0,0,0,0);
	}
	
}

static inline GColor time_square_default_color() {
#ifndef COLOR_INVERSION
	return GColorBlack;
#else
	return GColorWhite;
#endif
}

static inline GColor time_square_default_font_color() {
#ifndef COLOR_INVERSION
	return GColorWhite;
#else
	return GColorBlack;
#endif
}

#ifdef PBL_PLATFORM_BASALT
static inline GColor time_square_random_color() {
	return GColorFromRGB(rand()%128, rand()%128, rand()%128);
}

void time_square_randomize_color(TimeSquare * ts) {
	if (ts->text_layer) {
		text_layer_set_background_color(ts->text_layer, time_square_random_color());
	}
}
#endif

void time_square_reset_color(TimeSquare * ts) {
	if (ts->text_layer) {
		text_layer_set_background_color(ts->text_layer, time_square_default_color());
	}
}

TimeSquare * time_square_create(int dim, int ix, int iy) {
	TimeSquare * ts = (TimeSquare *) malloc(sizeof(TimeSquare));
	if (ts) {
		ts->dim = dim;
		ts->ix = ix;
		ts->iy = iy;

		strncpy(ts->time_str, "00", TS_LENGTH);

		ts->text_layer = text_layer_create(time_square_get_default_rec(ts));
		if (ts->text_layer) {
			text_layer_set_font(ts->text_layer, time_square_default_font(ts));
			text_layer_set_text_color(ts->text_layer, time_square_default_font_color());
			text_layer_set_text_alignment(ts->text_layer, GTextAlignmentCenter);
		}
		time_square_reset_color(ts);

		ts->prop_anim = NULL;
	}

	return ts;
}

void time_square_destroy(TimeSquare * ts) {
	if (ts) {

		if (ts->text_layer) {
			text_layer_destroy(ts->text_layer);
			ts->text_layer = 0;
		}

		free(ts);
	}
}

void time_square_add_parent(TimeSquare * ts, Layer * parent) {
	if (ts->text_layer) {
		layer_add_child(parent, time_square_get_layer(ts));
	}
}

static void time_square_update_text(TimeSquare * ts) {
	if (ts->text_layer) {
		text_layer_set_text(ts->text_layer, ts->time_str);
	}
}

void time_square_set_time(TimeSquare * ts, char * time_str) {
	if (time_str) {
		strncpy(ts->time_str, time_str, TS_LENGTH);
		time_square_update_text(ts);
	}
}

char * time_square_get_time(TimeSquare * ts, char * dest) {
	if (dest) {
		dest = strncpy(dest, ts->time_str, TS_LENGTH);
	} else {
		dest = NULL;
	}

	return dest;
}

Layer * time_square_get_layer(TimeSquare * ts) {
	Layer * layer = NULL;
	if (ts->text_layer) {
		layer = text_layer_get_layer(ts->text_layer);
	}
	return layer;
}

GRect time_square_get_frame(TimeSquare * ts) {
	Layer * layer = time_square_get_layer(ts);
	GRect frame;
	if (layer) {
		frame = layer_get_frame(layer);
	} else {
		frame = time_square_get_default_rec(ts);
	}
	return frame;
}

void time_square_destroy_animation(TimeSquare * ts) {
#ifdef PBL_PLATFORM_APLITE
	if (ts->prop_anim) {
		property_animation_destroy(ts->prop_anim);
	}
#endif
	ts->prop_anim = NULL;
}

Animation * time_square_create_animation(TimeSquare * ts, GRect * from_frame, GRect * to_frame) {
	Animation * anim = NULL;
	GRect temp;

	if (ts->text_layer) {
		Layer * layer = time_square_get_layer(ts);
		if (from_frame == NULL) {
			temp = time_square_get_frame(ts);
			from_frame = &temp;
		}

		ts->prop_anim = property_animation_create_layer_frame(layer, from_frame, to_frame);
		anim = property_animation_get_animation(ts->prop_anim);
	}

	return anim;
}

Animation * time_square_get_animation(TimeSquare * ts) {
	Animation * anim = NULL;
	if (ts->prop_anim) {
		anim = property_animation_get_animation(ts->prop_anim);
	}

	return anim;
}
