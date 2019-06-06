pixelpicasso
============

pixelpicasso is a very fast pixeldraw client with full IPv6 support written entirely in C.
It can handle both ordinary image files and animations.

# Compiling

## Dependencies

- pthread
- ImageMagick

Use ```make```.

# Usage

```
./pixelpicasso <host> [file to send] [-p <port>] [-i <0|1>] [-t <number of threads>] [-h] [-d <pen-id>]

host: IP address of pixelflut server
file to send: Image/Animation to show

-p: Server port
-i: Ignore broken broken pipe
-t: Number of threads used for flooding
-d The id of the pen to use to draw
-h: Show usage
```
Tipp: Only use one thread! (unless you can specify different pens)

## Example

```
./pixelpicasso 127.0.0.1 animation.gif
```

Searching for a fast pixelflut server? Check out [shoreline](https://github.com/TobleMiner/shoreline)
