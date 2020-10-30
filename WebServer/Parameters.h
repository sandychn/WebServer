/*
 * File: Parameters.h
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 19:58:33
 */

#include <string>
#include <getopt.h>
#include "base/ErrorHandle.h"

const std::string defaultLogPath = "./WebServer.log";

struct Parameters {
    int threadNum;
    int port;
    std::string logPath;

    Parameters(): threadNum(4), port(80), logPath(defaultLogPath) {}

    static Parameters getParameters(int argc, char *argv[]) {
        Parameters parameters;

        // parse args
        int opt;
        const char *str = "t:l:p:";
        while ((opt = getopt(argc, argv, str)) != -1) {
            switch (opt) {
                case 't': {
                    parameters.threadNum = atoi(optarg);
                    break;
                }
                case 'l': {
                    parameters.logPath = optarg;
                    if (parameters.logPath.size() < 2 || optarg[0] != '/') {
                        errorAbort("logPath should start with \"/\"");
                    }
                    break;
                }
                case 'p': {
                    parameters.port = atoi(optarg);
                    break;
                }
                default:
                    break;
            }
        }

        return parameters;
    }
};
