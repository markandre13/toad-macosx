#ifndef _FISCHLAND_FISCHLAND_HH
#define _FISCHLAND_FISCHLAND_HH

#include <string>
#include <toad/cursor.hh>

std::string resource(const std::string &file);
#define RESOURCE(file) resource(file)

namespace fischland {
  enum {
    CURSOR_PEN = 0,
    CURSOR_PEN_CLOSE = 1,
    CURSOR_PEN_EDGE = 2,
    CURSOR_PEN_DRAG = 3,
    CURSOR_PENCIL_CLOSE = 4,
    CURSOR_PENCIL = 5
  };
  extern toad::TCursor *cursor[16];
}

#endif
