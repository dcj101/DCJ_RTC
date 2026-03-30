#ifndef ICE_CREDENTIALS_H
#define ICE_CREDENTIALS_H
#include <string>

namespace xrtc {
struct IceParameters {
    IceParameters() = default;
    IceParameters(const std::string& ufrag, const std::string& password) : ice_ufrag(ufrag), ice_password(password) {};
    std::string ice_ufrag;
    std::string ice_password;
};

class IceCredentials {
public:
   static IceParameters create_random_ice_credentials();
};

}



#endif