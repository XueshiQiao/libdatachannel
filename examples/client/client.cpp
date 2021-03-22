//
// Created by Joey on 2021/2/24.
//

#include "client.h"
#include "rtc/rtc.hpp"
#include <functional>

using namespace rtc;
using namespace std;
using json = nlohmann::json;

namespace sample {

Client::Client(std::string &id, std::string& wsURL, std::unique_ptr<rtc::Configuration> configuration, ClientRole role): id(id), wsURL(wsURL), configuration(std::move(configuration)), myRole(role) {
    createWebSocket([=]() {
	});
}

void Client::createWebSocket(const std::function<void(void)> callback) {
    ws = std::make_shared<rtc::WebSocket>();//std::make_optional(configuration.get())
    std::promise<void> promise;
    auto future = promise.get_future();

    ws->onOpen([=, &promise] {
      std::cout << "ws did open" << std::endl;
	  json join = {
          {"join", id}
	  };
	  ws->send(join.dump());
      promise.set_value();
    });

    ws->onError([&promise](std::string s) {
      std::cout << "ws encounter an error:" << s  << std::endl;
      promise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    ws->onClosed([] {
      std::cout << "ws did close" << std::endl;
    });

    ws->onMessage([&](variant<binary, string> data){
      if (!holds_alternative<string>(data)) {
          std::cout << "receive an unaccpetd message" << std::endl;
          return;
      }
      json message = json::parse(get<string>(data));
      std::string remoteId = message["id"];
      std::string type = message["type"];
      if (std::empty(remoteId) || std::empty(type)) {
          std::cout << "remoteId or type is empty, return" << std::endl;
          return;
      }
	  std::shared_ptr<rtc::PeerConnection> pc;
	  if (auto pair = pcMap.find(remoteId); pair != pcMap.end()) {
		  pc = pair->second;
	  } else if (type == "offer") {
		  //do sth
		  std::cout << "receiver msg: " << type << std::endl;
		  pc = createPeerConnection(remoteId, ws);
		  pcMap[remoteId] = pc;
      } else {
		  return;
	  }
	  if (type == "answer") {
		  auto sdp = message["description"].get<std::string>();
          std::cout << "receive answer:" << sdp << std::endl;
		  pc->setRemoteDescription(Description(sdp, type));
	  } else if (type == "candidate") {
		  auto candidate = message["candidate"].get<std::string>();
          std::cout << "receive sdp:" << candidate << std::endl;
		  auto mid = message["mid"].get<std::string>();
		  pc->addRemoteCandidate(Candidate(candidate, mid));
	  }
    });

	ws->open(wsURL + "/" + id);

	future.get();
	callback();
	if (myRole == Client::ClientRoleSender) {
		while (true) {
            std::string remoteId;
            std::cout << "Enter the remoteid:" << std::endl;
            std::cin >> remoteId;
			cin.ignore();

            auto pc = createPeerConnection(remoteId, ws);
            pcMap[remoteId] = pc;

            std::string dcLabel = id+"<->"+remoteId;
            auto dc = pc->createDataChannel(dcLabel);
            dc->onOpen([&, wdc = std::weak_ptr<rtc::DataChannel>(dc)]() {
              cout << "data channel to " << remoteId << " open " << std::endl;
              if (auto dc = wdc.lock()) {
                  dc->send("hello, i am " + id + ".");
              }
            });
            dc->onClosed([&]() {
              std::cout << "datachannel to " << remoteId << " is closed" << std::endl;
            });
            dc->onMessage([&, wdc = std::weak_ptr<rtc::DataChannel>(dc)](std::variant<binary, string> data) {
              if (std::holds_alternative<string>(data)) {
                  std::string content = std::get<string>(data);
                  std::cout << "message from " << remoteId << " received: " << content << std::endl;
				  if (content == "welcome") {
					  if (auto dc = wdc.lock()) {
						  dc->send("welcome, too");
					  }
				  }
              } else {
                  std::cout << "Binary message from " << id
                            << " received, size=" << std::get<binary>(data).size() << std::endl;
              }
            });
            dcMap.emplace(remoteId, dc);
		}
	}
}
std::shared_ptr<rtc::PeerConnection> Client::createPeerConnection(const std::string& remoteId, std::weak_ptr<rtc::WebSocket> ws) {
	std::cout << "create pc" << std::endl;
    auto pc = std::make_shared<rtc::PeerConnection>(*configuration.get());
    pc->onStateChange([](rtc::PeerConnection::State state) {
      std::cout << "PC State: " << state << std::endl;
    });
    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
      std::cout << "PC Gathering State: " << state << std::endl;
    });
	pc->onLocalDescription([&](Description desc) {
      std::cout << "on local desp" << std::endl;
		json dsp = {
            {"id", remoteId},
            {"type", desc.typeString()},
            {"description", string(desc)}
		};
		if (auto websocket = ws.lock()) {
			websocket->send(dsp.dump());
		}
	});
	pc->onLocalCandidate([&](Candidate candidate) {
      std::cout << "on local candidate" << std::endl;
        json message = {
            {"id", remoteId},
            {"candidate", string(candidate)},
            {"mid", candidate.mid()}
		};
	    if (auto websocket = ws.lock()) {
			websocket->send(message.dump());
		}
	});
	pc->onDataChannel([&](std::shared_ptr<DataChannel> dc) {

        dc->onMessage([&](variant<binary, string> data) {
			if (holds_alternative<std::string>(data)) {
				cout << "message from " << remoteId << ": " << get<string>(data) << std::endl;
			} else {
                cout << "binary message from " << remoteId << ", size: " << get<binary>(data).size() << std::endl;
			}
		});
		dc->send("hello from " + id);
		dcMap.emplace(remoteId, dc);
	});
	return pc;
}

}



//int main(int argc, char** argv) {
//
//    return 0;
//}