#include <iostream>

#include <glowwindow/ContextFormat.h>
#include <glowwindow/Context.h>
#include <glowwindow/Window.h>

#include "game.h"

using namespace glowwindow;

int main()
{
    ContextFormat format;
    format.setVersion(3, 3);
    format.setProfile(ContextFormat::CoreProfile);

    Window window;

    if (window.create(format, "Elemate")) {
        window.context()->setSwapInterval(Context::VerticalSyncronization);

        window.show();
    }
    else {
        return -1;
    }

    //return MainLoop::run();

    Game game(window);

    game.start();

    return 0;
}
