//
// Created by Joey on 2021/2/24.
//

#ifndef LIBDATACHANNEL_RECEIVER_H
#define LIBDATACHANNEL_RECEIVER_H
#include "rtc/rtc.hpp"
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>

namespace sample {
class Client {
public:
	enum ClientRole {
		ClientRoleSender = 0,
		ClientRoleReceiver = 1
	};
    Client(std::string& id, std::string& wsURL, std::unique_ptr<rtc::Configuration> configuration, ClientRole role = ClientRoleReceiver);
    void sendFile(std::string& filePath) {}
    void onReceiveFile() {}

protected:
	void createWebSocket(const std::function<void(void)> callback);
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(const std::string& remoteId, std::weak_ptr<rtc::WebSocket> ws);
private:

	ClientRole myRole{ClientRoleReceiver};
    std::string id;
	std::string wsURL;
    std::unique_ptr<rtc::Configuration> configuration;
	std::shared_ptr<rtc::WebSocket> ws;
//    std::shared_ptr<rtc::PeerConnection> pc;
    std::map<std::string, std::shared_ptr<rtc::PeerConnection>> pcMap;
    std::map<std::string, std::shared_ptr<rtc::DataChannel>> dcMap;
};
}

#endif // LIBDATACHANNEL_RECEIVER_H
