#pragma once
#include "BaseApp.h"
#include <SDL2/SDL.h>
extern boost::signals2::signal<void(VariantList*)> g_sig_SDLEvent; //allow anyone to tap into SDL events
void SDLSetFullScreen();


