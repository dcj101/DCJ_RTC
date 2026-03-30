#include "ice_credentials.h"
#include <random>
#include <string>
#include <vector>
#include <rtc_base/helpers.h>
#include "ice_def.h"
namespace xrtc {

IceParameters IceCredentials::create_random_ice_credentials() {
    return IceParameters(rtc::CreateRandomString(ICE_UFRAG_LEN), rtc::CreateRandomString(ICE_PASSWORD_LEN));
}

}
