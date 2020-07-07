
#pragma once

/*
 * Our subset of sys/types.h
 */

typedef unsigned int size_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef char      int8_t;
typedef short     int16_t;
typedef int       int32_t;
typedef long long int64_t;

#define false 0
#define true 1
typedef uint8_t bool;


// Bit & Bit Field operations

#define BIT_SET(value, nbit)   ((value) |=  (1 << (nbit)))
#define BIT_CLEAR(value, nbit) ((value) &= ~(1 << (nbit)))
#define BIT_FLIP(value, nbit)  ((value) ^=  (1 << (nbit)))
#define BIT_CHECK(value, nbit) ((value) &   (1 << (nbit)))

// Produces [length] consecutive 1's.
#define BITS_ONE(length)                    ((1 << (length)) - 1)
#define BITFIELD_GET(value, offset, length) (((value) >> (offset)) & BITS_ONE(length))
#define BITFIELD_MERGE(old_value, new_bits, new_offset, new_length) \
    (((old_value) & (~(BITS_ONE(new_length) << (new_offset)))) \
        | (((new_bits) & BITS_ONE(new_length)) << (new_offset)))
#define BITFIELD_SET(field, value, offset, length) \
    ((field) = BITFIELD_MERGE(field, value, offset, length))
