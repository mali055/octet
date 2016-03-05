////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Text overlay
//

#pragma warning(disable : 4267)
#define OCTET_BULLET 1

#include "../../octet.h"

#include "tools_middleware_1.h"

/// Create a box with octet
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::tools_middleware_1 app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}


