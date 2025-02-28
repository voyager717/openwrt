#ifndef __math_compat_h
#define __math_compat_h

/**
 * @file
 * @brief Do not use, json-c internal, may be changed or removed at any time.
 */

#undef isnan
#define isnan(x) __builtin_isnan(x)
#undef isinf
#define isinf(x) __builtin_isinf(x)

#endif
