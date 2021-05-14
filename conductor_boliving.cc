#include <iostream>
#include "conductor_boliving.h"

#include <stddef.h>
#include <stdint.h>
#include <iostream>

#include <memory>
#include <utility>
#include <vector>
#include <stdio.h>

#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "examples/peerconnection/client/linux/defaults.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/strings/json.h"
#include "test/vcm_capturer.h"
#include "rtc_base/thread.h"
#include <thread>

const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

class DummySetSessionDescriptionObserver: public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() { std::cout << __FILE__ << " "<< __FUNCTION__ << __LINE__<< std::endl; }
    virtual void OnFailure(webrtc::RTCError error) {
        std::cout << __FILE__ << __FUNCTION__ << " " << ToString(error.type()) << ": "
                    << error.message() << std::endl;
    }
};

class CapturerTrackSource : public webrtc::VideoTrackSource {
 public:
  static rtc::scoped_refptr<CapturerTrackSource> Create() {
    const size_t kWidth = 640;
    const size_t kHeight = 480;
    const size_t kFps = 30;
    std::unique_ptr<webrtc::test::VcmCapturer> capturer;
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (!info) {
      return nullptr;
    }
    int num_devices = info->NumberOfDevices();
    for (int i = 0; i < num_devices; ++i) {
      capturer = absl::WrapUnique(webrtc::test::VcmCapturer::Create(kWidth, kHeight, kFps, i));
      if (capturer) {
        return new rtc::RefCountedObject<CapturerTrackSource>(
            std::move(capturer));
      }
    }

    return nullptr;
  }

 protected:
  explicit CapturerTrackSource(
      std::unique_ptr<webrtc::test::VcmCapturer> capturer)
      : VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

 private:
  rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
    return capturer_.get();
  }
  std::unique_ptr<webrtc::test::VcmCapturer> capturer_;
};

Conductor::Conductor(PeerConnectionClient *client)
    :client_(client) {
        client_->RegisterObserver(this);
        localBitrate = fopen("/home/jack-ma/QoSTestFramework/analysis/dataset/Data/localBitrate.txt", "w");
        localFps = fopen("/home/jack-ma/webrtc_m76/src/LocalFps.txt", "w");
}

void Conductor::ConnectToPeer(bool flag) {
    auto option = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
    if(flag) {
      option.offer_to_receive_video = 1;
      option.offer_to_receive_audio = 1;
    } else {
      option.offer_to_receive_video = 0;
      option.offer_to_receive_audio = 0;
    }
    if (InitializePeerConnection(flag)) {
        peer_connection_->CreateOffer(
            this, option);
            std::cout << "success to initialize PeerConnection" << std::endl;
    } else {
            std::cout << "Failed to initialize PeerConnection" << std::endl;
    }
}

bool Conductor::InitializePeerConnection(bool flag) {
    rtc::ThreadManager::Instance()->WrapCurrentThread();
    network_thread_ = rtc::Thread::CreateWithSocketServer();
    network_thread_->SetName("network_thread", nullptr);
    network_thread_->Start();

    worker_thread_ = rtc::Thread::Create();
    worker_thread_->SetName("worker_thread", nullptr);
    worker_thread_->Start();

    signal_thread_ = rtc::Thread::Create();
    signal_thread_->SetName("signaling_thread", nullptr);
    signal_thread_->Start();
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
        network_thread_.get() /* network_thread */, worker_thread_.get() /* worker_thread */,
        signal_thread_.get() /* signaling_thread */, nullptr /* default_adm */,
        webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(),
        webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
        nullptr /* audio_processing */);

    if (!peer_connection_factory_) {
        std::cout << "CreatePeerConnectionFactory fail!" << std::endl;
        return false;
    }

    if (!CreatePeerConnection(true)) {
        std::cout << "CreatePeerConnection fail!" << std::endl;
        return false;
    }

    if(!flag) {
       AddTracks();
    }

    return peer_connection_ != nullptr;
}

bool Conductor::CreatePeerConnection(bool dtls) {
    RTC_DCHECK(peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = dtls;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    config.servers.push_back(server);

    peer_connection_ = peer_connection_factory_->CreatePeerConnection(
        config, nullptr, nullptr, this);
    return peer_connection_ != nullptr;
}

// 在创建完offer后就会调用Onsuccess这个函数
void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc){
    std::cout << "conductor_boliving.cc OnSuccess() SetLocalDescription before" << std::endl;
    peer_connection_->SetLocalDescription(
        DummySetSessionDescriptionObserver::Create(), desc);
    std::cout << "conductor_boliving.cc OnSuccess() SetLocalDescription after" << std::endl;

    std::string sdp1;
    desc->ToString(&sdp1);
    std::cout << "OnSucces sdp:\r\n" << sdp1 << std::endl;

    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = webrtc::SdpTypeToString(desc->GetType());  //kSessionDescriptionTypeName = "type"
    jmessage[kSessionDescriptionSdpName] = sdp1;  // kSessionDescriptionSdpName = "sdp"
    std::string sdp = std::string(writer.write(jmessage));
    std::cout << "conductor_boliving.cc 185 lines sdp:\r\n" << sdp << std::endl;
    if(!client_->SendToPeer(sdp)) {
        std::cout << "conductor_boliving.cc OnSuccess() SendToPeer fail" << std::endl;
    } else {
        std::cout << "conductor_boliving.cc OnSuccess() SendToPeer success" << std::endl;
    }
}

void Conductor::OnFailure(webrtc::RTCError error) {
    std::cout << "conductor_boliving.cc 120 lines "<< ToString(error.type()) << ": " << error.message() << std::endl;
}

void Conductor::AddTracks() {
  std::cout << "conductor_boliving.cc AddTracks() start AddTracks" << std::endl;

  bool aec_enabled, agc_enabled, ns_enabled;
  aec_enabled = owt::base::GlobalConfiguration::GetAECEnabled();
  agc_enabled =  owt::base::GlobalConfiguration::GetAGCEnabled();
  ns_enabled =  owt::base::GlobalConfiguration::GetNSEnabled();
  if (!aec_enabled || !agc_enabled || !ns_enabled) {
    cricket::AudioOptions options;
    options.echo_cancellation =
        absl::optional<bool>(aec_enabled ? true : false);
    options.auto_gain_control =
        absl::optional<bool>(agc_enabled ? true : false);
    options.noise_suppression = absl::optional<bool>(ns_enabled ? true : false);
    options.residual_echo_detector =
        absl::optional<bool>(aec_enabled ? true : false);
     std::string audio_track_id("AudioTrack-" + rtc::CreateRandomUuid());
      rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
     peer_connection_factory_->CreateAudioTrack(
         audio_track_id, peer_connection_factory_->CreateAudioSource(options)));
    if(!audio_track) {
      std::cout << "conductor_boliving.cc create failed!" << std::endl;
    }
    auto result_or_error = peer_connection_->AddTrack(audio_track, {kStreamId});

    if (!result_or_error.ok()) {
      RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
                        << result_or_error.error().message();
    }
  }

  std::unique_ptr<CFileFrameGenerator> framer(new CFileFrameGenerator(640, 380, 25, 
    "/home/jack-ma/webrtc_m76/src/culture_640x380_1M.yuv"));
    framer->SetPublishTimeFile("/home/jack-ma/QoSTestFramework/analysis/dataset/Data/localPublishTime.txt");
    std::shared_ptr<owt::base::LocalCustomizedStreamParameters> 
    lcsp(new owt::base::LocalCustomizedStreamParameters(true, true));
    std::string media_stream_id("MediaStream-" + rtc::CreateRandomUuid());
    rtc::scoped_refptr<owt::base::LocalRawCaptureTrackSource> video_device =
        owt::base::LocalRawCaptureTrackSource::Create(lcsp, std::move(framer));

  if (video_device) {  //CreateVideoTrack()是class PeerConnectionFactory的实现方法
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
        peer_connection_factory_->CreateVideoTrack(media_stream_id, video_device));
   // main_wnd_->StartLocalRenderer(video_track_);

    auto result_or_error = peer_connection_->AddTrack(video_track_, {kStreamId});   //添加视频轨
    if (!result_or_error.ok()) {
      RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
                        << result_or_error.error().message();
    }
  } else {
    RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
  }
}

void Conductor::OnMessageFromPeer(const std::string& message) {
  Json::Reader reader;
  Json::Value jmessage;
  if (!reader.parse(message, jmessage)) {
      std::cout << "conductor_boliving.cc OnMessageFromPeer() Received unknown message. "
          << message << std::endl;
    return;
  }
  std::string type_str;
  std::string json_object;

  // kSessionDescriptionTypeName[] = "type"
  rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type_str);
  if (!type_str.empty()) {  // 这个解析的是answer
    absl::optional<webrtc::SdpType> type_maybe = webrtc::SdpTypeFromString(type_str);
    if (!type_maybe) {
      RTC_LOG(LS_ERROR) << "Unknown SDP type: " << type_str;
      return;
    }
    webrtc::SdpType type = *type_maybe;
    std::string sdp;
    // kSessionDescriptionSdpName[] = "sdp";
    if (!rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName,
                                      &sdp)) {
      RTC_LOG(WARNING) << "Can't parse received session description message.";
      return;
    }
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
        webrtc::CreateSessionDescription(type, sdp, &error);
    if (!session_description) {
        std::cout << "conductor_boliving.cc OnMessageFromPeer Can't parse received session description message. "
                       << "SdpParseError was: " << error.description << std::endl;
      return;
    }
    std::cout << " Received session description :" << message << std::endl;
    peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(),
        session_description.release());
    RTC_LOG(INFO) << "SetRemoteDescription success";
    if (type == webrtc::SdpType::kOffer) {
        peer_connection_->CreateAnswer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        std::cout << " conductor_boliving.cc peer_connection_->CreateAnswer" << std::endl;
    }
  } else {  // 这个解析的是candidate
    std::string sdp_mid;
    int sdp_mlineindex = 0;
    std::string sdp;
    if (!rtc::GetStringFromJsonObject(jmessage, kCandidateSdpMidName,
                                      &sdp_mid) ||
        !rtc::GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
                                   &sdp_mlineindex) ||
        !rtc::GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
      RTC_LOG(WARNING) << "Can't parse received message.";
      return;
    }
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));
    if (!candidate.get()) {
      RTC_LOG(WARNING) << "Can't parse received candidate message. "
                       << "SdpParseError was: " << error.description;
      return;
    }
    if (!peer_connection_->AddIceCandidate(candidate.get())) {
      RTC_LOG(WARNING) << "Failed to apply the received candidate";
      return;
    }
    RTC_LOG(INFO) << " Received candidate :" << message;
  }
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  RTC_LOG(INFO) << "OnIceCandidate excute";
  Json::StyledWriter writer;
  Json::Value jmessage;

  jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
  jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
  std::string sdp;
  if (!candidate->ToString(&sdp)) {
    RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }
  jmessage[kCandidateSdpName] = sdp;
  if(!client_->SendToPeer(writer.write(jmessage))) {
        std::cout << "conductor_boliving.cc OnSuccess() SendToPeer fail" << std::endl;
    } else {
        std::cout << "conductor_boliving.cc OnSuccess() SendToPeer success" << std::endl;
    }
}

void Conductor::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&streams) {
    std::cout << "conductor_boliving.cc OnAddTrack()" << std::endl;
    auto* track = reinterpret_cast<webrtc::MediaStreamTrackInterface*>(receiver->track().release());
    if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
        std::thread th1(&Conductor::GetStatus, this);
        th1.detach();
        //GetStatus();
        auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
        videorenderer_.reset(new VideoRenderer(video_track)); 
      }
  RTC_LOG(INFO) << __FUNCTION__ << " " << receiver->id() << "OnAddTrack";
}



void Conductor::SendMessage(const std::string& json_object) {

}

void Conductor::GetStatus() {
  while(true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::thread([&]{
      GetConnectionStats(
        [=](std::shared_ptr<owt::base::ConnectionStats> stats) {
          if((stats.get() != NULL) && (stats->video_receiver_reports.size() > 0)) {
            if(!localFps) {
              std::cout << "localBitrate no open" << std::endl;
            }
            fprintf(localBitrate, ",%ld", stats->video_receiver_reports[0]->bytes_rcvd);
            fflush(localBitrate);
            fprintf(localFps, ",%d", stats->video_receiver_reports[0]->framerate_output);
            fflush(localFps);
          }
        }
      );
    }).detach();
  }
}

void Conductor::GetConnectionStats(std::function<void(std::shared_ptr<owt::base::ConnectionStats>)> on_sucess) {
  if(on_sucess == nullptr) {
    std::cout << "on_sucess cannot be nullptr. Please provide on_sucess to get connection stats data" << std::endl;
    return;
  } else {
        rtc::scoped_refptr<owt::base::FunctionalStatsObserver> observer = 
            owt::base::FunctionalStatsObserver::Create(on_sucess);
        std::cout << "boliving_conductor.cc 397 lines GetConnectionStats()" << std::endl;
        peer_connection_->GetStats(observer, nullptr, 
        webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
      }
}



