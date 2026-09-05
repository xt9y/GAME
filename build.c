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
    c_link_system(target, "c++");
#else
    c_link_system(target, "GL");
    c_link_system(target, "GLU");
    c_link_system(target, "m");
    c_link_system(target, "dl");
    c_link_system(target, "pthread");
    c_link_system(target, "stdc++");
#endif
    c_link_system(target, "glfw");
}

void build(C_Build *b)
{
    C_Target *game = c_executable(b, "game");

    c_sources(game, "main.cpp");
    c_include(game, "/usr/local/include/lwcgl-2.9.3");
    c_flag(game, "-std=c++20");
    c_warnings_strict(game);

    C_Dependency *rasterizer = c_git(
        b,
        "ecs-model-rasterizer",
        "https://github.com/xt9y/ECS-MODEL-RASTERIZER.git",
        "main"
    );
    c_dep_cbuild(rasterizer, "ecs-model-rasterizer", C_TARGET_SHARED_LIBRARY);
    c_dep_include(rasterizer, ".");
    c_use(game, rasterizer);

    configurePlatform(game);
    c_link_flag(game, "-L/usr/local/lib");
    c_link_flag(game, "-llwcgl-2.9.3");
    c_link_flag(game, "-Wl,-rpath,/usr/local/lib");

    c_default_target(b, game);
}
