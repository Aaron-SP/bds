# Tunnels Makefile - CYGWIN | LINUX

# Query the freetype2 package config for the include directory
FREETYPE2_INCLUDE = $(shell freetype-config --cflags)

# Linker parameters
ifeq ($(OS),Windows_NT)
	MGL_PATH = C:/cygwin/usr/i686-w64-mingw32/sys-root/mingw/include/mgl
	LINKER = -lopengl32 -lgdi32 -lmingw32 -lfreetype.dll
else
	MGL_PATH = /usr/include/mgl
	LINKER = -lX11 -lGL -lfreetype
endif

# Override if MGL_DESTDIR specified
ifdef MGL_DESTDIR
	MGL_PATH = $(MGL_DESTDIR)/mgl
endif

# Compile parameters
NATIVE = -std=c++14 -Wall -O3 -march=native -fomit-frame-pointer -freciprocal-math -ffast-math
BUILD32 = -std=c++14 -Wall -O3 -m32 -static -static-libgcc -static-libstdc++
BUILD64 = -std=c++14 -Wall -O3 -m64 -static -static-libgcc -static-libstdc++
GAME = -DGLEW_STATIC $(MGL_PATH)/platform/min/glew.cpp source/game.cpp -o bin/game
TEST = -DGLEW_STATIC $(MGL_PATH)/platform/min/glew.cpp test/test.cpp -o bin/tests

# Include directories
LIB_SOURCES = -I$(MGL_PATH)/file -I$(MGL_PATH)/geom -I$(MGL_PATH)/math -I$(MGL_PATH)/platform -I$(MGL_PATH)/scene -I$(MGL_PATH)/renderer -Isource $(FREETYPE2_INCLUDE)
TEST_SOURCES = -Itest

# Default run target
build: tests
	g++ $(LIB_SOURCES) $(NATIVE) $(GAME) $(LINKER) 2> "gcc.txt"
build32: tests32
	g++ $(LIB_SOURCES) $(BUILD32) $(GAME) $(LINKER) 2> "gcc.txt"
build64: tests64
	g++ $(LIB_SOURCES) $(BUILD64) $(GAME) $(LINKER) 2> "gcc.txt"
tests:	
	g++ $(LIB_SOURCES) $(TEST_SOURCES) $(NATIVE) $(TEST) $(LINKER) 2> "gcc.txt"
tests32:	
	g++ $(LIB_SOURCES) $(TEST_SOURCES) $(BUILD32) $(TEST) $(LINKER) 2> "gcc.txt"
tests64:	
	g++ $(LIB_SOURCES) $(TEST_SOURCES) $(BUILD64) $(TEST) $(LINKER) 2> "gcc.txt"

# clean targets
clean:
	rm -f gcc.txt
	rm -f bin/game
	rm -f bin/tests
	rm -f bin/ex1