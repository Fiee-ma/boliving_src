// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "videorenderer.h"
#include <cstring>
#include <iostream>
#include <thread>
#include <pthread.h>


using namespace std;

CVideoRenderer::CVideoRenderer()
{
    m_width = 0;
    m_height = 0;
    //struct timeval m_tv;
    m_num = 0;
    m_fLocalARGB = nullptr;
    m_fLocalLatency = nullptr;
}

std::mutex m;
void CVideoRenderer::RenderFrame(std::unique_ptr<VideoBuffer> videoFrame)
{
    //LOG_DEBUG("");
    if (videoFrame && m_fLocalARGB && m_fLocalLatency)
    {
        std::cout << "videorenderer.cc 25 lines CVideoRenderer::RenderFrame()" << std::endl;
        m_width = videoFrame->resolution.width;
        m_height = videoFrame->resolution.height;
        std::cout << "m_num = " << m_num++ << std::endl;
        if (m_num == 40)   // 表示每40帧记录一次
        {
            std::cout << "videorenderer.cc 31 lines fprintf(m_fLocalARGB,  timestamp) before" << std::endl;
            gettimeofday(&m_tv, NULL);
            long timestamp = m_tv.tv_sec % 10000 * 1000 + m_tv.tv_usec / 1000;
            fprintf(m_fLocalARGB, "%ld,", timestamp);
            fprintf(m_fLocalLatency, "%ld,", timestamp);
            std::cout << "videorenderer.cc 31 lines fprintf(m_fLocalARGB,  timestamp) after" << std::endl;
            int value = 0;
            uint8_t *ptrTmp = videoFrame->buffer;
                 for (long i = 0; i < m_width * m_height * 4; ++i)
                {
                    value = (int)(*ptrTmp);
                    ptrTmp++;
                    fprintf(m_fLocalARGB, "%d", value);
                    fprintf(m_fLocalARGB, ",");
                    if (i / 4 % m_width >= 0 && i / 4 % m_width <= 239 && i / 4 / m_width >= 0 && i / 4 / m_width <= 59)
                    {
                        fprintf(m_fLocalLatency, "%d,", value);
                        fflush(m_fLocalLatency);
                    }
                    fflush(m_fLocalARGB);
                }
        //base::Thread th1(&excute, "test");
           
            m_num = 0;
        }
    }
    else
    {
        std::cout << "videoFrame is null" << std::endl;
    }
}

VideoRendererType CVideoRenderer::Type()
{
    //LOG_DEBUG("");
    return VideoRendererType::kARGB;
}

CVideoRenderer::~CVideoRenderer()
{
    if (m_fLocalARGB)
    {
        fclose(m_fLocalARGB);
        m_fLocalARGB = nullptr;
    }
    if (m_fLocalLatency)
    {
        fclose(m_fLocalLatency);
        m_fLocalLatency = nullptr;
    }
}

void CVideoRenderer::SetLocalARGBFile(const std::string &file)
{
    m_fLocalARGB = fopen(file.c_str(), "w");
    if(!m_fLocalARGB) {
        std::cout << "fopen() LocalARGBFile.txt failed" << std::endl;
    }
}

void CVideoRenderer::SetLocalLatencyFile(const std::string &file)
{
    m_fLocalLatency = fopen(file.c_str(), "w");
    if(!m_fLocalLatency) {
        std::cout << "fopen() LocalLatencyFile.txt failed" << std::endl;
    }
}

void CVideoRenderer::SetLocalRenderFile(const std::string &file) {
    m_fLocalRender = fopen(file.c_str(), "w");
    if(!m_fLocalRender) {
        std::cout << "SetLocalRenderFile failed" << std::endl;
    }
}
