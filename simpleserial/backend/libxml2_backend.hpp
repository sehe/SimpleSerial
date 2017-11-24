#include <simpleserial/frontend.hpp>

#include <libxml++/libxml++.h>
#include <cassert>

namespace SimpleSerial { 
    namespace LibXml2Backend {

        struct Document { // This is a sloppy interface, just to make the tester happy
            xmlpp::DomParser _parser;
            bool _owned = true;
            xmlpp::Document* _impl = new xmlpp::Document();

            void parse_file(std::string fname) {
                clear();
                _parser.parse_file(fname); 
                _owned = false;
                _impl = _parser.get_document();
            }

            void write_to_file(std::string fname) {
                assert(_impl);
                _impl->write_to_file(fname);
            }

            void clear() {
                if (_owned)
                    delete _impl;
                _impl = nullptr;
            }

            ~Document() { clear(); }
        };

        struct Saver {
            xmlpp::Element* _node;

            Saver(Document const& doc) : _node(doc._impl->get_root_node()) {
                if (!_node) {
                    _node = doc._impl->create_root_node("t-e-m-p");
                }
                assert(_node);
            }
            Saver(xmlpp::Element* node) : _node(node) {
                assert(_node);
            }

            template <typename T> Saver& operator&(T const& value) {
                do_save(_node, value);
                return *this;
            }
          private:
            void do_save(xmlpp::Element* node, std::string const& value) { node->add_child_text(to_xml(value)); }
            void do_save(xmlpp::Element* node, int const& value)         { node->add_child_text(std::to_string(value)); }

            template <typename T> 
            void do_save(xmlpp::Element* node, Frontend::nvp<T> const& wrap) {
                using Frontend::serialize;
                Saver sub{named_child(node, wrap.name)};
                serialize(sub, wrap.value); // recursive dispatch
            }

            template <typename T>
            void do_save(xmlpp::Element* parent, Frontend::ncp<T> const& wrap) {
                auto list = named_child(parent, wrap.container_name);

                for (auto& item : wrap.value)
                    do_save(list, Frontend::make_nvp(wrap.item_name, item));
            }

            template <typename T> static T const& to_xml(T const& v) { return v; }
            static char const* to_xml(std::string const& v) { return v.c_str(); }

            xmlpp::Element* named_child(xmlpp::Element* parent, std::string const& name) {
                if (parent->get_parent() == NULL && parent->get_name() == std::string("t-e-m-p")) {
                    parent->set_name(name);
                    return parent;
                } else
                    return parent->add_child(name);
            }
        };

        struct Loader {
            xmlpp::Element* _node;
            bool _is_root = false;

            Loader(Document const& doc) : _node(doc._impl->get_root_node()), _is_root(true) {
                assert(_node);
            }
            Loader(xmlpp::Element* n) : _node(n) {
                assert(_node);
            }

            template <typename T> Loader& operator&(T&& value) {
                do_load(_node, std::forward<T>(value));
                return *this;
            }
          private:
            void do_load(xmlpp::Element* node, std::string& value) { value = node->get_child_text()->get_content(); }
            void do_load(xmlpp::Element* node, int& value)         { 
                std::string s = node->get_child_text()->get_content();
                value = std::stoi(s);
            }

            template <typename T> 
            void do_load(xmlpp::Element* node, Frontend::nvp<T> const& wrap) {
                using Frontend::serialize;
                Loader sub{named_child(node, wrap.name)};
                serialize(sub, wrap.value); // recursive dispatch
            }

            template <typename T>
            void do_load(xmlpp::Element* parent, Frontend::ncp<T> const& wrap) {
                auto list = named_child(parent, wrap.container_name);

                for (auto node : list->get_children(wrap.item_name)) {
                    if (auto element = dynamic_cast<xmlpp::Element*>(node)) {
                        Loader sub{element};
                        wrap.value.emplace_back();
                        using Frontend::serialize;
                        serialize(sub, wrap.value.back()); // recursive dispatch
                    }
                }
            }

            xmlpp::Element* named_child(xmlpp::Element* parent, std::string const& name) {
                if (_is_root) {
                    assert(parent->get_parent() == NULL && parent->get_name() == name);
                    return parent;
                } else {
                    return dynamic_cast<xmlpp::Element*>(parent->get_first_child(name));
                }
            }
        };
    }
}
