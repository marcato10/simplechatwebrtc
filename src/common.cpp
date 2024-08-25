#include "common.hpp"

#include <utility>
#include <nlohmann/json.hpp>

using nlohmann::json;


SdpBundle::SdpBundle(std::string sdp, std::vector<rtc::Candidate> iceCandidates):sdp(std::move(sdp)),iceCandidates(std::move(iceCandidates)){}
SdpBundle::~SdpBundle() = default;

void SdpBundle::addIceCandidate(const rtc::Candidate& candidate) {
        this->iceCandidates.emplace_back(candidate);
}

SdpBundle::SdpBundle(const json &json) {
    sdp_bundle_serializer::from_json(json,*this);

}

nlohmann::json SdpBundle::createJson(const SdpBundle &sdpBundle) {
    nlohmann::json jSdpBundle;
    sdp_bundle_serializer::to_json(jSdpBundle,sdpBundle);
    return jSdpBundle;
}


namespace sdp_bundle_serializer{

    void to_json(json& jSdpBundle,const SdpBundle& sdpBundle) {
        json j_array_candidate = json::array();
        for(const rtc::Candidate& candidate : sdpBundle.iceCandidates){
            j_array_candidate.emplace_back(std::string(candidate));
        }
        jSdpBundle = {
                {"sdp",sdpBundle.sdp},
                {"ice-candidates",j_array_candidate}
        };
    }

    void from_json(const json & jsonInput,SdpBundle& sdpBundle) {
        sdpBundle.sdp = jsonInput["sdp"];
        std::vector<rtc::Candidate> iceCandidates;

        for (std::string candidate: jsonInput.at("ice-candidates")) {
            sdpBundle.addIceCandidate(candidate);
        }
    }
}


void initialize_peer_connection_callback(rtc::PeerConnection& peerConnection,std::vector<rtc::Candidate>&localCandidates){
    peerConnection.onLocalCandidate(
            [&localCandidates](const rtc::Candidate& candidate) {
                std::cout << "Candidate added" << std::endl;
                localCandidates.emplace_back(candidate);
                std::cout << std::string(candidate) << std::endl;
            });

    peerConnection.onGatheringStateChange(
            [&peerConnection, &localCandidates](const rtc::PeerConnection::GatheringState state) {
                if (state == rtc::PeerConnection::GatheringState::Complete) {

                    SdpBundle sdpBundle = SdpBundle(peerConnection.localDescription().value(), localCandidates);

                    json jSdpBundle;
                    jSdpBundle["sdp"] = sdpBundle.sdp;
                    jSdpBundle["ice-candidates"] = sdpBundle.iceCandidates;

                    const json jTest = SdpBundle::createJson(sdpBundle);

                    assert(jSdpBundle == jTest && "JSON SDP INVALID");
                    std::cout << "\nDescription created (Send to another Peer)" << std::endl;
                    std::cout << jSdpBundle << std::endl;
                }
            });
    peerConnection.onStateChange([](const rtc::PeerConnection::State &state){
        std::cout << "STATE: " << state << std::endl;
    });
}
