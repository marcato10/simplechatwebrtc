#include <nlohmann/json.hpp>
#include <rtc/candidate.hpp>
#include <rtc/peerconnection.hpp>
#include <string>
#include <vector>
#include "rtc/rtc.hpp"

const std::string DEFAULT_STUN_SERVER_URL = "stun.l.google.com";
constexpr uint16_t DEFAULT_STUN_SERVER_PORT = 19302;
const rtc::IceServer stunIceServer = {DEFAULT_STUN_SERVER_URL,
                                      DEFAULT_STUN_SERVER_PORT};

class SdpBundle {
public:
    std::string sdp;
    std::vector<rtc::Candidate> iceCandidates;

    explicit SdpBundle(std::string sdp, std::vector<rtc::Candidate> iceCandidates);
    ~SdpBundle();
    void addIceCandidate(const rtc::Candidate& candidate);
    explicit SdpBundle(const nlohmann::json& json);
    static nlohmann::json createJson(const SdpBundle& sdpBundle);
};

namespace sdp_bundle_serializer{
     void to_json(nlohmann::json& jSdpBundle,const SdpBundle& sdpBundle);
     void from_json(const nlohmann::json & jsonInput,SdpBundle& sdpBundle);
}
void initialize_peer_connection_callback(rtc::PeerConnection& peerConnection,std::vector<rtc::Candidate>&localCandidates);


