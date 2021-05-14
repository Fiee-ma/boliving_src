// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "log.h"
#include <iostream>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static std::mutex s_mtxLog;
static LogLevel s_level = LogLevel::Debug;

int _vscprintf(const char *format, va_list pargs)
{
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}

void CLog::log(LogLevel level, std::string file, std::string func, int line, const char *format, ...)
{
    if (s_level > level)
    {
        return;
    }
    s_mtxLog.lock();
    char *pszStr = NULL;
    if (NULL != format)
    {
        va_list marker;
        va_start(marker, format);
        size_t nLength = _vscprintf(format, marker) + 1;
        pszStr = new char[nLength];
        memset(pszStr, '\0', nLength);
        vsnprintf(pszStr, nLength, format, marker);
        va_end(marker);
    }
    std::string sLog = file + "::" + std::to_string(line) + "::" + func + "::" + pszStr + "\r\n";
    std::cout << sLog.c_str();
    delete[] pszStr;
    s_mtxLog.unlock();
}

void CLog::setLogParam(LogLevel level, std::string path)
{
    s_mtxLog.lock();
    s_level = level;
    if (path != "")
    {
        FILE *fp = freopen(path.c_str(), "a", stdout);
        if(fp == nullptr) {
            std::cout << "freopen" << path.c_str() << "failed!" << std::endl;
        }
    }
    s_mtxLog.unlock();
}

CLog::CLog()
{
}

CLog::~CLog()
{
}
