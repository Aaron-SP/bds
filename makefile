# Beyond Dying Skies Makefile - CYGWIN | LINUX

# Query the freetype2 package config for the include directory
FREETYPE2_INCLUDE = $(shell pkg-config freetype2 --cflags)

# Linker parameters
ifeq ($(OS),Windows_NT)
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

	LINKER = -lopengl32 -lgdi32 -lmingw32 -lfreetype.dll -lOpenAL32.dll -lvorbisfile.dll
	STATIC = -static -static-libgcc -static-libstdc++ 
else
	DEST_PATH = /opt/bds
	MGL_PATH = /usr/include/mgl
	LINKER = -lX11 -lGL -lfreetype -pthread -lopenal -lvorbisfile
	STATIC = -static-libgcc -static-libstdc++
endif

# Override if MGL_DESTDIR specified
ifdef MGL_DESTDIR
	MGL_PATH = $(MGL_DESTDIR)/mgl
endif

# Override if DESTDIR specified
ifdef DESTDIR
	DEST_PATH = $(DESTDIR)/bds
endif

# Enable GS rendering
ifdef MGL_GS_RENDER
	MGL_RENDER = -DUSE_GS_RENDER
endif

# Enable opengl43 features
ifdef MGL_VB43
	MGL_VB43 = -DMGL_VB43
endif

# Compile parameters
CPP = -s -std=c++14 -Wall -O3 -fomit-frame-pointer -freciprocal-math -ffast-math $(STATIC) $(MGL_RENDER) $(MGL_VB43)
DEBUG = -g -std=c++14 -Wall -O1 $(STATIC) $(MGL_RENDER) $(MGL_VB43)
NATIVE =  $(CPP) -march=native
BUILD32 = $(CPP) -m32
BUILD64 = $(CPP) -m64
EXTRA = -DGLEW_STATIC $(MGL_PATH)/platform/min/glew.cpp
GAME =  $(EXTRA) source/game.cpp -o bin/game
TEST =  $(EXTRA) test/test.cpp -o bin/tests

# Include directories
LIB_SOURCES = -I$(MGL_PATH)/file -I$(MGL_PATH)/geom -I$(MGL_PATH)/math -I$(MGL_PATH)/platform -I$(MGL_PATH)/renderer -I$(MGL_PATH)/scene -I$(MGL_PATH)/sound -Isource $(FREETYPE2_INCLUDE)
TEST_SOURCES = -Itest

# Printing colors
R=\033[0;31m
G=\033[0;32m
Y=\033[1;33m
NC=\033[0m

# Default run target
build: tests
	g++ $(LIB_SOURCES) $(NATIVE)  $(GAME)  $(LINKER) 2> "game.txt"
build32: tests32
	g++ $(LIB_SOURCES) $(BUILD32) $(GAME)  $(LINKER) 2> "game.txt"
build64: tests64
	g++ $(LIB_SOURCES) $(BUILD64) $(GAME)  $(LINKER) 2> "game.txt"
debug:
	g++ $(LIB_SOURCES) $(DEBUG) $(GAME) $(LINKER) 2> "game.txt"
tests:	
	g++ $(LIB_SOURCES) $(TEST_SOURCES)  $(NATIVE) $(TEST) $(LINKER) 2> "test.txt"
tests32:	
	g++ $(LIB_SOURCES) $(TEST_SOURCES) $(BUILD32) $(TEST) $(LINKER) 2> "test.txt"
tests64:	
	g++ $(LIB_SOURCES) $(TEST_SOURCES) $(BUILD64) $(TEST) $(LINKER) 2> "test.txt"
install: build
	printf "$(R)Installing $(Y)Beyond Dying Skies$(R) to $(G)\'$(DEST_PATH)\'$(R) $(NC)\n"
	mkdir -p $(DEST_PATH)/bin
	cp -v bin/game $(DEST_PATH)/bin
	cp -vr data $(DEST_PATH)
	printf '%s\n' '#!/bin/bash' 'cd $(DEST_PATH)' 'bin/game' > $(DEST_PATH)/bds.game
	chmod -R 755 $(DEST_PATH)
	ln -fs $(DEST_PATH)/bds.game /usr/bin/bds.game
uninstall:
	printf "$(R)Uninstalling $(Y)Beyond Dying Skies$(R) from $(G)\'$(DEST_PATH)\'$(R) $(NC)\n"
	rm -rI $(DEST_PATH)
	rm -i /usr/bin/bds.game

# clean targets
clean:
	rm -f *.txt
	rm -f bin/*
clear:
	rm -f bin/state
	rm -f bin/world.bmesh
