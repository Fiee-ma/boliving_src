// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "commontypes.h"
#include <thread>
#include <mutex>
#include "thread.h"


enum class VideoBufferType {
  kI420,
  kARGB,
};

enum class VideoRendererType {
  kI420,
  kARGB,
};

struct VideoBuffer {
  /// Video buffer
  uint8_t* buffer;
  /// Resolution for the Video buffer
  base::Resolution resolution;
  // Buffer type
  VideoBufferType type;
  ~VideoBuffer() { delete[] buffer; }
};

class CVideoRenderer{
public:
	typedef base::Mutex MutexType;
	void RenderFrame(std::unique_ptr<VideoBuffer> videoFrame);
	VideoRendererType Type();
	CVideoRenderer();
	~CVideoRenderer();
	void SetLocalARGBFile(const std::string &file);
	CVideoRenderer &operator=(const CVideoRenderer) {
		return *this;
	}
	void SetLocalLatencyFile(const std::string &file);
	void SetLocalRenderFile(const std::string &file);
	FILE  *GetLocalARGBFILE() const {return m_fLocalARGB;}
	FILE  *GetLocalLatencyFILE() const {return m_fLocalLatency;}
	FILE  *GetLocalRenderFILE() const {return m_fLocalRender;}

private:
	int m_width;
	int m_height;
	struct timeval m_tv;
	int m_num;
	FILE *m_fLocalARGB;
	FILE *m_fLocalLatency;
	FILE *m_fLocalRender;
	MutexType m_mutex;
};
