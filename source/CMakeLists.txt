# Main Game
make_program("game")
add_openal("game")
add_opengl("game")
add_freetype("game")
add_vorbis("game")
target_compile_definitions("game" PRIVATE MGL_INLINE)
