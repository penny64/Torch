CC     = clang
CFLAGS = -Wall -g -Iinclude -I/usr/local/include/
OFILES = src/framework/numbers.o src/framework/draw.o src/framework/logging.o src/framework/display.o src/framework/actors.o src/framework/input.o src/level.o src/rooms.o src/graphics.o src/items.o src/components.o src/systems.o src/entities.o src/spells.o src/rects.o src/ai.o src/effects.o src/particles.o src/lights.o src/player.o src/enemies.o src/combat.o src/intro.o src/ui.o src/main.o
LDFLAGS = -Wl,-rpath,. -Llib -L. -ltcod -lm
DEBUGLDFLAGS = -Wl,-rpath,. -L. -ltcod_debug -lm
BINNAME = hollowearth

all: cgame
debug: cgame-debug

cgame: $(OFILES)
		$(CC) $(LDFLAGS) -o $(BINNAME) $^

cgame-debug: $(OFILES)
		$(CC) $(DEBUGLDFLAGS) -O0 -o $(BINNAME) $^

clean:
	rm $(BINNAME) src/*.o src/framework/*.o
