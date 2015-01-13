CC     = gcc
CFLAGS = -Wall -Iinclude -I/usr/local/include
OFILES = src/framework/logging.o src/framework/display.o src/framework/draw.o src/framework/actors.o src/framework/input.o src/player.o src/main.o
LDFLAGS = -Wl,-rpath,. -Llib -ltcod

linux: cgame cgame_linux

osx: cgame cgame_osx

cgame: $(OFILES)
		$(CC) $(LDFLAGS) -o cgame $^

cgame_linux:
	cp -r data bin/
	cp lib/*.so bin/

cgame_osx:
	cp -r data bin/
	cp lib/*.dylib bin/

clean:
	rm src/*.o src/framework/*.o
	rm -r bin/data
	rm bin/*.dylib
	rm bin/*.so
	rm bin/*.dll
