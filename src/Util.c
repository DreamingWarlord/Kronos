#include "Common.h"

void BugOn(char *expr_str, char *file, int line)
{
	printf("\x1B[1m%s:%d:\x1B[31;22m Assertion failed: \x1B[0m%s\n", file, line, expr_str);
	exit(1);
}
