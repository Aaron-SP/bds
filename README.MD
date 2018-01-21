# Fractex
## Repository for the Minimal Graphics Library Fractal Explorer

Verify a bin directory exists before compiling with the makefile. 
- 'mkdir -p bin'

### The GNU makefile contains various build targets.
- 'make' - builds game and tests targeting 'native'
- 'make build' - builds game and targeting 'native'
- 'make build32' - builds game and targeting 'x86'
- 'make build64' - builds game and targeting 'x86-64'
- 'make tests' - builds only tests targeting 'native'
- 'make clean' - cleans up all generated output files

These build targets have been tested for compilation on Arch Linux x64 and Windows 7 x86/x86-x64 platforms.

### Dependencies for each platform

Additional dependencies that are required and not distributed in this package include:
- 'mgl' - Minimal Graphics Library (https://github.com/Aaron-SP/mgl)
- 'g++-5.4' - all platforms
- 'freetype' - all platforms
- 'opengl3.3+' - all platforms
- 'mingw32'/'mingw64' - for win32 only
- 'X11' - for linux only

### Optional build options

The $(MGL_DESTDIR) bash variable will override the GNU makefile with the path looking for MGL installed on the system.
The default path is 'C:/cygwin/usr/i686-w64-mingw32/sys-root/mingw/include' for CYGWIN x86 or '/usr/include' for Linux systems.

### Compile Flags

An alternative rendering mode can be enabled by exporting a variable to bash before compiling with the makefile.
This mode benefits computers with slow CPU's but have modern GPU's that run geometry shaders quickly.
- 'export MGL_GS_RENDER=true'

An alternative rendering mode can be enabled by exporting a variable to bash before compiling with the makefile.
This mode benefits computers with slow CPU's but have modern GPU's that run instancing shaders quickly.
- 'export MGL_INST_RENDER=true'

An alternative VBO mode can be enabled by exporting a variable to bash before compiling with the makefile. 
This mode allows faster vertex_buffer.bind_buffer() switching because it uses OpenGL 4.3 features to separate VBO specification from within VAO state.
This mode requires using a OpenGL 4.3 core profile.
- 'export MGL_VB43=true'

### For compiling on CYGWIN:

1. Add a symbolic link to the mingw g++ executable (example for x86):
- ln -s /usr/bin/i686-w64-mingw32-g++.exe /usr/bin/g++.exe
    
2. Add the following paths to your ENVIRONMENT.

#### PATHS FOR CYGWIN x86:
- C:\cygwin\lib\gcc\i686-w64-mingw32\5.4.0\include;
- C:\cygwin\usr\i686-w64-mingw32\sys-root\mingw\include;
- C:\cygwin\usr\i686-w64-mingw32\sys-root\mingw\bin;

#### PATHS FOR CYGWIN x86-64:
- C:\cygwin\lib\gcc\x86_64-w64-mingw32\5.4.0\include;
- C:\cygwin\usr\x86_64-w64-mingw32\sys-root\mingw\include;
- C:\cygwin\usr\x86_64-w64-mingw32\sys-root\mingw\bin;

### Run flags

#### -fps flag
The '-fps' flag is an optional parameter for controlling the target playback frame count per second. The default is 60. Adjusting the '-fps' flag may affect the playback speed of the game. If you use a multiple of 180 i.e... 1, 15, 30, 45, 60, or 90 the game time will match the time of our universe.
- Example: 'bin/game -fps 45' will render 45 frames per second.

#### -chunk flag
The '-chunk' flag is an optional parameter for controlling the size of each chunk. The default is 16 and must be an even divisible factor of the grid size, and greater than or equal to 2.
- Example: 'bin/game -chunk 32' produce chunks of size 32x32x32.

#### -grid flag
The '-grid' flag is an optional parameter for controlling the half size of world grid. The default is 64 and greater than or equal to 4.
Any previous saves will be deleted upon resizing the game grid to avoid crashing the game.
- Example: 'bin/game -grid 72 -chunk 18' produce a grid of size 144x144x144 and chunks of size 18x18x18.

#### -view flag
The '-view' flag is an optional parameter for controlling how many chunks are viewable on the screen. The default is 3 and must be an odd number greater than one.
- Example: 'bin/game -view 31' will render 15 chunks on each side of the player, (15 * 2) + 1 = 31.

#### -width flag and -height flag
The '-width' and '-height' flag changes the default window dimensions.
- Example: 'bin/game -width 1600 -height 900' will created a window width of 1600 pixels and height of 900 pixels.


### PLAYER USAGE INSTRUCTIONS:
![Alt text](pictures/spawn.png)

1. Keys [WASD]: forward/left/back/right
2. Key [Q]: switch between block edit mode and power suit mode
3. Keys [ZXC]: scale unit block (block edit mode)
4. Key [E]: reset scale unit block (block edit mode)
5. Keys [1-8]: switch textures (edit mode), switch power suit skills (power suit mode)
6. Left mouse: use skill (edit mode / power suite mode)
7. Key [F1]: quit
8. Key [F2]: toggle debug text
9. Key [F3]: music up
10. Key [F4]: music down
11. Key [ESCAPE]: pause the game


### Optimization: Help! I am lagging!
Lag usually comes in two forms, a CPU bottleneck or a GPU bottleneck. If you are CPU bottlenecked, expect most of your CPU time going to creating terrain memory buffers.

#### CPU Bottleneck
The MGL_VB43 compile time flag can only be used on OpenGL 4.3 systems, but will reduce the number of OpenGL calls by a factor of 2x. 
If your GPU supports OpenGL geometry shaders, the MGL_GS_RENDER compile time flag should alleviate any CPU bottleneck since it removes the need for the CPU to create terrain memory buffers. However this can **increase lag if** your GPU doesn't render geometry shaders efficiently. MGL_INST_RENDER is basically in the same vein as MGL_GS_RENDER and should alleviate most the of terrain processing and memory overhead compared with the default option. This flag requires the CPU to batch matrices instead of entire geometry primitives, so some CPU processing is still required, unlike that of MGL_GS_RENDER which requires no processing at all.

Known CPU bottleneck system configurations include AMD Phenom II x4 965 + Radeon HD 5770. However the MGL_VB43 and MGL_GS_RENDER increase performance to playable status, where previously the game would be unplayable.

#### Running Linux?
If you are running Linux, consider disabling any desktop compositor that may be running. The desktop compositor that runs with KDE Plasma can increase lag significantly.

#### GPU Bottleneck
If you have tried all previous steps and failed, you are likely GPU bottlenecked. In this case you are lagging simply because the GPU can not draw all of the pixels on the screen. The most obvious remedy to this problem is to reduce your screen dimensions using the '-width' and '-height' command line flags. 

However you can also try changing the size of the view frustum and chunk size using the '-view' and '-chunk' command line flags. By default the view frustum consists of 27 chunks, 3x3x3. This is actually alot of geometry to render, so we only draw the chunks inside the camera frustum which is on average ~18 (but can be 27 on a world edge). Decreasing the chunk size directly decreases the number of triangles drawn at any one time. The view frustum's lowest value is currently the default at 27 chunks. Increasing the frustum will draw more chunks and thus more geometry. 

You can also set the render speed of the game by using the '-fps' command line flag. The default is 60 and if you are lagging you can drop down to 30 without many issues. However, decreasing the frames per second will increase the workload for the physics engine because it is coded to run at 600 times per second independent of the frame count. Thus decreasing the frame count will increase the number of physics iterations per frame. This is only important if you are both CPU and GPU bottlenecked.
