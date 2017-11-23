#include <pugixml.hpp>
#include <iostream>
#include <vector>

namespace SimpleSerial {

    namespace Frontend { // support plumbing, independent of e.g. Pugi backend
        template<typename T>
        struct nvp { // named value
            const char* name;
            T value;
        };

        template<typename T>
        struct ncp { // named container
            const char* container_name;
            const char* item_name;
            T value;
        };

        template <typename T> nvp<T&>       make_nvp(const char* name, T& v)       { return {name, v}; }
        template <typename T> nvp<T const&> make_nvp(const char* name, T const& v) { return {name, v}; }

        template <typename T> ncp<T&>       make_ncp(const char* outer, const char* inner, T& v)       { return {outer, inner, v}; }
        template <typename T> ncp<T const&> make_ncp(const char* outer, const char* inner, T const& v) { return {outer, inner, v}; }

        template <typename T, typename Ar, typename Enable = void> struct has_intrusive_serialize : std::false_type {};
        template <typename T, typename Ar> 
            struct has_intrusive_serialize<T, Ar, decltype(std::declval<T&>().serialize(std::declval<Ar&>()))> 
                : std::true_type 
            {};

        template <typename Ar, typename T>
            typename std::enable_if<has_intrusive_serialize<T, Ar>{} >::type serialize(Ar& ar, T& v) {
                v.serialize(ar);
            }

        template <typename Ar, typename T>
            typename std::enable_if<not has_intrusive_serialize<T, Ar>{} >::type serialize(Ar& ar, T& v) {
                ar & v; 
            }
    }

}

namespace SimpleSerial { 
    namespace PugiBackend { // support plumbing, independent of Pugi

        struct Saver {
            pugi::xml_node _node;

            template <typename T> Saver& operator&(T const& value) {
                do_save(_node, value);
                return *this;
            }
          private:
            void do_save(pugi::xml_node node, std::string const& value) { node.text().set(to_xml(value)); }
            void do_save(pugi::xml_node node, int const& value)         { node.text().set(to_xml(value)); }

            template <typename T> 
            void do_save(pugi::xml_node node, Frontend::nvp<T> const& wrap) {
                using Frontend::serialize;
                Saver sub{named_child(node, wrap.name)};
                serialize(sub, wrap.value); // recursive dispatch
            }

            template <typename T>
            void do_save(pugi::xml_node parent, Frontend::ncp<T> const& wrap) {
                auto list = named_child(parent, wrap.container_name);

                for (auto& item : wrap.value)
                    do_save(list, Frontend::make_nvp(wrap.item_name, item));
            }

            template <typename T> static T const& to_xml(T const& v) { return v; }
            static char const* to_xml(std::string const& v) { return v.c_str(); }

            pugi::xml_node named_child(pugi::xml_node parent, std::string const& name) {
                auto child = parent.append_child();
                child.set_name(name.c_str());
                return child;
            }
        };

        struct Loader {
            pugi::xml_node _node;

            template <typename T> Loader& operator&(T&& value) {
                do_load(_node, std::forward<T>(value));
                return *this;
            }
          private:
            void do_load(pugi::xml_node node, std::string& value) { value = node.text().as_string(); }
            void do_load(pugi::xml_node node, int& value)         { value = node.text().as_int(); }

            template <typename T> 
            void do_load(pugi::xml_node node, Frontend::nvp<T> const& wrap) {
                using Frontend::serialize;
                Loader sub{named_child(node, wrap.name)};
                serialize(sub, wrap.value); // recursive dispatch
            }

            template <typename T>
            void do_load(pugi::xml_node parent, Frontend::ncp<T> const& wrap) {
                auto list = named_child(parent, wrap.container_name);

                for (auto& node : list) {
                    if (node.type() != pugi::xml_node_type::node_element) {
                        std::cerr << "Warning: unexpected child node type ignored\n"; continue;
                    }
                    if (node.name() != std::string(wrap.item_name)) {
                        std::cerr << "Warning: unexpected child node ignored (" << node.name() << ")\n";
                        continue;
                    }

                    Loader sub{node};
                    wrap.value.emplace_back();
                    using Frontend::serialize;
                    serialize(sub, wrap.value.back()); // recursive dispatch
                }
            }

            pugi::xml_node named_child(pugi::xml_node parent, std::string const& name) {
                return parent.first_element_by_path(name.c_str());
            }
        };
    }
}

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

int main() {
    using namespace SimpleSerial::PugiBackend;
    using SimpleSerial::Frontend::make_nvp;

    {
        pugi::xml_document doc;
        Saver saver{doc.root()};

        HardwareHostDto host = { 1, 1, "kiosk", { { 1, 2, "friendly" } } };
        saver & make_nvp("HardwareHost", host);

        doc.save_file("test.xml");
    }

    {
        HardwareHostDto roundtrip;
        {
            pugi::xml_document doc;
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
