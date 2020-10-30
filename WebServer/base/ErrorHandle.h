/*
 * File: ErrorHandle.h
 * Project: base
 * Author: Sandy
 * Last Modified: 2020-10-30 19:56:30
 */

void _errorAbort(const char* filename, int lineNumber, const char* message);

#define errorAbort(message) _errorAbort(__FILE__, __LINE__, message)
