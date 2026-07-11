#include "std.h"
#include "bbsys.h"

bool debug;
gxRuntime* gx_runtime;
gxCanvas* gx_depth_canvas = nullptr;
const char* errorfunc = "";
const char* errorlog = "";