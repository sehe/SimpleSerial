#pragma once
#include <utility> // std::declval

namespace SimpleSerial {

    namespace Frontend { // support plumbing, independent of backends
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
