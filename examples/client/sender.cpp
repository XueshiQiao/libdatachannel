//
// Created by Joey on 2021/3/21.
//
#include "client.h"
#include "rtc/rtc.hpp"
#include <cstdio>

#include "parse_cl.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <string>

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

int main(int argc, char **argv) {
	std::string id = "joey";
	std::string ws = "ws://localhost:8000";
    auto config = std::make_unique<rtc::Configuration>();
	config->iceServers.push_back(rtc::IceServer("stun://stun.l.google.com:19302"));
	sample::Client client(id, ws, std::move(config), sample::Client::ClientRoleSender);



	return 0;
}