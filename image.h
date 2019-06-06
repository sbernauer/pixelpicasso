#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>

#if IMAGICK < 7
	#include <wand/MagickWand.h>
#else
	#include <MagickWand/MagickWand.h>
#endif

#include "progress.h"

struct img_ctx {
	MagickWand* wand;
};

struct img_pixel {
	unsigned int x;
	unsigned int y;

	union {
		struct {
			uint8_t alpha;
			uint8_t blue;
			uint8_t green;
			uint8_t red;
		} color;
		uint32_t abgr;
	};
};

struct img_frame {
	struct img_pixel* pixels;
	size_t num_pixels;

	unsigned long duration_ms;
};

struct img_animation {
	unsigned int width;
	unsigned int height;

	struct img_frame* frames;
	size_t num_frames;
};

int image_alloc(struct img_ctx** ret);
void image_free(struct img_ctx* ctx);
int image_load_animation(struct img_animation** ret, char* fname, progress_cb progress_cb);
void image_free_animation(struct img_animation* anim);

void image_shuffle_frame(struct img_frame* frame);
void image_shuffle_animation(struct img_animation* anim, progress_cb progress_cb);

#endif
