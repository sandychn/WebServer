void _errorAbort(const char* filename, int lineNumber, const char* message);

#define errorAbort(message) _errorAbort(__FILE__, __LINE__, message)
