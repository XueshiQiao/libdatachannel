/*
 * libdatachannel client example
 * Copyright (c) 2019-2020 Paul-Louis Ageneau
 * Copyright (c) 2019 Murat Dogan
 * Copyright (c) 2020 Will Munn
 * Copyright (c) 2020 Nico Chatzi
 * Copyright (c) 2020 Lara Mackey
 * Copyright (c) 2020 Erik Cota-Robles
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "rtc/rtc.hpp"

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
#include "filereader.h"

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

#include "Client.h"

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

unordered_map<string, shared_ptr<PeerConnection>> peerConnectionMap;
unordered_map<string, shared_ptr<DataChannel>> dataChannelMap;

string localId;

shared_ptr<PeerConnection> createPeerConnection(const Configuration &config,
                                                weak_ptr<WebSocket> wws, string id);
string randomId(size_t length);

int main(int argc, char **argv) try {
	Cmdline params(argc, argv);

//	std::string src = "/Users/joey/Code/libdatachannel/examples/client/filereader.h";
//	std::string dest = "/Users/joey/Code/libdatachannel/examples/client/file";
//	readFile(src, [&](const std::vector<char>& buffer) {
//		appendFile(dest, buffer);
//	});
//
//	std::vector<char> content = readFile("/Volumes/SSD/Code/libdatachannel/examples/client/filereader.h");
//    saveFile("/Volumes/SSD/Code/libdatachannel/examples/client/a.log", content);
//    std::vector<char> content2 = readFile("/Volumes/SSD/Code/libdatachannel/examples/client/a.log");
//
//	assert(content.size() == content2.size());
	rtc::InitLogger(LogLevel::Info);

	Configuration config;

	//joey
	config.iceServers.emplace_back(rtc::IceServer("stun:47.95.213.21:3478"));

	localId = randomId(4);
	cout << "The local ID is: " << localId << endl;

	std::string websocketURL = "ws://127.0.0.1:8000/" + localId;
	auto client = std::make_shared<Client>(localId, websocketURL, std::make_shared<rtc::Configuration>(config));
	std::thread t([&]() {
        client->openConnect();
		if (params.noStun()) { //use for temporary
            client->onDataChannelConnected([&](std::shared_ptr<rtc::DataChannel> dataChannel){
              std::string file{"/Volumes/SSD/Downloads/macupdater_latest.dmg"};
              std::cout << "will send file " << file << std::endl;
              client->sendFile(file);
            });
		}
	});

	t.join();

	while(true){
		pthread_yield_np();
	}
	return 0;

} catch (const std::exception &e) {
	std::cout << "Error: " << e.what() << std::endl;
	return -1;
}

// Helper function to generate a random ID
string randomId(size_t length) {
	static const string characters(
	    "0123456789"); //ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	string id(length, '0');
	default_random_engine rng(random_device{}());
	uniform_int_distribution<int> dist(0, int(characters.size() - 1));
	generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
	return id;
}

