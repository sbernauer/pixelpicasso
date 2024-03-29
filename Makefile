CC=gcc
CCFLAGS=-Ofast -Wall -march=native
RM=rm -f

MAGICK_VERSION=$(shell pkg-config --modversion ImageMagick | grep -E -o '^[0-9]+')

all: clean pixelpicasso

pixelpicasso:
	$(CC) $(CCFLAGS) -DIMAGICK=$(MAGICK_VERSION) image.c network.c main.c progress.c -lpthread `pkg-config --cflags --libs MagickWand` -o pixelpicasso

clean:
	$(RM) pixelpicasso
