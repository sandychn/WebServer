/*
 * File: HttpData.cpp
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 20:30:04
 */

#include "HttpData.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Util.h"
#include "Config.h"
#include "time.h"

#include "Logger.h"

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;              // ms
const int DEFAULT_KEEP_ALIVE_TIME = 60 * 1000;      // ms

char favicon[555] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',  '\x0',  '\x0',  '\xD',  'I',    'H',
    'D',    'R',    '\x0',  '\x0',  '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',  '\x0',
    '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',  '\x19', 't',    'E',    'X',    't',    'S',
    'o',    'f',    't',    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',    'e',    '\x20',
    'I',    'm',    'a',    'g',    'e',    'R',    'e',    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C',
    '\x0',  '\x0',  '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94', '\x93', '9',    'H',
    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D', 'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E',
    '\xA0', 'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA', 'J',    '\x9A', '\x80', '\x8',
    'A',    '\xB4', 'q',    '\x85', 'X',    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',    '\xC1', 'F',    '\xB4', '\x15', '\xCF',
    '\x22', 'X',    '\x98', '\xB',  'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',    '\xC9',
    '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF', '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8',
    '\xD5', 'j',    'H',    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF', '\xF7', '\x2F',
    '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',
    '\xAA', 'T',    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5', 'r',    '9',    'd',
    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',  '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',
    '\xA7', '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A', '\xC6', '\x16', '\x82', '\x20',
    'X',    'R',    '\x14', 'E',    '6',    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',  '\x9E', '\xBC', '\x9',  'A',    '\x7C',
    '\x3E', '\x1F', '\x83', 'D',    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',    '\xE5',
    '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',  '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40',
    'g',    '\x98', '\xE9', '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8', '\xBD', '\x15',
    '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20',
    '\x9F', '\xCF', '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',    '\x0',  'i',    '6',
    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84', '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B',
    '7',    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9', 'd',    '\x2',  'B',    '\xAF',
    '\x85', '2',    'f',    'F',    '\xBA', '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93', '\xA2', '\x28', '\xA0', 'S',    'j',
    '\xB1', '\xB8', '\xDF', '\x29', '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',    'j',
    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7',
    '\xA',  '\x26', '\xB6', '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',  '\xA',  '\x2E',
    '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27', '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB',
    '\xD6', 'G',    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF',
    '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',
    '\xAC', '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7', 'X',    '\x7E',
    '\x7E', '\x85', '\xBB', '\xCD', 'N',    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D', '\x7F', '\x9',  '2',    '\x95', '\xBF',
    '\x1F', '\xDB', '\xD7', 'c',    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3', '\x87',
    '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A', 'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',
    '\x0',  'I',    'E',    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};

void MimeType::init() {
    mime[".html"] = "text/html; charset=utf-8";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime[".mp4"] = "video/mp4";
    mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string &suffix) {
    pthread_once(&once_control, MimeType::init);
    return mime.find(suffix) == mime.end() ? mime["default"] : mime[suffix];
}

HttpData::HttpData(EventLoop *loop, int connfd)
    : loop_(loop),
      channel_(new Channel(loop, connfd)),
      fd_(connfd),
      error_(false),
      connectionState_(ConnectionState::H_CONNECTED),
      method_(HttpMethod::METHOD_GET),
      HTTPVersion_(HttpVersion::HTTP_11),
      nowReadPos_(0),
      state_(ProcessState::STATE_PARSE_URI),
      hState_(ParseState::H_START),
      keepAlive_(false) {
    channel_->setReadHandler(bind(&HttpData::handleRead, this));
    channel_->setWriteHandler(bind(&HttpData::handleWrite, this));
    channel_->setConnHandler(bind(&HttpData::handleConn, this));
}

void HttpData::reset() {
    fileName_.clear();
    nowReadPos_ = 0;
    state_ = ProcessState::STATE_PARSE_URI;
    hState_ = ParseState::H_START;
    headers_.clear();
    seperateTimer();
}

void HttpData::seperateTimer() {
    if (timer_.lock()) {
        std::shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->clearReq();
        timer_.reset();
    }
}

void HttpData::handleRead() {
    __uint32_t events_ = channel_->getEvents();
    do {
        bool zero = false;
        int read_num = Util::readn(fd_, inBuffer_, zero);
        Logger::getLogger().info("Request from fd={0}: {1}", fd_, Util::replaceCRLF(inBuffer_));
        Logger::getLogger().info("Request from fd={0}: {1}", fd_, Util::replaceCRLF(inBuffer_));
        Logger::getLogger().debug("read_num={0}, zero={1}", read_num, zero);
        if (connectionState_ == ConnectionState::H_DISCONNECTING) {
            inBuffer_.clear();
            break;
        }
        if (read_num < 0) {
            Logger::getLogger().warn("read_num<0: {0}", Util::getErrnoString());
            error_ = true;
            handleError(fd_, 400, "Bad Request");
            break;
        } else if (zero) {
            // 有请求出现但是读不到数据, 可能是Request Aborted, 或者来自网络的数据没有达到等原因
            // 最可能是对端已经关闭了, 统一按照对端已经关闭处理
            connectionState_ = ConnectionState::H_DISCONNECTING;
            if (read_num == 0) {
                break;
            }
        }

        if (state_ == ProcessState::STATE_PARSE_URI) {
            URIState flag = this->parseURI();
            if (flag == URIState::PARSE_URI_AGAIN) {
                break;
            } else if (flag == URIState::PARSE_URI_ERROR) {
                Logger::getLogger().warn("flag == URIState::PARSE_URI_ERROR : {0}", Util::getErrnoString());
                Logger::getLogger().warn("fd = {0}, inBuffer = ", fd_, Util::replaceCRLF(inBuffer_));
                inBuffer_.clear();
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            } else {
                state_ = ProcessState::STATE_PARSE_HEADERS;
            }
        }
        if (state_ == ProcessState::STATE_PARSE_HEADERS) {
            HeaderState flag = this->parseHeaders();
            if (flag == HeaderState::PARSE_HEADER_AGAIN) {
                break;
            } else if (flag == HeaderState::PARSE_HEADER_ERROR) {
                Logger::getLogger().warn("flag == HeaderState::PARSE_HEADER_ERROR : {0}", Util::getErrnoString());
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            }
            if (method_ == HttpMethod::METHOD_POST) {
                state_ = ProcessState::STATE_RECV_BODY;  // POST方法准备
            } else {
                state_ = ProcessState::STATE_ANALYSIS;
            }
        }
        if (state_ == ProcessState::STATE_RECV_BODY) {
            Logger::getLogger().info("state_ == ProcessState::STATE_RECV_BODY");
            auto it = headers_.find("Content-Length");
            int content_length = (it == headers_.end() ? -1 : stoi(it->second));
            if (it == headers_.end()) {
                error_ = true;
                handleError(fd_, 400, "Bad Request: Lack of argument (Content-Length)");
                break;
            }
            Logger::getLogger().debug("[state_ == ProcessState::STATE_RECV_BODY] content_length = {}", content_length);
            Logger::getLogger().debug("[state_ == ProcessState::STATE_RECV_BODY] inBuffer = {}", inBuffer_);
            if (static_cast<int>(inBuffer_.size()) < content_length) break;
            state_ = ProcessState::STATE_ANALYSIS;
        }
        if (state_ == ProcessState::STATE_ANALYSIS) {
            AnalysisState flag = this->analysisRequest();
            if (flag == AnalysisState::ANALYSIS_SUCCESS) {
                state_ = ProcessState::STATE_FINISH;
                break;
            } else {
                error_ = true;
                break;
            }
        }
    } while (false);

    if (!error_) {
        if (outBuffer_.size() > 0) {
            handleWrite();
        }
        // error_ may change
        if (!error_ && state_ == ProcessState::STATE_FINISH) {
            this->reset();
            if (inBuffer_.size() > 0 && connectionState_ != ConnectionState::H_DISCONNECTING) {
                handleRead();
            }
        } else if (!error_ && connectionState_ != ConnectionState::H_DISCONNECTED) {
            channel_->setEvents(events_ |= EPOLLIN);
        }
    }
}

void HttpData::handleWrite() {
    if (!error_ && connectionState_ != ConnectionState::H_DISCONNECTED) {
        __uint32_t events_ = channel_->getEvents();
        if (Util::writen(fd_, outBuffer_) < 0) {
            Logger::getLogger().error("writen error: {0}", Util::getErrnoString());
            channel_->setEvents(0);
            error_ = true;
        }
        if (outBuffer_.size() > 0) {
            Logger::getLogger().debug("outBuffer_.size() > 0");
            channel_->setEvents(events_ | EPOLLOUT);
        }
    }
}

void HttpData::handleConn() {
    seperateTimer();
    __uint32_t events_ = channel_->getEvents();
    if (!error_ && connectionState_ == ConnectionState::H_CONNECTED) {
        if (events_ != 0) {
            int timeout = DEFAULT_EXPIRED_TIME;
            if (keepAlive_) timeout = DEFAULT_KEEP_ALIVE_TIME;
            if ((events_ & EPOLLIN) && (events_ & EPOLLOUT)) {
                events_ = static_cast<__uint32_t>(0) || EPOLLOUT;
                channel_->setEvents(events_);
            }
            events_ |= EPOLLET;
            channel_->setEvents(events_);
            loop_->updatePoller(channel_, timeout); // will call addTimer()
        } else if (keepAlive_) {
            events_ |= (EPOLLIN | EPOLLET);
            channel_->setEvents(events_);
            int timeout = DEFAULT_KEEP_ALIVE_TIME;
            loop_->updatePoller(channel_, timeout); // will call addTimer()
        } else {
            events_ |= (EPOLLIN | EPOLLET);
            channel_->setEvents(events_);
            int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
            loop_->updatePoller(channel_, timeout); // will call addTimer()
        }
    } else if (!error_ && connectionState_ == ConnectionState::H_DISCONNECTING && (events_ & EPOLLOUT)) {
        events_ = (EPOLLOUT | EPOLLET);
        channel_->setEvents(events_);
    } else {
        if (error_) {
            Logger::getLogger().warn("close with error");
        } else {
            Logger::getLogger().info("runInLoop(bind(&HttpData::handleClose, shared_from_this()))");
        }
        loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
    }
}

URIState HttpData::parseURI() {
    // sample:  GET / HTTP/1.1

    // 读到完整的请求行再开始解析请求
    size_t pos = inBuffer_.find('\r', nowReadPos_);
    if (pos == std::string::npos) {
        return URIState::PARSE_URI_AGAIN;
    }
    // 去掉请求行所占的空间，节省空间
    std::string requestLine = inBuffer_.substr(0, pos);
    Logger::getLogger().debug("requestLine: {}", requestLine);
    if (inBuffer_.size() > pos + 1) {
        inBuffer_ = inBuffer_.substr(pos + 1);
    } else {
        inBuffer_.clear();
    }

    // method
    size_t posGet = std::string::npos;
    size_t posPost = std::string::npos;
    size_t posHead = std::string::npos;
    if ((posGet = requestLine.find("GET")) != std::string::npos) {
        pos = static_cast<int>(posGet);
        method_ = HttpMethod::METHOD_GET;
    } else if ((posPost = requestLine.find("POST")) != std::string::npos) {
        pos = static_cast<int>(posPost);
        method_ = HttpMethod::METHOD_POST;
    } else if ((posHead = requestLine.find("HEAD")) != std::string::npos) {
        pos = static_cast<int>(posHead);
        method_ = HttpMethod::METHOD_HEAD;
    } else {
        return URIState::PARSE_URI_ERROR;
    }

    // filename
    pos = requestLine.find("/", pos);
    if (pos == std::string::npos) {
        fileName_ = "index.html";
        HTTPVersion_ = HttpVersion::HTTP_11;
        return URIState::PARSE_URI_SUCCESS;
    } else {
        // requestLine[pos] == '/'
        size_t spaceAfterFilenamePos = requestLine.find(' ', pos);
        if (spaceAfterFilenamePos - pos > 1) {
            fileName_ = requestLine.substr(pos + 1, spaceAfterFilenamePos - pos - 1);
            size_t questionMarkPos = fileName_.find('?');
            fileName_ = fileName_.substr(0, questionMarkPos);
        } else {
            fileName_ = "index.html";
        }
        pos = spaceAfterFilenamePos;
    }

    // HTTP version
    pos = requestLine.find_first_not_of(' ', pos);
    if (pos == std::string::npos) {
        return URIState::PARSE_URI_ERROR;
    } else {
        std::string version(requestLine.begin() + pos, requestLine.end());
        if (version == "HTTP/1.0") {
            HTTPVersion_ = HttpVersion::HTTP_10;
        } else if (version == "HTTP/1.1") {
            HTTPVersion_ = HttpVersion::HTTP_11;
        } else {
            return URIState::PARSE_URI_ERROR;
        }
    }

    return URIState::PARSE_URI_SUCCESS;
}

HeaderState HttpData::parseHeaders() {
    std::string& str = inBuffer_;
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    size_t i = 0;

    // BUG FIXED: when notFinish == false, ++i should not be performed
    for (; i < str.size() && notFinish; notFinish && ++i) {
        switch (hState_) {
            case ParseState::H_START: {
                if (str[i] == '\n' || str[i] == '\r') break;
                hState_ = ParseState::H_KEY;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
            case ParseState::H_KEY: {
                if (str[i] == ':') {
                    key_end = i;
                    if (key_end - key_start <= 0) return HeaderState::PARSE_HEADER_ERROR;
                    hState_ = ParseState::H_COLON;
                } else if (str[i] == '\n' || str[i] == '\r')
                    return HeaderState::PARSE_HEADER_ERROR;
                break;
            }
            case ParseState::H_COLON: {
                if (str[i] == ' ') {
                    hState_ = ParseState::H_SPACES_AFTER_COLON;
                } else
                    return HeaderState::PARSE_HEADER_ERROR;
                break;
            }
            case ParseState::H_SPACES_AFTER_COLON: {
                hState_ = ParseState::H_VALUE;
                value_start = i;
                break;
            }
            case ParseState::H_VALUE: {
                if (str[i] == '\r') {
                    hState_ = ParseState::H_CR;
                    value_end = i;
                    if (value_end - value_start <= 0) return HeaderState::PARSE_HEADER_ERROR;
                } else if (i - value_start > 255)
                    return HeaderState::PARSE_HEADER_ERROR;
                break;
            }
            case ParseState::H_CR: {
                if (str[i] == '\n') {
                    hState_ = ParseState::H_LF;
                    std::string key(str.begin() + key_start, str.begin() + key_end);
                    std::string value(str.begin() + value_start, str.begin() + value_end);
                    headers_[key] = value;
                    now_read_line_begin = i;
                } else
                    return HeaderState::PARSE_HEADER_ERROR;
                break;
            }
            case ParseState::H_LF: {
                if (str[i] == '\r') {
                    hState_ = ParseState::H_END_CR;
                } else {
                    key_start = i;
                    hState_ = ParseState::H_KEY;
                }
                break;
            }
            case ParseState::H_END_CR: {
                if (str[i] == '\n') {
                    hState_ = ParseState::H_END_LF;
                } else
                    return HeaderState::PARSE_HEADER_ERROR;
                break;
            }
            case ParseState::H_END_LF: {
                notFinish = false;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
        }
    }
    if (hState_ == ParseState::H_END_LF) {
        str = str.substr(i);
        return HeaderState::PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return HeaderState::PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::analysisRequest() {
    std::string requestFileFullPath = Config::webPath + Config::pathSeperator + fileName_;

    if (method_ == HttpMethod::METHOD_GET) {
        std::string header;
        header += "HTTP/1.1 200 OK\r\n";

        do {
            auto it = headers_.find("Connection");
            if (it == headers_.end()) break;
            const std::string& connectionValue = it->second;
            if (connectionValue == "Keep-Alive" || connectionValue == "keep-alive") {
                keepAlive_ = true;
                header += "Connection: Keep-Alive\r\n";
                header += "Keep-Alive: timeout=" + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
            }
        } while (false);

        // echo test
        if (fileName_ == "hello") {
            Logger::getLogger().debug("echo test");
            std::string echoMessage = "Hello World";
            outBuffer_ = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
            outBuffer_ += std::to_string(echoMessage.length()) + "\r\n\r\n";
            outBuffer_ += echoMessage;
            return AnalysisState::ANALYSIS_SUCCESS;
        }

        if (fileName_ == "favicon.ico") {
            header += "Content-Type: " + MimeType::getMime(".ico") + "\r\n";
            header += "Content-Length: " + std::to_string(sizeof(favicon)) + "\r\n";
            header += "Server: " + Util::getServerName() + "\r\n";
            header += "\r\n";
            outBuffer_ += header;
            outBuffer_ += std::string(favicon, favicon + sizeof(favicon));
            return AnalysisState::ANALYSIS_SUCCESS;
        }

        struct stat sbuf;

        if (stat(requestFileFullPath.c_str(), &sbuf) < 0) {
            header.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }

        int dot_pos = fileName_.find('.');
        std::string filetype = MimeType::getMime(
            dot_pos < 0 ? "default" : fileName_.substr(dot_pos));

        header += "Content-Type: " + filetype + "\r\n";
        header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
        header += "Server: " + Util::getServerName() + "\r\n";
        header += "\r\n";
        outBuffer_ += header;

        int src_fd = open(requestFileFullPath.c_str(), O_RDONLY, 0);
        if (src_fd < 0) {
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }
        
        void* mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        close(src_fd);
        
        if (mmapRet == MAP_FAILED) {
            munmap(mmapRet, sbuf.st_size);
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }
        char* src_addr = static_cast<char*>(mmapRet);
        outBuffer_ += std::string(src_addr, src_addr + sbuf.st_size);
        munmap(mmapRet, sbuf.st_size);
        return AnalysisState::ANALYSIS_SUCCESS;
    }
    
    if (method_ == HttpMethod::METHOD_HEAD) {
        std::string header;
        header += "HTTP/1.1 200 OK\r\n";

        do {
            auto it = headers_.find("Connection");
            if (it == headers_.end()) break;
            const std::string& connectionValue = it->second;
            if (connectionValue == "Keep-Alive" || connectionValue == "keep-alive") {
                keepAlive_ = true;
                header += "Connection: Keep-Alive\r\n";
                header += "Keep-Alive: timeout=" + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
            }
        } while (false);

        // echo test
        if (fileName_ == "hello") {
            Logger::getLogger().debug("echo test");
            std::string echoMessage = "Hello World";
            outBuffer_ = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
            outBuffer_ += std::to_string(echoMessage.length()) + "\r\n\r\n";
            outBuffer_ += echoMessage;
            return AnalysisState::ANALYSIS_SUCCESS;
        }

        if (fileName_ == "favicon.ico") {
            header += "Content-Type: " + MimeType::getMime(".ico") + "\r\n";
            header += "Content-Length: " + std::to_string(sizeof(favicon)) + "\r\n";
            header += "Server: " + Util::getServerName() + "\r\n";
            header += "\r\n";
            outBuffer_ += header;
            outBuffer_ += std::string(favicon, favicon + sizeof(favicon));
            return AnalysisState::ANALYSIS_SUCCESS;
        }

        struct stat sbuf;
        if (stat(requestFileFullPath.c_str(), &sbuf) < 0) {
            header.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }

        int dot_pos = fileName_.find('.');
        std::string filetype = MimeType::getMime(dot_pos < 0 ? "default" : fileName_.substr(dot_pos));

        header += "Content-Type: " + filetype + "\r\n";
        header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
        header += "Server: " + Util::getServerName() + "\r\n";
        header += "\r\n";
        outBuffer_ += header;

        return AnalysisState::ANALYSIS_SUCCESS;
    }

    if (method_ == HttpMethod::METHOD_POST) {
        Logger::getLogger().debug("method_ == HttpMethod::METHOD_POST");
        int postContentLength = stoi(headers_["Content-Length"]);
        std::string postContent(inBuffer_.begin(), inBuffer_.begin() + postContentLength);
        Logger::getLogger().info("postContent = {}", postContent);
        inBuffer_.erase(inBuffer_.begin(), inBuffer_.begin() + postContentLength);

        std::string header;
        header += "HTTP/1.1 200 OK\r\n";
        do {
            auto it = headers_.find("Connection");
            if (it == headers_.end()) break;
            const std::string& connectionValue = it->second;
            if (connectionValue == "Keep-Alive" || connectionValue == "keep-alive") {
                keepAlive_ = true;
                header += "Connection: Keep-Alive\r\n";
                header += "Keep-Alive: timeout=" + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
            }
        } while (false);

        // echo test
        if (fileName_ == "hello") {
            Logger::getLogger().debug("echo test");
            std::string echoMessage = "Hello World";
            outBuffer_ = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
            outBuffer_ += std::to_string(echoMessage.length()) + "\r\n\r\n";
            outBuffer_ += echoMessage;
            return AnalysisState::ANALYSIS_SUCCESS;
        }

        if (fileName_ == "favicon.ico") {
            header += "Content-Type: " + MimeType::getMime(".ico") + "\r\n";
            header += "Content-Length: " + std::to_string(sizeof(favicon)) + "\r\n";
            header += "Server: " + Util::getServerName() + "\r\n";
            header += "\r\n";
            outBuffer_ += header;
            outBuffer_ += std::string(favicon, favicon + sizeof(favicon));
            return AnalysisState::ANALYSIS_SUCCESS;
        }

        struct stat sbuf;
        if (stat(requestFileFullPath.c_str(), &sbuf) < 0) {
            header.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }

        int dot_pos = fileName_.find('.');
        std::string filetype = MimeType::getMime(dot_pos < 0 ? "default" : fileName_.substr(dot_pos));

        header += "Content-Type: " + filetype + "\r\n";
        header += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
        header += "Server: " + Util::getServerName() + "\r\n";
        header += "\r\n";
        outBuffer_ += header;

        int src_fd = open(requestFileFullPath.c_str(), O_RDONLY, 0);
        if (src_fd < 0) {
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }
        
        void* mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        close(src_fd);
        
        if (mmapRet == MAP_FAILED) {
            munmap(mmapRet, sbuf.st_size);
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found");
            return AnalysisState::ANALYSIS_ERROR;
        }
        char* src_addr = static_cast<char*>(mmapRet);
        outBuffer_ += std::string(src_addr, src_addr + sbuf.st_size);
        munmap(mmapRet, sbuf.st_size);
        return AnalysisState::ANALYSIS_SUCCESS;
    }
    
    return AnalysisState::ANALYSIS_ERROR;
}

void HttpData::handleError(int fd, int err_num, std::string short_msg) {
    std::string errNumAndMsg = std::to_string(err_num) + " " + short_msg;

    std::string body_buff;
    body_buff += "<html>\r\n";
    body_buff += "<head>\r\n";
    body_buff += "    <meta charset=\"UTF-8\">\r\n";
    body_buff += "    <title>" + errNumAndMsg + "</title>\r\n";
    body_buff += "</head>\r\n";
    body_buff += "<body>\r\n";
    body_buff += "<span style=\"color: red\"><b>" + errNumAndMsg + "</b></span>\r\n";
    body_buff += "<hr>\r\n";
    body_buff += "<em>" + Util::getServerName() + "</em>\r\n";
    body_buff += "</body>\r\n";
    body_buff += "</html>\r\n";

    std::string header_buff;
    header_buff += "HTTP/1.1 " + errNumAndMsg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: " + Util::getServerName() + "\r\n";
    header_buff += "\r\n";
    
    // 错误处理不考虑writen不完的情况
    Util::writen(fd, header_buff.c_str(), header_buff.size());
    Util::writen(fd, body_buff.c_str(), body_buff.size());
}

void HttpData::handleClose() {
    connectionState_ = ConnectionState::H_DISCONNECTED;
    std::shared_ptr<HttpData> guard(shared_from_this());
    loop_->removeFromPoller(channel_);
}

void HttpData::newEvent() {
    channel_->setEvents(DEFAULT_EVENT);
    loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}
