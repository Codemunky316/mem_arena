#ifndef MEM_ARENA_H
#define MEM_ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////
/// Memory Arena
/// No Fancy-Pants stuff.
/// Just a large hunk of memory.
/// Just a single c header file that includes the implementation.
/// sorrynotsorry
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
/// Declarations
////////////////////////////////////////////////////////////////////

typedef struct
{
    char* _base;
    size_t _offset;
    size_t _size;
    
} Arena;

int
is_power_of_two(uintptr_t n);

int
Arena_init(Arena* arena, size_t size);

void*
Arena_allocate_aligned(Arena* arena, size_t size, size_t align);

void*
Arena_allocate(Arena* arena, size_t size);

void
Arena_reset(Arena* arena);

void
Arena_destroy(Arena* arena);

////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////

/// This is the 'bitwise AND' trick.
/// Powers of 2 have exactly 1 bit set in their representation, 
/// so if you subtract 1, none of the bits would be equal in the resulting number.
int
is_power_of_two(uintptr_t n)
{
    return (n > 0) && ((n & (n-1)) == 0);
}

int
Arena_init(Arena* arena, size_t size)
{
    // TODO(mike): build overflow-safe arena with padding before and after used size that is write-restricted

    char* base = malloc(size);
    if (!base) {
        return -1;
    }

    arena->_base = base;
    arena->_offset = 0;
    arena->_size = size;

    return 0;
}

void*
Arena_allocate_aligned(Arena* arena, size_t size, size_t align)
{
    if (size <= 0) {
        return NULL;
    }

    if (!arena->_base) {
        return NULL;
    }

    if (arena->_offset + size > arena->_size) {
        // arena out of memory
        // TODO(mike): maybe set errno?
        return NULL;
    }

    // power of 2 alignment
    uintptr_t new_offset = arena->_offset + size;
    if (!is_power_of_two(new_offset)) {
        uintptr_t p, a, modulo;

        assert(is_power_of_two(align));

        p = new_offset;

        a = (uintptr_t) align;

        // Same as (p % a) but faster as 'a' is a power of two    
        modulo = p & (a-1);
        if (modulo != 0) {
            // If 'p' address is not aligned, push the address to the
            // next value which is aligned
            p += a - modulo;
        }
        new_offset = p;
    }

    void* ptr = &arena->_base[new_offset];
    arena->_offset = new_offset + size;
    // we'll be nice and zero the allocated memory
    memset(ptr, 0, size);

    return ptr;
}

#ifndef DEFAULT_ALIGN
#define DEFAULT_ALIGN (2*(sizeof(void *)))
#endif
void*
Arena_allocate(Arena *arena, size_t size) 
{
    return Arena_allocate_aligned(arena, size, DEFAULT_ALIGN);
}


void
Arena_reset(Arena* arena)
{
    arena->_offset = 0;
}

void
Arena_destroy(Arena* arena)
{
    free(arena->_base);
    arena->_base = NULL;
    arena->_offset = 0;
    arena->_size = 0;
}
////////////////////////////////////////////////////////////////////

#endif