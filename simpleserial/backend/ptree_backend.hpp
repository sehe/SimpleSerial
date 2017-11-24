#include <simpleserial/frontend.hpp>

#include <boost/property_tree/ptree.hpp>

namespace SimpleSerial { 
    namespace PtreeBackend {

        using Document = boost::property_tree::ptree;

        struct Saver {
            Document& _node;

            template <typename T> Saver& operator&(T const& value) {
                do_save(_node, value);
                return *this;
            }
          private:
            void do_save(Document& node, std::string const& value) { node.put_value(value); }
            void do_save(Document& node, int const& value)         { node.put_value(value); }

            template <typename T> 
            void do_save(Document& node, Frontend::nvp<T> const& wrap) {
                using Frontend::serialize;
                Saver sub{named_child(node, wrap.name)};
                serialize(sub, wrap.value); // recursive dispatch
            }

            template <typename T>
            void do_save(Document& parent, Frontend::ncp<T> const& wrap) {
                auto& list = named_child(parent, wrap.container_name);

                for (auto& item : wrap.value)
                    do_save(list, Frontend::make_nvp(wrap.item_name, item));
            }

            template <typename T> static T const& to_xml(T const& v) { return v; }
            static char const* to_xml(std::string const& v) { return v.c_str(); }

            Document& named_child(Document& parent, std::string const& name) {
                return parent.add_child(name, {});
            }
        };

        struct LoadError : std::runtime_error  {
            LoadError(std::string msg) : std::runtime_error(std::move(msg)) {}
        };

        struct Loader {
            Document& _node;
            bool throw_on_unexpected = false;

            Loader(Document& n) : _node(n) {}

            template <typename T> Loader& operator&(T&& value) {
                do_load(_node, std::forward<T>(value));
                return *this;
            }
          private:
            void do_load(Document& node, std::string& value) { value = node.get_value<std::string>(); }
            void do_load(Document& node, int& value)         { value = node.get_value<int>(); }

            template <typename T> 
            void do_load(Document& node, Frontend::nvp<T> const& wrap) {
                using Frontend::serialize;
                Loader sub{named_child(node, wrap.name)};
                serialize(sub, wrap.value); // recursive dispatch
            }

            template <typename T>
            void do_load(Document& parent, Frontend::ncp<T> const& wrap) {
                auto& list = named_child(parent, wrap.container_name);

                for (auto& node : list) {
                    if (node.first != std::string(wrap.item_name)) {
                        if (throw_on_unexpected) throw LoadError("unexpected child node");
                        continue;
                    }

                    Loader sub{node.second};
                    wrap.value.emplace_back();
                    using Frontend::serialize;
                    serialize(sub, wrap.value.back()); // recursive dispatch
                }
            }

            Document& named_child(Document& parent, std::string const& name) {
                return parent.get_child(name);
            }
        };
    }
}
