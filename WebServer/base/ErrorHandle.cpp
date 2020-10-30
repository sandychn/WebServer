#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>

void _errorAbort(const char* filename, int lineNumber, const char* message) {
    char* errorStr = strerror(errno);
    fprintf(stderr, "%s:%d: %s: %s\n", filename, lineNumber, message, errorStr);
    abort();
}
