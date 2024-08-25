#include "common.hpp"
#include "rtc/rtc.hpp"
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <rtc/candidate.hpp>
#include <rtc/datachannel.hpp>
#include <rtc/peerconnection.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using nlohmann::json;

int main(int argc, char **argv) {
    rtc::InitLogger(rtc::LogLevel::Info);
    std::vector<rtc::Candidate> localCandidates;
    std::string localSdp;
    rtc::Configuration config;

    config.iceServers.emplace_back(stunIceServer);
    auto peerConnection = std::make_unique<rtc::PeerConnection>(config);

    initialize_peer_connection_callback(*peerConnection.get(),localCandidates);

    std::shared_ptr<rtc::DataChannel>dataChannel;
    peerConnection->onDataChannel([&peerConnection,&dataChannel](const std::shared_ptr<rtc::DataChannel>& _dataChannel)
    {
        std::cout << "Data channel opened" << _dataChannel << std::endl;
        dataChannel = _dataChannel;

        dataChannel->onMessage([&](const rtc::message_variant& packet){
            if(holds_alternative<std::string>(packet)){
                rtc::Candidate local,remote;
                peerConnection->getSelectedCandidatePair(&local,&remote);
                std::cout << "[" << remote.candidate() << "]: " << std::get<std::string>(packet) << std::endl;
            }
        });

    });

    if (!peerConnection) {
        std::cerr << "Failed to create PeerConnection" << std::endl;
        return -1;
    }

    std::string inputSdpBundle;
    while (peerConnection->state() == rtc::PeerConnection::State::New || peerConnection->state() == rtc::PeerConnection::State::Connecting) {
        std::cout << "Paste the offer: \nPress 0 to exit" << std::endl;
        std::getline(std::cin,inputSdpBundle);

        if(inputSdpBundle.empty()){
            std::cout << "Empty SDP" << std::endl;
            continue;
        }

        json jInput = json::parse(inputSdpBundle);
        SdpBundle sdpBundle = SdpBundle(jInput);

        peerConnection->setRemoteDescription(sdpBundle.sdp);
        peerConnection->addRemoteCandidate(sdpBundle.iceCandidates.begin()->candidate());

        if(peerConnection->state() == rtc::PeerConnection::State::Connected)
            break;
    }

    std::cout << dataChannel.get() << std::endl;
    std::string message;
    while(dataChannel || dataChannel->isOpen()){
        std::cout << "Type a message: " << std::endl;
        std::getline(std::cin,message);

        dataChannel->send(message);
    }
    std::cout << "Session Closed" << std::endl;
    return 0;
}


/*
 */
