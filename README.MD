# MGLCRAFT
Repository for the Minimal Graphics Library Crafting Simulator

The GNU makefile contains various build targets.

'make' - builds game and tests targeting 'native'

'make build' - builds game and targeting 'native'

'make build32' - builds game and targeting 'x86'

'make build64' - builds game and targeting 'x86-64'

'make tests' - builds only tests targeting 'native'

'make clean' - cleans up all generated output files

These build targets have been tested for compilation on Arch Linux x64 and Windows 7 x86/x86-x64 platforms.

Additional dependencies that are required and not distributed in this package include:

    'mgl' - Minimal Graphics Library (https://github.com/Aaron-SP/mgl)

    'mingw32' - for win32 only

    'freetype' - for win32 and linux

    'X11' - for linux only


LD_LIBRARY_PATH will override the GNU makefile with the path looking for MGL installed on the system.
The default path is 'C:/cygwin/usr/i686-w64-mingw32/sys-root/mingw/include' for CYGWIN x86 or '/usr/include' for Linux systems.


For compiling on CYGWIN:

1.) Add a symbolic link to the mingw g++ executable (example for x86):

    ln -s /usr/bin/i686-w64-mingw32-g++.exe /usr/bin/g++.exe
    
2.) Add the following paths to your ENVIRONMENT.


PATHS FOR CYGWIN x86:

    C:\cygwin\lib\gcc\i686-w64-mingw32\5.4.0\include;

    C:\cygwin\usr\i686-w64-mingw32\sys-root\mingw\include;

    C:\cygwin\usr\i686-w64-mingw32\sys-root\mingw\bin;


PATHS FOR CYGWIN x86-64:

    C:\cygwin\lib\gcc\x86_64-w64-mingw32\5.4.0\include;

    C:\cygwin\usr\x86_64-w64-mingw32\sys-root\mingw\include;

    C:\cygwin\usr\x86_64-w64-mingw32\sys-root\mingw\bin;

PLAYER INSTRUCTIONS:

1.) Keys [WASD] to move

2.) Enter/Exit block edit mode

3.) Keys [ZXC] to scale unit block

4.) Keys [1-8] to switch textures (edit mode)

5.) Left mouse button to place and remove blocks (edit mode)

6.) Right mouse button to launch a grappling hook

7.) Key [F1] to quit

8.) Key [F2] to toggle debug text
