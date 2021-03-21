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
    Client(std::string& id, std::unique_ptr<Configuration> configuration): id(id), configuration(std::move(configuration));
    void sendFile(std::string& filePath) {}
    void onReceiveFile() {}

protected:
	void createWebSocket();
	void createPeerConnection();
private:

    std::string id;
    std::unique_ptr<Configuration> configuration;
	std::shared_ptr<WebSocket> ws;
//    std::shared_ptr<rtc::PeerConnection> pc;
    std::map<std::string, std::shared_ptr<rtc::PeerConnection> pcMap;
    std::map<std::string, std::shared_ptr<rtc::DataChannel> dcMap;
};
}

#endif // LIBDATACHANNEL_RECEIVER_H
