#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"

const char* gdkx112Name = "libgdk-x11-2.0.so.0";
#define LIBNAME gdkx112

#include "wrappedlib_init.h"

#define CUSTOM_INIT \
    lib->priv.w.needed = 3; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libgobject-2.0.so.0"); \
    lib->priv.w.neededlibs[1] = strdup("libgio-2.0.so.0");  \
    lib->priv.w.neededlibs[1] = strdup("libgdk_pixbuf-2.0.so.0");