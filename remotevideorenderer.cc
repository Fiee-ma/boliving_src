#include "remotevideorenderer.h"

#include "examples/peerconnection/client/linux/nativehandlebuffer.h"
#include "examples/peerconnection/client/linux/videorenderer.h"
#include "examples/peerconnection/client/linux/commontypes.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include <thread>

VideoRenderer::VideoRenderer(webrtc::VideoTrackInterface* track_to_render)
    : rendered_track_(track_to_render) {
        rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
        //renderer_ = CVideoRenderer();
        renderer_.SetLocalARGBFile("/home/jack-ma/QoSTestFramework/analysis/dataset/Data/localARGB.txt");
        renderer_.SetLocalLatencyFile("/home/jack-ma/QoSTestFramework/analysis/dataset/Data/localLatency.txt");
//        renderer_.SetLocalRenderFile("/home/jack-ma/webrtc_m76/src/LocalRenderFile.txt");
}

VideoRenderer::~VideoRenderer() {
        rendered_track_->RemoveSink(this);
}

void VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {
    VideoRendererType renderer_type = renderer_.Type();
    base::Resolution resolution(video_frame.width(), video_frame.height());
    SaveVideoFrameToFile(video_frame, "/home/jack-ma/webrtc_m76/src/LocalYUV.yuv");

    if (renderer_type == VideoRendererType::kARGB) {
        uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 4];
        webrtc::ConvertFromI420(video_frame, webrtc::VideoType::kARGB, 0,
                            static_cast<uint8_t*>(buffer));
        std::unique_ptr<VideoBuffer> video_buffer(
            new VideoBuffer{buffer, resolution, VideoBufferType::kARGB});
        
        renderer_.RenderFrame(std::move(video_buffer));  // 这个调用的是CVideoRenderer的RenderFrame
    } else {
        uint8_t* buffer = new uint8_t[resolution.width * resolution.height * 3 / 2];
        webrtc::ConvertFromI420(video_frame, webrtc::VideoType::kI420, 0,
                            static_cast<uint8_t*>(buffer));
        std::unique_ptr<VideoBuffer> video_buffer(
            new VideoBuffer{buffer, resolution, VideoBufferType::kI420});
        renderer_.RenderFrame(std::move(video_buffer));
  }
}

 void  VideoRenderer::SaveVideoFrameToFile(const webrtc::VideoFrame& frame, std::string file) {
     rtc::scoped_refptr<webrtc::VideoFrameBuffer> vfb = frame.video_frame_buffer();
    static FILE *fp = fopen(file.c_str(), "wb+");
    if (fp != NULL)	{
        fwrite(vfb.get()->GetI420()->DataY(), 1, frame.height() * frame.width(), fp);
        fwrite(vfb.get()->GetI420()->DataU(), 1, frame.height() * frame.width() / 4, fp); 
        fwrite(vfb.get()->GetI420()->DataV(), 1, frame.height() * frame.width() / 4, fp);
        fflush(fp);
    }
 }