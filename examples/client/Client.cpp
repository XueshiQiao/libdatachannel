//
// Created by joey on 2021/3/23.
//

#include "Client.h"
#include <variant>

using namespace std;
using namespace rtc;
using json = nlohmann::json;

template <class T>
weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { 
	return ptr; 
}

Client::Client(const std::string& myID,
               const std::string& websocketURL,
               std::shared_ptr<rtc::Configuration> configuration):
               websocketURL(websocketURL), configuration(configuration), myID(myID), remoteId(""), websocket(std::make_shared<rtc::WebSocket>()), peerConnection(nullptr), dataChannel(nullptr) {
}

void Client::openConnect() {
    connectWebsocket([&](){
      promptForInput();
    });
}
void Client::connectWebsocket(function<void(void)> completion) {
    std::promise<void> wsPromise;
	auto wsFuture = wsPromise.get_future();
	websocket->onOpen([&wsPromise](){
      std::cout << "websocket connected, signaling ready" << std::endl;
	  wsPromise.set_value();
    });

	websocket->onError([&wsPromise](std::string error){
      std::cout << "websocket error:" << error << std::endl;
	  wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(error)));
    });

	websocket->onClosed([]() {
      std::cout << "websocket closed" << std::endl;
    });

	websocket->onMessage([&](std::variant<rtc::binary, std::string> data) {
		handleWSMessage(data);
	});

	websocket->open(websocketURL);
	wsFuture.get();
    completion();
}

void Client::handleWSMessage(std::variant<rtc::binary, std::string> data) {
    if (!holds_alternative<string>(data))
        return;

    json message = json::parse(get<string>(data));

    auto it = message.find("id");
    if (it == message.end())
        return;
    string id = it->get<string>();

    it = message.find("type");
    if (it == message.end())
        return;
    string type = it->get<string>();

	if (peerConnection == nullptr) {
	    if (type == "offer") {
            cout << "Answering to " + id << endl;
            peerConnection = createPeerConnection(*configuration.get(), websocket, id);
		} else {
			return ;
		}
	}

    cout << "receive " << type << " from " << id << std::endl;
    if (type == "offer" || type == "answer") {
        auto sdp = message["description"].get<string>();
        peerConnection->setRemoteDescription(Description(sdp, type));
    } else if (type == "candidate") {
        auto sdp = message["candidate"].get<string>();
        auto mid = message["mid"].get<string>();
        peerConnection->addRemoteCandidate(Candidate(sdp, mid));
    }
}

void Client::promptForInput() {
    string remoteID;
    cout << "Enter a remote ID to send an offer:" << endl;
    cin >> remoteID;
    cin.ignore();
    if (remoteID.empty())
        return;
    if (remoteID == myID)
        return;

    cout << "Offering to " + remoteID << endl;
	auto config = configuration.get();
    peerConnection = createPeerConnection(*config, websocket, remoteID);

    // We are the offerer, so create a data channel to initiate the process
    const string label = "test";
    cout << "Creating DataChannel with label \"" << label << "\"" << endl;
    dataChannel = peerConnection->createDataChannel(label);

    dataChannel->onOpen([&, remoteID, wdc = make_weak_ptr(dataChannel)]() {
      cout << "DataChannel from " << remoteID << " open" << endl;
      if (auto dc = wdc.lock())
          dc->send("Hello from " + myID);
    });

    dataChannel->onClosed([remoteID]() { cout << "DataChannel from " << remoteID << " closed" << endl; });

    dataChannel->onMessage([&, remoteID, wdc = make_weak_ptr(dataChannel)](variant<binary, string> data) {
      if (holds_alternative<string>(data))
          cout << "Message from " << remoteID << " received: " << get<string>(data) << endl;
      else {
          cout << "1Binary message from " << remoteID
			   << " received, size=" << get<binary>(data).size() << endl;
          handleFile(wdc, get<binary>(data));
      }
    });
}

// Create and setup a PeerConnection
std::shared_ptr<rtc::PeerConnection> Client::createPeerConnection(const rtc::Configuration &config, std::weak_ptr<rtc::WebSocket> wws, std::string remoteID) {
    auto pc = make_shared<PeerConnection>(config);

    pc->onStateChange([](PeerConnection::State state) { cout << "State: " << state << endl; });

    pc->onGatheringStateChange(
        [](PeerConnection::GatheringState state) { cout << "Gathering State: " << state << endl; });

    pc->onLocalDescription([wws, remoteID](Description description) {
      cout << "onLocalDescription " << std::endl;
      json message = {
          {"id", remoteID}, {"type", description.typeString()}, {"description", string(description)}};

      if (auto ws = wws.lock())
          ws->send(message.dump());
    });

    pc->onLocalCandidate([wws, remoteID](Candidate candidate) {
      cout << "onLocalCandidate " << std::endl;
      json message = {{"id", remoteID},
                      {"type", "candidate"},
                      {"candidate", string(candidate)},
                      {"mid", candidate.mid()}};

      if (auto ws = wws.lock())
          ws->send(message.dump());
    });

    pc->onDataChannel([&, remoteID](shared_ptr<DataChannel> dc) {
		dataChannel = dc;
      cout << "DataChannel from " << remoteID << " received with label \"" << dc->label() << "\""
           << endl;

      dc->onClosed([remoteID]() { cout << "DataChannel from " << remoteID << " closed" << endl; });

      dc->onMessage([&, remoteID, wdc = make_weak_ptr(dc)](variant<binary, string> data) {
        if (holds_alternative<string>(data))
            cout << "Message from " << remoteID << " received: " << get<string>(data) << endl;
        else {
            cout << "2Binary message from " << remoteID
				 << " received, size=" << get<binary>(data).size() << endl;
            handleFile(wdc, get<binary>(data));
        }
      });

      dc->send("Hello from " + myID);
//		dc->send()
//      std::cout << "start send file " << std::endl;
//      std::string file{"/Users/joey/Downloads/OpenVPNEnablerForBigSur.zip"};
////        std::vector<char> fileContent = readFile();
//      readFile(file, [&](const std::vector<char>& chars) {
//        std::vector<std::byte> bytes(chars.size());
//        std::transform(chars.begin(), chars.end(), bytes.begin(), [&](const char& c) {
//          return static_cast<std::byte>(c);
//        });
//        dc->send(bytes.data(), bytes.size());
//      });

//      dataChannelMap.emplace(remoteID, dc);
    });

//    peerConnectionMap.emplace(remoteID, pc);
    return pc;
};

void Client::handleFile(std::weak_ptr<rtc::DataChannel> dc, const std::vector<std::byte>& bytes) {
    if (dc.lock()) {
        std::string dest = "/Users/joey/Downloads/file.zip";
        std::vector<char> chars(bytes.size());
        std::transform(bytes.begin(), bytes.end(), chars.begin(), [](const std::byte byte){
          return static_cast<char>(byte);
        });
        appendFile(dest, chars);
    }
}



