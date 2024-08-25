#include <iostream>
#include "rtc/rtc.hpp"
#include <memory>
#include <rtc/candidate.hpp>
#include <rtc/description.hpp>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include "common.hpp"

using namespace std::chrono_literals;
using nlohmann::json;

static void printMenu();


int main(int argc, char **argv) {
    rtc::InitLogger(rtc::LogLevel::Warning);
    std::vector<rtc::Candidate> localCandidates = {};

    rtc::Configuration config;
    config.iceServers.emplace_back(stunIceServer);
    auto peerConnection = std::make_unique<rtc::PeerConnection>(config);
    initialize_peer_connection_callback(*peerConnection.get(),localCandidates);

    auto dataChannel = peerConnection->createDataChannel("chat-channel");

    dataChannel->onOpen([&]() {
        std::cout << "Data channel opened" << dataChannel->label() << std::endl;
    });

    dataChannel->onMessage([&](const rtc::message_variant& packet){
        if(holds_alternative<std::string>(packet)){
            rtc::Candidate local,remote;
            peerConnection->getSelectedCandidatePair(&local,&remote);
            std::cout << "[" << remote.candidate() << "]: " << std::get<std::string>(packet) << std::endl;
        }
    });

    if (!peerConnection) {

        std::cerr << "Failed to create PeerConnection" << std::endl;
        return -1;
    }

    std::string inputSdpBundle;

    while (true) {
        if(peerConnection->state() == rtc::PeerConnection::State::Connected || peerConnection->state() == rtc::PeerConnection::State::Failed)
            break;

        std::cout << "Paste the answer: \nPress 0 to exit" << std::endl;
        std::getline(std::cin,inputSdpBundle);

        if(inputSdpBundle.empty()){
            std::cout << "Empty SDP" << std::endl;
            continue;
        }
        json jInput = json::parse(inputSdpBundle);
        SdpBundle sdpBundle = SdpBundle(jInput);

        peerConnection->setRemoteDescription(sdpBundle.sdp);
        peerConnection->addRemoteCandidate(sdpBundle.iceCandidates.back().candidate());

    }

    std::string message;
    while(dataChannel || dataChannel->isOpen()){
        std::cout << "Type a message: " << std::endl;
        std::getline(std::cin,message);

        dataChannel->send(message);
    }
    std::cout << "Session Closed" << std::endl;
    return 0;

}


