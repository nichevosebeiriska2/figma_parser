#include "allocation.h"

std::pmr::monotonic_buffer_resource g_arena_allocator{10*1024*1024};