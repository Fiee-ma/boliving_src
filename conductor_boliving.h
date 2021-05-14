#ifndef PEERCONNECTION_CLIENT_CONDUCTOR_BOLIVING_H__
#define PEERCONNECTION_CLIENT_CONDUCTOR_BOLIVING_H__

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "/home/jack-ma/webrtc_m76/src/api/media_stream_interface.h"
#include "/home/jack-ma/webrtc_m76/src/api/scoped_refptr.h"
#include "/home/jack-ma/webrtc_m76/src/api/peer_connection_interface.h"
#include "/home/jack-ma/webrtc_m76/src/rtc_base/ref_counted_object.h"
#include "/home/jack-ma/webrtc_m76/src/api/jsep.h"
#include "/home/jack-ma/webrtc_m76/src/api/rtc_error.h"
#include "/home/jack-ma/webrtc_m76/src/rtc_base/system/rtc_export.h"
#include "peer_connection_client.h"

#include "modules/sdk/base/customizedframescapturer.h"
#include "modules/sdk/base/customizedvideoencoderproxy.h"
#include "modules/sdk/include/cpp/owt/base/localcamerastreamparameters.h"
#include "examples/peerconnection/client/linux/fileframegenerator.h"
#include "modules/sdk/base/customizedvideosource.h"
#include "modules/sdk/include/cpp/owt/base/commontypes.h"
#include "modules/sdk/base/customizedvideosource.h"
#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "modules/sdk/include/cpp/owt/base/globalconfiguration.h"
#include "remotevideorenderer.h"
#include "modules/sdk/base/functionalobserver.h"

class Conductor : public webrtc::PeerConnectionObserver,
                  public webrtc::CreateSessionDescriptionObserver,
                  public PeerConnectionClientObserver {
public:
    Conductor(PeerConnectionClient *client);
    bool InitializePeerConnection(bool flag);
  // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
    bool CreatePeerConnection(bool dtls);
    void ConnectToPeer(bool flag);
    void SendMessage(const std::string& json_object);
    void AddTracks();
    void OnMessageFromPeer(const std::string& message) override;
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {}
  void OnRemoveTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {}
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnSignedIn() override {}

  void OnDisconnected() override {}

  void OnPeerConnected(int id, const std::string& name) override {}

  void OnPeerDisconnected(int id) override {}

  void OnIceConnectionReceivingChange(bool receiving) override {}

  void OnMessageSent(int err) override {}

  void OnServerConnectionFailure() override {}

protected:
    void GetConnectionStats(std::function<void(std::shared_ptr<owt::base::ConnectionStats>)> on_sucess);
    void GetStatus();


private:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
    PeerConnectionClient* client_;
    std::unique_ptr<rtc::Thread> network_thread_;
    std::unique_ptr<rtc::Thread> worker_thread_;
    std::unique_ptr<rtc::Thread> signal_thread_;
    std::shared_ptr<VideoRenderer> videorenderer_;
    FILE *localBitrate;
    FILE *localFps;
};

#endif
