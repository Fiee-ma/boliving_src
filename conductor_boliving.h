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

class Conductor : public webrtc::PeerConnectionObserver,
                  public webrtc::CreateSessionDescriptionObserver,
                  public PeerConnectionClientObserver {
public:
    Conductor(PeerConnectionClient *client);
    bool InitializePeerConnection();
  // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;
    void Connect(std::string server, int port);
    bool CreatePeerConnection(bool dtls);
    void ConnectToPeer();
    void SendMessage(const std::string& json_object);
    void AddTracks();

private:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
    PeerConnectionClient* client_;
};

#endif
