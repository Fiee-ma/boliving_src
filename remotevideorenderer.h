#ifndef EXAMPLES_PEERCONNECTION_CLIENT_REMOTE_RENDERER_
#define EXAMPLES_PEERCONNECTION_CLIENT_REMOTE_RENDERER_

#include "api/video/video_frame_buffer.h"
#include "api/video/video_frame.h"
#include "api/media_stream_interface.h"
#include "examples/peerconnection/client/linux/commontypes.h"
#include "examples/peerconnection/client/linux/videorenderer.h"

class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
   public:
    VideoRenderer(webrtc::VideoTrackInterface* track_to_render);
    virtual ~VideoRenderer();

    // VideoSinkInterface implementation
    void OnFrame(const webrtc::VideoFrame& frame) override;
    void  SaveVideoFrameToFile(const webrtc::VideoFrame& frame, std::string file);

   protected:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
    CVideoRenderer renderer_ ;
  };

  #endif