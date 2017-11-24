#pragma once
#include <string>
#include <vector>

#include <simpleserial/frontend.hpp>
#define NVP(v)        ::SimpleSerial::Frontend::make_nvp(#v, v)
#define NCP(v, inner) ::SimpleSerial::Frontend::make_ncp(#v, inner, v)

struct HardwareDto {
    int HardwareHostID;
    int HardwareID;
    std::string HardwareFriendlyName;

    template <typename Ar> void serialize(Ar& ar) {
        ar & NVP(HardwareHostID)
           & NVP(HardwareID)
           & NVP(HardwareFriendlyName);
    }
};

struct HardwareHostDto {
    int HardwareHostID;
    int BranchID;
    std::string HardwareHostFriendlyName;
    std::vector<HardwareDto> HardwareList;

    template <typename Ar> void serialize(Ar& ar) {
        ar & NVP(HardwareHostID)
           & NVP(BranchID)
           & NVP(HardwareHostFriendlyName)
           & NCP(HardwareList, "Hardware");
    }
};

