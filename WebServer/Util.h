#pragma once

#include <cstdlib>
#include <string>

namespace Util {
ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, const void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handleForSigpipe();
int setSocketNonBlocking(int fd);
void setSocketNoDelay(int fd);
void setSocketNoLinger(int fd);
void shutDownWR(int fd);
int socketBindListen(int port);
std::string replaceCRLF(const std::string& str);
std::string getServerName();
}  // namespace Util
