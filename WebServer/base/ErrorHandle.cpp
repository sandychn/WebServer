/*
 * File: ErrorHandle.cpp
 * Project: base
 * Author: sandy
 * Last Modified: 2020-10-30 19:56:23
 */

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>

void _errorAbort(const char* filename, int lineNumber, const char* message) {
    char errorStr[256];
    strerror_r(errno, errorStr, sizeof(errorStr));
    fprintf(stderr, "%s:%d: %s: %s\n", filename, lineNumber, message, errorStr);
    abort();
}
