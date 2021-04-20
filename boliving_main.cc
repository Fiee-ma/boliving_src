#include <iostream>
#include <string>

#include "conductor_boliving.h"
#include "peer_connection_client.h"
#include "/home/jack-ma/webrtc_m76/src/rtc_base/ref_counted_object.h"
#include "/home/jack-ma/webrtc_m76/src/api/scoped_refptr.h"
#include "/home/jack-ma/webrtc_m76/src/rtc_base/ssl_adapter.h"

int main() {
    rtc::InitializeSSL();

    PeerConnectionClient client;
    rtc::scoped_refptr<Conductor> conductor(new rtc::RefCountedObject<Conductor>(&client));

    std::string server = "172.0.0.1";
    int port = 1554;
    conductor->Connect(server, port);
    conductor->client_->DoConnect();

    conductor->
}
