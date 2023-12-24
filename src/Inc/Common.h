#pragma once

#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>


#define TRUE ((bool) 1)
#define FALSE ((bool) 0)

#define Assert(expr) if(!(expr)) BugOn(#expr, __FILE__, __LINE__)
#define containerof(ptr, struc, member) ((void *) ((uint8 *) ptr - offsetof(struc, member)))

#define LOG_INT 0
#define LOG_MEM 1
#define LOG_TLB 2

#define PROC_MAX 64


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint32_t bool;


void BugOn(char *expr_str, char *file, int line);

void LogLevel(uint64 level);

void Log(uint64 vec, char *fmt, ...);

void LoopRegister(void (*update) (double delta, double time));

void Loop();
