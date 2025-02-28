/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * asm-generic/int-ll64.h
 *
 * Integer declarations for architectures which use "long long"
 * for 64-bit types.
 */

#ifndef _ASM_GENERIC_INT_LL64_H
#define _ASM_GENERIC_INT_LL64_H

typedef __signed__ char __s8;
<<<<<<< HEAD
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
=======

typedef __signed__ short __s16;

typedef __signed__ int __s32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
#else
typedef __signed__ long long __s64;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#endif

#endif /* _ASM_GENERIC_INT_LL64_H */
