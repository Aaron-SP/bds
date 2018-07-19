# Beyond Dying Skies Makefile - CYGWIN | LINUX

# Query the freetype2 package config for the include directory
FREETYPE2_INCLUDE = $(shell pkg-config freetype2 --cflags)

# Compile binaries
BIN_GAME = bin/game
OBJ_GAME = bin/game.o
OBJ_MGL = bin/mgl.o
BIN_PCH = source/game/pch.hpp.gch
BIN_TEST = bin/tests

# Linker parameters
ifeq ($(OS),Windows_NT)
	# 64 bit
	APPLICATIONS ?= $(shell cygpath -m /usr/share/applications)
	PREFIX ?= $(shell cygpath -m /usr/local)
	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        MGL_DESTDIR ?= $(shell cygpath -m /usr/x86_64-w64-mingw32/sys-root/mingw/include/mgl)
    endif

	#64 bit
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        MGL_DESTDIR ?= $(shell cygpath -m /usr/x86_64-w64-mingw32/sys-root/mingw/include/mgl)
    else
	#32 bit
        MGL_DESTDIR ?= $(shell cygpath -m /usr/i686-w64-mingw32/sys-root/mingw/include/mgl)
    endif
	DATAPATH ?= $(DESTDIR)$(PREFIX)/share/bds
	SAVEPATH ?= $(shell cygpath -m $(HOME)/.bds-game)

	# Link library settings
	LINKER = -lgdi32 -lopengl32 -lfreetype.dll -lOpenAL32.dll -lvorbisfile.dll -lglew32.dll
	STATIC = $(LINKER) -static -lmingw32 -static-libgcc -static-libstdc++ -Wl,--as-needed
	DYNAMIC = -Wl,-Bdynamic $(LINKER) -lmingw32 -Wl,--as-needed
	BIN_MGL = bin/libmgl.dll
	LINK_MGL = bin/libmgl_dll.a
	MGL_SHARED = -fPIC -shared $(OBJ_MGL) -o $(BIN_MGL) -Wl,--out-implib,$(LINK_MGL)
else
	APPLICATIONS ?= /usr/share/applications
	PREFIX ?= /usr/local
	MGL_DESTDIR ?= /usr/include/mgl
	DATAPATH ?= $(DESTDIR)$(PREFIX)/share/bds
	SAVEPATH ?= $(HOME)/.bds-game

	# Link library settings
	LINKER = -lX11 -lGL -lfreetype -lopenal -lvorbisfile -lGLEW
	STATIC = $(LINKER) -Wl,-Bstatic -pthread -static-libgcc -static-libstdc++ -Wl,--as-needed
	DYNAMIC = -Wl,-Bdynamic $(LINKER) -pthread -Wl,--as-needed
	BIN_MGL = bin/libmgl.so
	LINK_MGL = $(BIN_MGL)
	MGL_SHARED = -fPIC -shared $(OBJ_MGL) -o $(BIN_MGL)
endif

# Compile flags
WARNFLAGS = -Wall -Wextra -pedantic -Winvalid-pch -Wno-unused-parameter 
DEBUGFLAGS = -std=c++14 $(WARNFLAGS) -O1
INLINEFLAGS = --param max-inline-insns-auto=100 --param early-inlining-insns=200
RELEASEFLAGS = -std=c++14 $(WARNFLAGS) -O3 -fomit-frame-pointer -freciprocal-math -ffast-math

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

ifndef DATALOCAL

# Enable file save redirect
ifdef DATAPATH
	CXXFLAGS += -DDATA_PATH=$(DATAPATH)
endif

# Enable file save redirect
ifdef SAVEPATH
	CXXFLAGS += -DSAVE_PATH=$(SAVEPATH)
endif
endif

# Build targets
GAME = -c source/game.cpp -o $(OBJ_GAME)
HEAD = source/game/pch.hpp
INLINE = -DMGL_INLINE source/game.cpp -o $(BIN_GAME)
MGL = -c source/mgl.cpp -o $(OBJ_MGL)
TEST = test/test.cpp -o $(BIN_TEST)

# Include directories
LIB_SOURCES = -I$(MGL_DESTDIR)/file -I$(MGL_DESTDIR)/geom -I$(MGL_DESTDIR)/math -I$(MGL_DESTDIR)/platform -I$(MGL_DESTDIR)/renderer -I$(MGL_DESTDIR)/scene -I$(MGL_DESTDIR)/sound -I$(MGL_DESTDIR)/util -Isource $(FREETYPE2_INCLUDE)
TEST_SOURCES = -Itest

# Printing colors
R=\033[0;31m
G=\033[0;32m
Y=\033[1;33m
NC=\033[0m

# Default run target
all: tests inline-dynamic
rebuild: $(BIN_PCH)
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(CXXFLAGS) $(GAME)
dynamic: $(BIN_MGL) $(BIN_GAME)
static: $(OBJ_MGL) $(OBJ_GAME)
	$(CXX) $(SYMBOLS) $(CXXFLAGS) $^ -o $(BIN_GAME) $(STATIC)
inline-dynamic:
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(CXXFLAGS) $(INLINEFLAGS) $(INLINE) $(DYNAMIC)
inline-static:
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(CXXFLAGS) $(INLINEFLAGS) $(INLINE) $(STATIC)
tests: $(BIN_TEST)
$(BIN_GAME): $(OBJ_GAME)
	$(CXX) $(SYMBOLS) $(CXXFLAGS) $^ -L. -l:$(LINK_MGL) $(DYNAMIC) -o $@
$(BIN_MGL):
	$(CXX) -fPIC $(LIB_SOURCES) $(CXXFLAGS) $(MGL)
	$(CXX) $(SYMBOLS) $(CXXFLAGS) $(MGL_SHARED)
$(BIN_PCH):
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(HEAD)
$(BIN_TEST):
	$(CXX) $(SYMBOLS) $(LIB_SOURCES) $(TEST_SOURCES) $(CXXFLAGS) $(TEST) $(DYNAMIC)
$(OBJ_GAME): $(BIN_PCH) $(BIN_TEST)
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(GAME)
$(OBJ_MGL):
	$(CXX) $(LIB_SOURCES) $(CXXFLAGS) $(MGL)

install:
	printf "$(R)Installing $(Y)Beyond Dying Skies$(R) to $(G)'$(DESTDIR)$(PREFIX)'$(R) $(NC)\n"
	install -dv $(DESTDIR)$(PREFIX)/bin
	install -dv $(DATAPATH)
	install -dv $(DESTDIR)$(APPLICATIONS)
	install -m 755 $(BIN_GAME) $(DESTDIR)$(PREFIX)/bin/bds
	cp -vr data/* $(DATAPATH)
	cp -v favicon.ico $(DATAPATH)
	chmod -R 755 $(DATAPATH)
	printf '%s\n' '[Desktop Entry]' 'Version=1.0' 'Name=Beyond Dying Skies' \
	'Comment=Beyond Dying Skies is a FOSS, FPS-RPG 3D game for GNU/Linux operating systems.' \
	'Exec=$(DESTDIR)$(PREFIX)/bin/bds' 'Icon=$(DATAPATH)/favicon.ico' \
	'Terminal=false' 'Type=Application' 'Categories=Game' > $(DESTDIR)$(APPLICATIONS)/bds.desktop
	chmod 755 $(DESTDIR)$(APPLICATIONS)/bds.desktop
uninstall:
	printf "$(R)Uninstalling $(Y)Beyond Dying Skies$(R) from $(G)'$(DESTDIR)$(PREFIX)'$(R) $(NC)\n"
	rm -i $(DESTDIR)$(PREFIX)/bin/bds
	rm -rI $(DATAPATH)
	rm -i $(DESTDIR)$(APPLICATIONS)/bds.desktop
savepath:
	install -dv $(SAVEPATH)/save
clean:
	rm -f $(BIN_GAME)
	rm -f $(OBJ_GAME)
	rm -f $(BIN_MGL) $(LINK_MGL) $(OBJ_MGL)
	rm -f $(BIN_TEST)
	rm -f $(BIN_PCH)
clear:
	rm -f save/keymap.*
	rm -f save/state.*
	rm -f save/world.*
