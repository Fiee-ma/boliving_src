#include <iostream>
#include "conductor_boliving.h"

#include <stddef.h>
#include <stdint.h>
#include <iostream>

#include <memory>
#include <utility>
#include <vector>

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
#include "examples/peerconnection/client/defaults.h"
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

Conductor::Conductor(PeerConnectionClient *client)
    :client_(client) {
        client_->RegisterObserver(this);
}

void Conductor::Connect(std::string server, int port) {
    client_->Connect(server, port);
}

void Conductor::ConnectToPeer() {
    if (InitializePeerConnection()) {
    peer_connection_->CreateOffer(
        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    } else {
        std::cout << "Failed to initialize PeerConnection" << std::endl;
    }
}

bool Conductor::InitializePeerConnection() {
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
        nullptr /* network_thread */, nullptr /* worker_thread */,
        nullptr /* signaling_thread */, nullptr /* default_adm */,
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

    AddTracks();

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

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc){
    peer_connection_->SetLocalDescription(
        DummySetSessionDescriptionObserver::Create(), desc);
    std::cout << "conductor_boliving.cc 102 lines SetLocalDescription" << std::endl;

    std::string sdp1;
    desc->ToString(&sdp1);
    std::cout << "sdp:\r\n" << sdp1 << std::endl;

    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = webrtc::SdpTypeToString(desc->GetType());  //kSessionDescriptionTypeName = "type"
    jmessage[kSessionDescriptionSdpName] = sdp1;  // kSessionDescriptionSdpName = "sdp"
    std::string sdp = std::string(writer.write(jmessage));
    std::cout << "conductor_boliving.cc 113 lines sdp:\r\n" << sdp << std::endl;
    if(!client_->SendToPeer(sdp)) {
        std::cout << "SendToPeer fail" << std::endl;
    }
}

void Conductor::OnFailure(webrtc::RTCError error) {

}

void Conductor::AddTracks() {
  std::cout << "conductor.cc 435lines start AddTracks" << std::endl;

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      peer_connection_factory_->CreateAudioTrack(
          kAudioLabel, peer_connection_factory_->CreateAudioSource(
                           cricket::AudioOptions())));
  auto result_or_error = peer_connection_->AddTrack(audio_track, {kStreamId});   //添加音频轨
  if (!result_or_error.ok()) {
      std::cout << "Failed to add audio track to PeerConnection: "
                      << result_or_error.error().message() << std::endl;
  }

  rtc::scoped_refptr<CapturerTrackSource> video_device =
      CapturerTrackSource::Create();
  if (video_device) {
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
        peer_connection_factory_->CreateVideoTrack(kVideoLabel, video_device));
    main_wnd_->StartLocalRenderer(video_track_);

    result_or_error = peer_connection_->AddTrack(video_track_, {kStreamId});   //添加视频轨
    if (!result_or_error.ok()) {
        std::cout << "Failed to add video track to PeerConnection: "
                        << result_or_error.error().message() << std::endl;
    }
  } else {
      std::cout << "OpenVideoCaptureDevice failed" << std::endl;
  }
}

void Conductor::SendMessage(const std::string& json_object) {

}



