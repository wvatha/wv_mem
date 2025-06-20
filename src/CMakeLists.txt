file(GLOBAL GLOB_RECURSE SRC_FILES "*.c")

add_library(wv_mem STATIC ${SRC_FILES})