typedef unsigned int uint;
typedef unsigned long ulong;

#include <sys/types.h> /* for size_t and ssize_t */
typedef size_t usize;
typedef ssize_t ssize;

#include <stdint.h> /* for (u)int*_t */
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;

#define AUTOLIB(x) static int __autolib_##x=0
