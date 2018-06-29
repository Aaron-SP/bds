# Beyond Dying Skies Makefile - CYGWIN | LINUX

# Query the freetype2 package config for the include directory
FREETYPE2_INCLUDE = $(shell pkg-config freetype2 --cflags)

# Compile binaries
BIN_GAME = bin/game
OBJ_GAME = bin/game.o
OBJ_GLEW = bin/glew.o
OBJ_MGL = bin/mgl.o
BIN_PCH = source/game/pch.hpp.gch
BIN_TEST = bin/tests

# Linker parameters
ifeq ($(OS),Windows_NT)
	# 64 bit
	DESKTOP_PATH = /usr/share/applications
	DEST_PATH = /opt/bds
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

	# Link library settings
	LINKER = -lgdi32 -lopengl32 -lfreetype.dll -lOpenAL32.dll -lvorbisfile.dll
	STATIC = $(LINKER) -static -lmingw32 -static-libgcc -static-libstdc++ -Wl,--as-needed
	DYNAMIC = -Wl,-Bdynamic $(LINKER) -lmingw32 -Wl,--as-needed
	BIN_MGL = bin/libmgl.dll
	LINK_MGL = bin/libmgl_dll.a
	MGL_SHARED = -fPIC -shared $(OBJ_MGL) -o $(BIN_MGL) -Wl,--out-implib,$(LINK_MGL)
else
	DESKTOP_PATH = /usr/share/applications
	DEST_PATH = /opt/bds
	MGL_PATH = /usr/include/mgl

	# Link library settings
	LINKER = -lX11 -lGL -lfreetype -lopenal -lvorbisfile
	STATIC = $(LINKER) -Wl,-Bstatic -pthread -static-libgcc -static-libstdc++ -Wl,--as-needed
	DYNAMIC = -Wl,-Bdynamic $(LINKER) -pthread -Wl,--as-needed
	BIN_MGL = bin/libmgl.so
	LINK_MGL = $(BIN_MGL)
	MGL_SHARED = -fPIC -shared $(OBJ_MGL) -o $(BIN_MGL)
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

# Compile flags
DEBUGFLAGS = -std=c++14 -Wall -O1
INLINEFLAGS = --param max-inline-insns-auto=100 --param early-inlining-insns=200
RELEASEFLAGS = -std=c++14 -Wall -Winvalid-pch -O3 -fomit-frame-pointer -freciprocal-math -ffast-math

# Set architecture
ifeq ($(BUILD),debug)
	CXXFLAGS += $(DEBUGFLAGS)
	SYMBOLS = -g
else
ifeq ($(BUILD),arch32)
	CXXFLAGS += $(RELEASEFLAGS) -m32
	SYMBOLS = -s
else
ifeq ($(BUILD),arch64)
	CXXFLAGS += $(RELEASEFLAGS) -m64
	SYMBOLS = -s
else
	CXXFLAGS += $(RELEASEFLAGS) -march=native
	SYMBOLS = -s
endif
endif
endif

# Enable GS rendering
ifdef MGL_GS_RENDER
	CXXFLAGS += -DMGL_GS_RENDER
endif

# Enable opengl43 features
ifdef MGL_VB43
	CXXFLAGS += -DMGL_VB43
endif

# Build targets
GAME = -DGLEW_STATIC -c source/game.cpp -o $(OBJ_GAME)
GLEW = -DGLEW_STATIC -c $(MGL_PATH)/platform/min/glew.cpp -o $(OBJ_GLEW)
HEAD = -DGLEW_STATIC source/game/pch.hpp
INLINE = -DGLEW_STATIC -DMGL_INLINE $(OBJ_GLEW) source/game.cpp -o $(BIN_GAME)
MGL = -c source/mgl.cpp -o $(OBJ_MGL)
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
dynamic: $(BIN_MGL) $(BIN_GAME)
$(BIN_GAME): $(OBJ_GLEW) $(OBJ_GAME)
	$(CXX) $(SYMBOLS) $(CXXFLAGS) $^ -L. -l:$(LINK_MGL) $(DYNAMIC) -o $@ 2> "game.txt"
static: $(OBJ_MGL) $(OBJ_GLEW) $(OBJ_GAME)
	$(CXX) $(SYMBOLS) $(CXXFLAGS) $^ -o $(BIN_GAME) $(STATIC) 2> "game.txt"
game: $(BIN_PCH)
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(CXXFLAGS) $(GAME) 2> "game.txt"
inline-dynamic: $(OBJ_GLEW)
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(CXXFLAGS) $(INLINEFLAGS) $(INLINE) $(DYNAMIC) 2> "game.txt"
inline-static: $(OBJ_GLEW)
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(CXXFLAGS) $(INLINEFLAGS) $(INLINE) $(STATIC) 2> "game.txt"
$(BIN_MGL):
	$(CXX) -fPIC $(LIB_SOURCES) $(CXXFLAGS) $(MGL) 2> "mgl.txt"
	$(CXX) $(SYMBOLS) $(CXXFLAGS) $(MGL_SHARED) 2> "mgl.txt"
$(BIN_PCH):
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(HEAD) 2> "pch.txt"
$(BIN_TEST):
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(TEST_SOURCES) $(CXXFLAGS) $(TEST) $(DYNAMIC) 2> "test.txt"
$(OBJ_GAME): $(BIN_PCH) $(BIN_TEST)
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(GAME) 2> "game.o.txt"
$(OBJ_GLEW):
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(GLEW) 2> "glew.o.txt"
$(OBJ_MGL):
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(MGL) 2> "mgl.o.txt"

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
	rm -f $(BIN_GAME)
	rm -f $(OBJ_GAME)
	rm -f $(OBJ_GLEW)
	rm -f $(BIN_MGL) $(LINK_MGL) $(OBJ_MGL)
	rm -f $(BIN_TEST)
	rm -f $(BIN_PCH)
clear:
	rm -f save/keymap.*
	rm -f save/state.*
	rm -f save/world.*
