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

Client::Client(std::string &id, std::string& wsURL, std::unique_ptr<rtc::Configuration> configuration): id(id), wsURL(wsURL), configuration(std::move(configuration)) {
    createWebSocket([=]() {
		createPeerConnection();
	});
}

void Client::createWebSocket(std::function<void(void)> callback) {
    ws = std::make_shared<rtc::WebSocket>();//std::make_optional(configuration.get())
    std::promise<void> promise;
    auto future = promise.get_future();

    ws->onOpen([&promise] {
      std::cout << "ws did open" << std::endl;
      promise.set_value();
    });

    ws->onError([&promise](std::string s) {
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
	ws->open(wsURL);

	future.get();
	callback();
}

void Client::createPeerConnection() {
    auto pc = std::make_shared<rtc::PeerConnection>(*configuration.get());
    pc->onStateChange([](rtc::PeerConnection::State state) {
      std::cout << "State: " << state << std::endl;
    });
    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
      std::cout << "Gathering State: " << state << std::endl;
    });
}

}



//int main(int argc, char** argv) {
//
//    return 0;
//}