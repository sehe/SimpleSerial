#include <iostream>
#include "test/tester.hpp"

void run_pugi_backend(); // using PugiXML
void run_ptree_backends(); // both xml and json

int main() {
    run_pugi_backend();
    run_ptree_backends();
}

#include <simpleserial/backend/pugi_backend.hpp>

void run_pugi_backend()  { // using PugiXML
    using namespace SimpleSerial::PugiBackend;
    Tester<Document, Saver, Loader> tester;

    tester.roundtrip([](Document& doc) { doc.save_file("test-pugi.xml"); },
                     [](Document& doc) { doc.load_file("test-pugi.xml"); });
}

#include <simpleserial/backend/ptree_backend.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

void run_ptree_backends() { // both xml and json
    using namespace SimpleSerial::PtreeBackend;
    Tester<Document, Saver, Loader> tester;

    tester.roundtrip([](Document& pt) { write_json("test-ptree.json", pt); },
                     [](Document& pt) { read_json("test-ptree.json", pt); });

    tester.roundtrip([](Document& pt) { write_xml("test-ptree.xml", pt); },
                     [](Document& pt) { read_xml("test-ptree.xml",   pt); });
}
