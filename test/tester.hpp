#pragma once
#include "domain_dtos.hpp"
#include <functional>

template <typename Document, typename Saver, typename Loader> struct Tester {
    using ExternalOperation = std::function<void(Document&)>;

    void roundtrip(ExternalOperation physical_write, ExternalOperation physical_read) {
        using SimpleSerial::Frontend::make_nvp;

        {
            Document doc;
            Saver saver{doc};

            HardwareHostDto host = { 1, 1, "kiosk", { { 1, 2, "friendly" }, { 3, 4, "name" }, } };
            saver & make_nvp("HardwareHost", host);

            physical_write(doc);
        }

        {
            HardwareHostDto roundtrip;
            {
                Document doc;
                physical_read(doc);

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
};

