# Beyond Dying Skies Makefile - CYGWIN | LINUX

# Query the freetype2 package config for the include directory
FREETYPE2_INCLUDE = $(shell pkg-config freetype2 --cflags)

# Linker parameters
ifeq ($(OS),Windows_NT)
	DESKTOP_PATH = /usr/share/applications
	DEST_PATH = /opt/bds

	# 64 bit
	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        MGL_PATH = C:/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/include/mgl
    endif

	#64 bit
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        MGL_PATH = C:/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/include/mgl
    else
	#32 bit
        MGL_PATH = C:/cygwin/usr/i686-w64-mingw32/sys-root/mingw/include/mgl
    endif

	LINKER = -lopengl32 -lgdi32 -lmingw32 -lfreetype.dll -lOpenAL32.dll -lvorbisfile.dll -static -static-libgcc -static-libstdc++
else
	DESKTOP_PATH = /usr/share/applications
	DEST_PATH = /opt/bds
	MGL_PATH = /usr/include/mgl
	LINKER = -lX11 -lGL -lfreetype -pthread -lopenal -lvorbisfile -static-libgcc -static-libstdc++
endif

# Override if MGL_DESTDIR specified
ifdef MGL_DESTDIR
	MGL_PATH = $(MGL_DESTDIR)/mgl
endif

# Override if MGL_DESTDIR specified
ifdef DESKTOPDIR
	DESKTOP_PATH = $(DESKTOPDIR)
endif

# Override if DESTDIR specified
ifdef DESTDIR
	DEST_PATH = $(DESTDIR)/bds
endif

# Enable GS rendering
ifdef MGL_GS_RENDER
	MGL_RENDER = -DMGL_GS_RENDER
endif

# Enable opengl43 features
ifdef MGL_VB43
	MGL_RENDER_VB = -DMGL_VB43
endif

CXXFLAGS = -s -std=c++14 -Wall -Winvalid-pch -O3 -fomit-frame-pointer -freciprocal-math -ffast-math
DEBUGFLAGS = -g -std=c++14 -Wall -O1

# Set architecture
ifeq ($(BUILD),debug)
	FLAGS = $(DEBUGFLAGS)
else 
ifeq ($(BUILD),arch32)
	FLAGS = $(CXXFLAGS) -m32
else
ifeq ($(BUILD),arch64)
	FLAGS = $(CXXFLAGS) -m64
else
	FLAGS = $(CXXFLAGS) -march=native
endif
endif
endif

# Compile binaries
BIN_EXEC = bin/game
BIN_GAME = bin/game.o
BIN_GLEW = bin/glew.o
BIN_MGL = bin/mgl.o
BIN_PCH = source/game/pch.h.gch
BIN_TEST = bin/tests

# Compile parameters
CPP = -H $(MGL_RENDER) $(MGL_RENDER_VB)
GAME = -DGLEW_STATIC -c source/game.cpp -o $(BIN_GAME)
GLEW = -DGLEW_STATIC -c $(MGL_PATH)/platform/min/glew.cpp -o $(BIN_GLEW)
HEAD = -DGLEW_STATIC source/game/pch.h
INLINE = -DGLEW_STATIC -DMGL_INLINE $(BIN_GLEW) source/game.cpp -o $(BIN_EXEC)
MGL = -DGLEW_STATIC -c source/mgl.cpp -o $(BIN_MGL)
TEST = test/test.cpp -o $(BIN_TEST)

# Include directories
LIB_SOURCES = -I$(MGL_PATH)/file -I$(MGL_PATH)/geom -I$(MGL_PATH)/math -I$(MGL_PATH)/platform -I$(MGL_PATH)/renderer -I$(MGL_PATH)/scene -I$(MGL_PATH)/sound -Isource $(FREETYPE2_INCLUDE)
TEST_SOURCES = -Itest

# Printing colors
R=\033[0;31m
G=\033[0;32m
Y=\033[1;33m
NC=\033[0m

# Default run target
$(BIN_EXEC): $(BIN_GLEW) $(BIN_MGL) $(BIN_GAME)
	g++ $(FLAGS) $^ -o $@ $(LINKER) 2> "exec.txt"
$(BIN_GAME): $(BIN_PCH) $(BIN_TEST)
	g++ $(LIB_SOURCES) $(FLAGS) $(CPP) $(GAME) 2> "game.txt"
$(BIN_GLEW):
	g++ $(LIB_SOURCES) $(FLAGS) $(CPP) $(GLEW) 2> "glew.txt"
$(BIN_MGL):
	g++ $(LIB_SOURCES) $(FLAGS) $(CPP) $(MGL) 2> "mgl.txt"
$(BIN_PCH):
	g++ $(LIB_SOURCES) $(FLAGS) $(HEAD) 2> "pch.txt"
$(BIN_TEST):
	g++ $(LIB_SOURCES) $(TEST_SOURCES) $(FLAGS) $(CPP) $(TEST) $(LINKER) 2> "test.txt"
game: $(BIN_PCH)
	g++ $(LIB_SOURCES) $(FLAGS) $(CPP) $(GAME) 2> "game.txt"
inline: $(BIN_GLEW)
	g++ $(LIB_SOURCES) $(FLAGS) $(CPP) $(INLINE) $(LINKER) 2> "game.txt"
install:
	printf "$(R)Installing $(Y)Beyond Dying Skies$(R) to $(G)'$(DEST_PATH)'$(R) $(NC)\n"
	mkdir -p $(DEST_PATH)/bin
	cp -v bin/game $(DEST_PATH)/bin
	cp -vr data $(DEST_PATH)
	cp -v favicon.ico $(DEST_PATH)
	printf '%s\n' '#!/bin/bash' 'cd $(DEST_PATH)' 'bin/game "$$@"' > $(DEST_PATH)/bds.game
	chmod -R 755 $(DEST_PATH)
	mkdir -p $(DEST_PATH)/save
	chmod -R 777 $(DEST_PATH)/save
	ln -fs $(DEST_PATH)/bds.game /usr/bin/bds.game
	@if [ -d $(DESKTOP_PATH) ] && [ ! -z "$(wildcard $(DESKTOP_PATH)/*.desktop)" ]; then\
		cp -v bds.desktop $(DESKTOP_PATH)/bds.desktop;\
	fi
uninstall:
	printf "$(R)Uninstalling $(Y)Beyond Dying Skies$(R) from $(G)'$(DEST_PATH)'$(R) $(NC)\n"
	rm -rI $(DEST_PATH)
	rm -i /usr/bin/bds.game

# clean targets
clean:
	rm -f *.txt
	rm -f $(BIN_EXEC)
	rm -f $(BIN_GAME)
	rm -f $(BIN_GLEW)
	rm -f $(BIN_MGL)
	rm -f $(BIN_TEST)
	rm -f $(BIN_PCH)
clear:
	rm -f save/keymap.*
	rm -f save/state.*
	rm -f save/world.*
