//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2013 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_CPP11_DEFINE_MAP_HPP
#define MSGPACK_CPP11_DEFINE_MAP_HPP

#if defined(MSGPACK_USE_BOOST)

// BOOST_PP_VARIADICS is defined in boost/preprocessor/config/config.hpp
// http://www.boost.org/libs/preprocessor/doc/ref/variadics.html
// However, supporting compiler detection is not complete. msgpack-c requires
// variadic macro arguments support. So BOOST_PP_VARIADICS is defined here explicitly.
#define BOOST_PP_VARIADICS
#include <boost/preprocessor.hpp>

#include "msgpack/versioning.hpp"
#include "msgpack/adaptor/adaptor_base.hpp"

// for MSGPACK_ADD_ENUM
#include "msgpack/adaptor/int.hpp"

#include <type_traits>
#include <tuple>

#define MSGPACK_DEFINE_MAP_EACH_PROC(r, data, elem) \
    BOOST_PP_IF( \
        BOOST_PP_IS_BEGIN_PARENS(elem), \
        elem, \
        (BOOST_PP_STRINGIZE(elem))(elem) \
    )

#define MSGPACK_DEFINE_MAP_IMPL(...) \
    BOOST_PP_SEQ_TO_TUPLE( \
        BOOST_PP_SEQ_FOR_EACH( \
            MSGPACK_DEFINE_MAP_EACH_PROC, \
            0, \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
        ) \
    )

#define MSGPACK_DEFINE_MAP(...) \
    template <typename Packer> \
    void msgpack_pack(Packer& pk) const \
    { \
        msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_pack(pk); \
    } \
    void msgpack_unpack(msgpack::object const& o) \
    { \
        msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_unpack(o); \
    }\
    template <typename MSGPACK_OBJECT> \
    void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone& z) const \
    { \
        msgpack::type::make_define_map \
            MSGPACK_DEFINE_MAP_IMPL(__VA_ARGS__) \
            .msgpack_object(o, z); \
    }

#define MSGPACK_BASE_MAP(base) \
    (BOOST_PP_STRINGIZE(base))(*const_cast<base *>(static_cast<base const*>(this)))

namespace msgpack {
/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond
namespace type {

template <typename Tuple, std::size_t N>
struct define_map_imp {
    template <typename Packer>
    static void pack(Packer& pk, Tuple const& t) {
        define_map_imp<Tuple, N-1>::pack(pk, t);
        pk.pack(std::get<N-1>(t));
    }
    static void unpack(
        msgpack::object const& o, Tuple const& t,
        std::map<std::string, msgpack::object const*> const& kvmap) {
        define_map_imp<Tuple, N-2>::unpack(o, t, kvmap);
        auto it = kvmap.find(std::get<N-2>(t));
        if (it != kvmap.end()) {
            it->second->convert(std::get<N-1>(t));
        }
    }
    static void object(msgpack::object* o, msgpack::zone& z, Tuple const& t) {
        define_map_imp<Tuple, N-2>::object(o, z, t);
        o->via.map.ptr[(N-1)/2].key = msgpack::object(std::get<N-2>(t), z);
        o->via.map.ptr[(N-1)/2].val = msgpack::object(std::get<N-1>(t), z);
    }
};

template <typename Tuple>
struct define_map_imp<Tuple, 0> {
    template <typename Packer>
    static void pack(Packer&, Tuple const&) {}
    static void unpack(
        msgpack::object const&, Tuple const&,
        std::map<std::string, msgpack::object const*> const&) {}
    static void object(msgpack::object*, msgpack::zone&, Tuple const&) {}
};

template <typename... Args>
struct define_map {
    define_map(Args&... args) :
        a(args...) {}
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        static_assert(sizeof...(Args) % 2 == 0, "");
        pk.pack_map(sizeof...(Args) / 2);

        define_map_imp<std::tuple<Args&...>, sizeof...(Args)>::pack(pk, a);
    }
    void msgpack_unpack(msgpack::object const& o) const
    {
        if(o.type != msgpack::type::MAP) { throw msgpack::type_error(); }
        std::map<std::string, msgpack::object const*> kvmap;
        for (uint32_t i = 0; i < o.via.map.size; ++i) {
            kvmap.emplace(
                std::string(
                    o.via.map.ptr[i].key.via.str.ptr,
                    o.via.map.ptr[i].key.via.str.size),
                &o.via.map.ptr[i].val);
        }
        define_map_imp<std::tuple<Args&...>, sizeof...(Args)>::unpack(o, a, kvmap);
    }
    void msgpack_object(msgpack::object* o, msgpack::zone& z) const
    {
        static_assert(sizeof...(Args) % 2 == 0, "");
        o->type = msgpack::type::MAP;
        o->via.map.ptr = static_cast<msgpack::object_kv*>(z.allocate_align(sizeof(msgpack::object_kv)*sizeof...(Args)/2));
        o->via.map.size = sizeof...(Args) / 2;

        define_map_imp<std::tuple<Args&...>, sizeof...(Args)>::object(o, z, a);
    }

    std::tuple<Args&...> a;
};


template <typename... Args>
define_map<Args...> make_define_map(Args&... args)
{
    return define_map<Args...>(args...);
}

}  // namespace type
/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond
}  // namespace msgpack

#endif // defined(MSGPACK_USE_BOOST)

#endif // MSGPACK_CPP11_DEFINE_MAP_HPP
