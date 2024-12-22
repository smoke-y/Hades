#pragma once

#define ARRAY_LENGTH(x)   sizeof(x)/sizeof(x[0])
#define SET_BIT(x, pos)   x |=  (1 << (u32)pos)
#define CLEAR_BIT(x, pos) x &= ~(1 << (u32)pos)
#define IS_BIT(x, pos)    ((((u32)(x))>>((u32)pos)) & 1)

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long int  u64;
typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;
typedef signed long long int    s64;
typedef unsigned char           b8;
typedef unsigned short          b16;
typedef unsigned int            b32;
typedef unsigned long long int  b64;
typedef float                   f32;
typedef double                  f64;

#define TRUE  1
#define FALSE 0

#define PANIC(...) kprint("\n[PANIC] %s(%s:%d) ", __FILE__, __FUNCTION__, __LINE__);kprint(__VA_ARGS__)
#define ASSERT(x) if(!(x)){kprint("\n[ASSERT] %s(%s:%d) %s\n", __FILE__, __FUNCTION__, __LINE__, #x);}