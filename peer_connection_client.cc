/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "peer_connection_client.h"

#include "examples/peerconnection/client/linux/defaults.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/net_helpers.h"

#ifdef WIN32
#include "rtc_base/win32_socket_server.h"
#endif

namespace {

// This is our magical hangup signal.
// Delay between server connection retries, in milliseconds

//rtc::AsyncSocket* CreateClientSocket(int family) {
//#ifdef WIN32
//  rtc::Win32Socket* sock = new rtc::Win32Socket();
//  sock->CreateT(family, SOCK_STREAM);
//  return sock;
//#elif defined(WEBRTC_POSIX)
//  rtc::Thread* thread = rtc::Thread::Current();
//  RTC_DCHECK(thread != NULL);
//  std::cout << "CreateClientSocket family=" << family << std::endl;
//  auto temp = thread->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);
//  if(temp == NULL) {
//      std::cout << "haha CreateClientSocket temp == NULL" << std::endl;
//  }
//  return temp;
////  return thread->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);  // 这个调用的是SsSocketFactory类的方法
//#else
//#error Platform not supported.
//#endif
//}

}  // namespace

PeerConnectionClient::PeerConnectionClient()
    : callback_(NULL), resolver_(NULL), state_(NOT_CONNECTED){}

PeerConnectionClient::~PeerConnectionClient() {}

//void PeerConnectionClient::InitSocketSignals() {
//  RTC_DCHECK(control_socket_.get() != NULL);
//  control_socket_->SignalCloseEvent.connect(this,
//                                            &PeerConnectionClient::OnClose);
//  control_socket_->SignalConnectEvent.connect(this,
//                                              &PeerConnectionClient::OnConnect);
//  control_socket_->SignalReadEvent.connect(this, &PeerConnectionClient::OnRead);
//}

void PeerConnectionClient::RegisterObserver(PeerConnectionClientObserver* callback) {
  RTC_DCHECK(!callback_);
  callback_ = callback;
}

void PeerConnectionClient::Connect(const std::string& server,int port) {
  if (state_ != NOT_CONNECTED) {
      std::cout << "peer_connection.cc 82 The client must not be connected before you can call Connect()" << std::endl;
      return;
  }

  if (port <= 0)
    port = kDefaultServerPort;   //kDefaultServerPort = 8885

  server_address_.SetIP(server);
  server_address_.SetPort(port);
}

bool PeerConnectionClient::SendToPeer(const std::string& message) {
//  control_socket_.reset(CreateClientSocket(server_address_.ipaddr().family()));
//  if(control_socket_.get() == NULL) {
//      std::cout << "CreateClientSocket failed" << std::endl;
//  }
//  InitSocketSignals();
//  if (state_ != CONNECTED)
//    return false;

  char headers[1024];
  snprintf(headers, sizeof(headers),
           "POST /live/rtc1.sdp HTTP/1.0\r\n"
           "Content-Length: %zu\r\n"
           "Content-Type: application/json\r\n"
           "\r\n",message.length());
  onconnect_data_ = headers;
  onconnect_data_ += message;
  std::cout << "peer_connection_client.cc 106 lines onconnect_data_:\r\n" << onconnect_data_ << std::endl;
  return ConnectControlSocket();
}

bool PeerConnectionClient::IsSendingMessage() {
  return state_ == CONNECTED &&
         control_socket_->GetState() != rtc::Socket::CS_CLOSED;
}

void PeerConnectionClient::Close() {
  control_socket_->Close();
  onconnect_data_.clear();
  state_ = NOT_CONNECTED;
}

bool PeerConnectionClient::ConnectControlSocket() {
    int ret, n;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("52.80.133.9");
    servaddr.sin_port = htons(8081);

    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        std::cout << "socket create failed" << std::endl;
    }

    ret = connect(m_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0) {
        std::cout << "connection failed" << std::endl;
    }

    cliaddrlen = sizeof(cliaddr);
    ret = getsockname(m_sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
    if (ret < 0)
        std::cout << "getsockaddr failed" << std::endl;
    std::cout << "cliaddr:" << inet_ntoa(cliaddr.sin_addr) << " " <<ntohs(cliaddr.sin_port) << std::endl;
    n = Send((char*)onconnect_data_.c_str(),onconnect_data_.length());
    if(n <=0) {
      std::cout << "Send failed" << std::endl;
      return false;
    }
    OnRead(m_sockfd);
    return true;
}

 int PeerConnectionClient::Send(char *data, int length) {
   return write(m_sockfd, (char*)onconnect_data_.c_str(),onconnect_data_.length());
 }

void PeerConnectionClient::OnConnect(rtc::AsyncSocket* socket) {
  RTC_DCHECK(!onconnect_data_.empty());
  size_t sent = socket->Send(onconnect_data_.c_str(), onconnect_data_.length());
  RTC_DCHECK(sent == onconnect_data_.length());
  onconnect_data_.clear();
}

bool PeerConnectionClient::GetHeaderValue(const std::string& data,
                                          size_t eoh,
                                          const char* header_pattern,
                                          size_t* value) {
  RTC_DCHECK(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    *value = atoi(&data[found + strlen(header_pattern)]);   //计算response的内容长度
    return true;
  }
  return false;
}

bool PeerConnectionClient::GetHeaderValue(const std::string& data,
                                          size_t eoh,
                                          const char* header_pattern,
                                          std::string* value) {
  RTC_DCHECK(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    size_t begin = found + strlen(header_pattern);
    size_t end = data.find("\r\n", begin);
    if (end == std::string::npos)
      end = eoh;
    value->assign(data.substr(begin, end - begin));
    return true;
  }
  return false;
}

bool PeerConnectionClient::ReadIntoBuffer(int sockfd,
                                          std::string* data) {
  char buffer[0xffff];
  do {
//    int bytes = socket->Recv(buffer, sizeof(buffer), nullptr);  //这个调用的是sslsocketfactory类的Recv方法
    int bytes = read(sockfd, buffer, sizeof(buffer));
    if (bytes <= 0)
      break;
    data->append(buffer, bytes);
  } while (true);

  bool ret = false;

  size_t i = data->find("\r\n\r\n");
  if(i != std::string::npos){
      ret = true;
  }

  return ret;
}

void PeerConnectionClient::OnRead(int sockfd) {
  RTC_LOG(INFO) << __FUNCTION__;
  std::cout << "peer_connection_client.cc PeerConnectionClient::OnRead" << std::endl;
  if(ReadIntoBuffer(sockfd, &notification_data_)){
    std::cout << "peer_connection_client.cc 219 lines notification_data_=\r\n" << notification_data_ << std::endl;
    size_t eoh = 0;
    bool ok = ParseServerResponse(notification_data_, &eoh);
    if (ok) {
        size_t pos = eoh + 4;
        callback_->OnMessageFromPeer(notification_data_.substr(pos));
    }
  } else {
     std::cout << "ReadIntoBuffer faild" << std::endl;
    }
     notification_data_.clear();
}

bool PeerConnectionClient::ParseEntry(const std::string& entry,
                                      std::string* name,
                                      int* id,
                                      bool* connected) {
  RTC_DCHECK(name != NULL);
  RTC_DCHECK(id != NULL);
  RTC_DCHECK(connected != NULL);
  RTC_DCHECK(!entry.empty());

  *connected = false;
  size_t separator = entry.find(',');
  if (separator != std::string::npos) {
    *id = atoi(&entry[separator + 1]);
    name->assign(entry.substr(0, separator));
    separator = entry.find(',', separator + 1);
    if (separator != std::string::npos) {
      *connected = atoi(&entry[separator + 1]) ? true : false;
    }
  }
  return !name->empty();
}

int PeerConnectionClient::GetResponseStatus(const std::string& response) {
  int status = -1;
  size_t pos = response.find(' ');
  if (pos != std::string::npos)
    status = atoi(&response[pos + 1]);
  return status;
}

bool PeerConnectionClient::ParseServerResponse(const std::string& response,
                                               size_t* eoh) {
  int status = GetResponseStatus(response.c_str());
  if (status == 202 || status == 200) {
     
      std::cout << "Respose Status=OK" << std::endl;
     // Close();
 //     return false;
  } else {
       std::cout << "peer_connection_client.cc 361 lines Received error from server" << std::endl;
  }

  *eoh = response.find("\r\n\r\n");
  if (*eoh == std::string::npos)
    return false;

  // See comment in peer_channel.cc for why we use the Pragma header and
  // not e.g. "X-Peer-Id".
  //GetHeaderValue(response, *eoh, "\r\nPragma: ", peer_id);

  return true;
}

void PeerConnectionClient::OnClose(rtc::AsyncSocket* socket, int err) {
  RTC_LOG(INFO) << __FUNCTION__;

  socket->Close();
  std::cout << "peer_connection_client.cc 415 lines err=" << err << std::endl;

}

void PeerConnectionClient::OnMessage(rtc::Message* msg) {
  // ignore msg; there is currently only one supported message ("retry")
    std::cout << "DoConnect();" << std::endl;
}
