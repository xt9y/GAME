#include <cbuild.h>

static void configurePlatform(C_Target *target)
{
#ifdef __APPLE__
    c_define(target, "GL_SILENCE_DEPRECATION");
    c_include(target, "/opt/homebrew/include");
    c_link_flag(target, "-L/opt/homebrew/lib");
    c_framework(target, "OpenGL");
    c_framework(target, "Cocoa");
    c_framework(target, "IOKit");
    c_framework(target, "CoreVideo");
#else
    c_link_system(target, "GL");
    c_link_system(target, "GLU");
    c_link_system(target, "m");
    c_link_system(target, "dl");
    c_link_system(target, "pthread");
#endif
    c_link_system(target, "glfw");
}

void build(C_Build *b)
{
    C_Target *game = c_executable(b, "game");

    c_sources(game, "main.cpp");
    c_include(game, "../ECS-MODEL-RASTERIZER");
    c_include(game, "/usr/local/include/lwcgl-2.9.3");
    c_flag(game, "-std=c++20");
    c_warnings_strict(game);

    configurePlatform(game);
    c_link_system(game, "stdc++");

    c_link_flag(game, "-L../ECS-MODEL-RASTERIZER/build/debug");
    c_link_flag(game, "-lecs-model-rasterizer");
    c_link_flag(game, "-L/usr/local/lib");
    c_link_flag(game, "-llwcgl-2.9.3");
    c_link_flag(game, "-Wl,-rpath,/usr/local/lib");
#ifdef __APPLE__
    c_link_flag(game, "-Wl,-rpath,@loader_path/../../../ECS-MODEL-RASTERIZER/build/debug");
#else
    c_link_flag(game, "-Wl,-rpath,$ORIGIN/../../../ECS-MODEL-RASTERIZER/build/debug");
#endif

    c_default_target(b, game);
}
