//
// Created by joey on 2021/3/23.
//

#ifndef LIBDATACHANNEL_CLIENT_H
#define LIBDATACHANNEL_CLIENT_H

#include <vector>
#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <future>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <string>

#include "filereader.h"
#include "rtc/rtc.hpp"
#include "parse_cl.h"
#include <nlohmann/json.hpp>

using namespace std::chrono_literals;

class Client {
public:
    Client(const std::string& myID, const std::string& websocketURL, std::shared_ptr<rtc::Configuration> configuration);
    void openConnect();
protected:
	void connectWebsocket(std::function<void(void)> completion);
    void handleWSMessage(std::variant<rtc::binary, std::string> data);
    void promptForInput();
    void handleFile(std::weak_ptr<rtc::DataChannel> dc, const std::vector<std::byte>& bytes);
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(const rtc::Configuration &config, std::weak_ptr<rtc::WebSocket> wws, std::string remoteID);
private:
    std::string myID;
    std::string remoteId;
	std::string websocketURL;
	std::shared_ptr<rtc::Configuration> configuration;
	std::shared_ptr<rtc::WebSocket> websocket;
	std::shared_ptr<rtc::PeerConnection> peerConnection;
	std::shared_ptr<rtc::DataChannel> dataChannel;
};

#endif // LIBDATACHANNEL_CLIENT_H
