#include <iostream>
#include <string>
#include <pthread.h>
#include<unistd.h>

#include "conductor_boliving.h"
#include "peer_connection_client.h"
#include "/home/jack-ma/webrtc_m76/src/rtc_base/ref_counted_object.h"
#include "/home/jack-ma/webrtc_m76/src/api/scoped_refptr.h"
#include "/home/jack-ma/webrtc_m76/src/rtc_base/ssl_adapter.h"

void *run(void *arg) {
    PeerConnectionClient client;
    rtc::scoped_refptr<Conductor> conductor(new rtc::RefCountedObject<Conductor>(&client));
    conductor->ConnectToPeer(false);
    std::cout << "run return" << std::endl;
    getchar();
    return ((void *)0);
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, run, NULL);
 //   sleep(1);
    rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
    rtc::LogMessage::ConfigureLogging("tstamp thread info debug");
    PeerConnectionClient client;
//    rtc::scoped_refptr<Conductor> conductor(new rtc::RefCountedObject<Conductor>(&client));
    rtc::scoped_refptr<Conductor> conductor(new rtc::RefCountedObject<Conductor>(&client));

    conductor->ConnectToPeer(true);
    rtc::InitializeSSL();

    
    getchar();
    rtc::CleanupSSL();
    return 0;

}
