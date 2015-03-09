CC     = gcc
CFLAGS = -Wall -Iinclude
OFILES = src/framework/numbers.o src/framework/draw.o src/framework/logging.o src/framework/display.o src/framework/actors.o src/framework/input.o src/level.o src/graphics.o src/items.o src/lights.o src/player.o src/enemies.o src/combat.o src/intro.o src/ui.o src/main.o
LDFLAGS = -Wl,-rpath,. -L. -ltcod
DEBUGLDFLAGS = -Wl,-rpath,. -L. -ltcod_debug

all: cgame

debug: cgame-debug

cgame: $(OFILES)
		$(CC) $(LDFLAGS) -o torch $^

cgame-debug: $(OFILES)
		$(CC) $(DEBUGLDFLAGS) -O0 -g -o torch $^

clean:
	rm torch src/*.o src/framework/*.o
