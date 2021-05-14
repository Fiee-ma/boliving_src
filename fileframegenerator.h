// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <stdio.h>
#include <string>
#include <fstream>
#include "modules/sdk/include/cpp/owt/base/framegeneratorinterface.h"

class CFileFrameGenerator : public owt::base::VideoFrameGeneratorInterface
{
public:
  CFileFrameGenerator(int width, int height, int fps, std::string filename);
  ~CFileFrameGenerator();

  uint32_t GetNextFrameSize();
  uint32_t GenerateNextFrame(uint8_t *frame_buffer, const uint32_t capacity);
  int GetHeight();
  int GetWidth();
  int GetFps();
  VideoFrameGeneratorInterface::VideoFrameCodec GetType();
  void SetPublishTimeFile(const std::string &file);

private:
  int m_countTag;
  int m_width;
  int m_height;
  int m_fps;
  uint32_t m_frameDataSize;
  std::string m_videoPath;
  VideoFrameGeneratorInterface::VideoFrameCodec m_type;
  std::fstream m_fd;
  FILE *m_fLocalPublishTime;
};
