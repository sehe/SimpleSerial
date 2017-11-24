#include <simpleserial/frontend.hpp>

#include <pugixml.hpp>
#include <stdexcept>

namespace SimpleSerial { 
    namespace PugiBackend {

        using Document = pugi::xml_document;

        struct Saver {
            pugi::xml_node _node;

            Saver(Document const& doc) : _node(doc.root()) {}
            Saver(pugi::xml_node node) : _node(node) {}

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

        struct LoadError : std::runtime_error  {
            LoadError(std::string msg) : std::runtime_error(std::move(msg)) {}
        };

        struct Loader {
            pugi::xml_node _node;
            bool throw_on_unexpected = false;

            Loader(pugi::xml_node n) : _node(n) {}

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
                    if (node.type() != pugi::xml_node_type::node_element ) {
                        if (throw_on_unexpected) throw LoadError("Warning: unexpected child node type ignored");
                        continue;
                    }
                    if (node.name() != std::string(wrap.item_name)) {
                        if (throw_on_unexpected) throw LoadError("unexpected child node");
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
