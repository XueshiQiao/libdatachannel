//
// Created by Joey on 2021/2/24.
//

#include "client.h"
#include "peerconnection.hpp"

namespace sample {

Client::Client(std::string &id, std::unique_ptr<Configuration> configuration) {
    createWebSocket([](){
        createPeerConnection();
	});
}

void Client::createWebSocket(std::function<void> callback) {
    ws = std::make_shared<WebSocket>(configuration);
    std::promise<void> promise;
    auto future = promise.get_future();

    ws->onOpen([&promise] {
      std::cout << "ws did open" << std::endl;
      promise.set_value();
    });

    ws->onError([&promise](string s) {
      std::cout << "ws encounter an error:" << s  << std::endl;
      promise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    ws->onClosed([] {
      std::cout << "ws did close" << std::endl;
    });

    ws->onMessage([](variant<binary, string> data){
      if (!holds_alternative<string>(data)) {
          std::cout << "receive an unaccpetd message" << std::endl;
          return;
      }
      json message = json::parse(get<string>(data));
      std::string id = message["id"];
      std::string type = message["type"];
      if (std::empty(id) || std::empty(type)) {
          std::cout << "id or type is empty, return" << std::endl;
          return;
      }
      if (type == "offer" || type == "answer") {
        //do sth
      }
    });
	configuration
	ws->open(configuration->webSocketServer);

	future.get();
	callback();
}

}

void Client::createPeerConnection() {
    auto pc = std::make_shared<rtc::PeerConnection>(configuration.get());
	pc->onStateChange([](PeerConnection::State state) {
		std::cout << "State: " << state << std::endl;
	});
	pc->onGatheringStateChange([](PeerConnection:GatheringState state) {
		std::cout << "Gathering State: " << state << std::endl;
	});
}


//int main(int argc, char** argv) {
//
//    return 0;
//}