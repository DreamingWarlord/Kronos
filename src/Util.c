#include "Common.h"


static uint64 log_level = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static const char *log_names[] = { "interrupt", "memory", "tlb" };


void BugOn(char *expr_str, char *file, int line)
{
	printf("\x1B[1m%s:%d:\x1B[31;22m Assertion failed: \x1B[0m%s\n", file, line, expr_str);
	exit(1);
}

void LogLevel(uint64 level)
{
	log_level = level;
}

void Log(uint64 vec, char *fmt, ...)
{
	Assert(vec < 64);

	if(!(log_level & (1ULL << vec)))
		return;

	pthread_mutex_lock(&log_mutex);
	printf("%s: ", log_names[vec]);
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	putchar('\n');
	pthread_mutex_unlock(&log_mutex);
}
