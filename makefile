CC     = gcc
CFLAGS = -Wall -Iinclude
OFILES = src/framework/draw.o src/framework/logging.o src/framework/display.o src/framework/actors.o src/framework/input.o src/level.o src/graphics.o src/player.o src/main.o
LDFLAGS = -Wl,-rpath,. -L. -ltcod

all: cgame

cgame: $(OFILES)
		$(CC) $(LDFLAGS) -o cgame $^

clean:
	rm src/*.o src/framework/*.o
