// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <stdio.h>
#include <string>
#include <fstream>
#include "videoencoderinterface.h"

class CEncodedVideoInput : public owt::base::VideoEncoderInterface
{
public:
    static CEncodedVideoInput *Create(const std::string &videoFile, owt::base::VideoCodec codec);
    CEncodedVideoInput(const std::string &videoFile, owt::base::VideoCodec codec);
    ~CEncodedVideoInput();

    virtual bool InitEncoderContext(owt::base::Resolution &resolution, uint32_t fps, uint32_t bitrate, owt::base::VideoCodec video_codec) override;
    virtual bool EncodeOneFrame(std::vector<uint8_t> &buffer, bool keyFrame) override;
    virtual bool Release() override;
    virtual VideoEncoderInterface *Copy() override;
    void SetPublishTimeFile(const std::string &file);

private:
    std::string m_videoPath;
    owt::base::VideoCodec m_codec;
   std::fstream m_fd;
    FILE *m_fLocalPublishTime;
    std::string m_publishTimeFile;
};
