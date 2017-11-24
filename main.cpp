#include <iostream>
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

#include <simpleserial/backend/pugi_backend.hpp>

int main() {
    using SimpleSerial::Frontend::make_nvp;

    {
        using namespace SimpleSerial::PugiBackend;

        Document doc;
        Saver saver{doc.root()};

        HardwareHostDto host = { 1, 1, "kiosk", { { 1, 2, "friendly" } } };
        saver & make_nvp("HardwareHost", host);

        doc.save_file("test.xml");
    }

    {
        using namespace SimpleSerial::PugiBackend;

        HardwareHostDto roundtrip;
        {
            Document doc;
            doc.load_file("test.xml");

            Loader loader{doc};

            loader & make_nvp("HardwareHost", roundtrip);
        }

        std::cout << "Read back: " << roundtrip.HardwareHostID << "\n";
        std::cout << "Read back: " << roundtrip.BranchID << "\n";
        std::cout << "Read back: " << roundtrip.HardwareHostFriendlyName << "\n";
        for (auto& h: roundtrip.HardwareList) {
            std::cout << "Item: " << h.HardwareID << ", " << h.HardwareHostID << ", " << h.HardwareFriendlyName << "\n";
        }
    }
}
