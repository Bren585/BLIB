#pragma once
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/string.hpp>

#define NVP(obj, name) cereal::make_nvp(#name, obj.name)
#define SERIALIZE(...)		template <class Ar> inline void serialize(Ar& ar) { ar(__VA_ARGS__); }
#define SERIALIZE_EMPTY()	template <class Ar> inline void serialize(Ar& ar) { }
#define SERIALIZE_BASE(base, ...)	template <class Ar> inline void serialize(Ar& ar) { ar(cereal::base_class<base>(this)); ar(__VA_ARGS__); }
#define SERIALIZE_EMPTY_BASE(base)	template <class Ar> inline void serialize(Ar& ar) { ar(cereal::base_class<base>(this)); }

#define CONSTRUCT(type, ...)										\
namespace cereal {													\
	template <class Ar>												\
	inline void load_and_construct(Ar& ar, construct<type>& co) {	\
	type* _ptr = static_cast<type*>(::operator new(sizeof(type)));	\
	co(_ptr);														\
	auto& _ref = *_ptr;												\
	[&]() {ar(__VA_ARGS__); }();									\
} }	

#define CONSTRUCT_BASE(type, base, ...)									\
namespace cereal {														\
	template <class Ar>													\
	inline void load_and_construct(Ar& ar, construct<type>& co) {		\
		OutputDebugStringW(L#type);										\
		type* _ptr = static_cast<type*>(::operator new(sizeof(type)));	\
		co(_ptr);														\
		auto& self = *_ptr;												\
		[&]() {ar(base_class<base>(&self), __VA_ARGS__); }();			\
} }	

#include <fstream>
#define CEREAL(filepath, ...)	std::ofstream ofs((const char*)filepath, std::ios::binary); cereal::BinaryOutputArchive BOA(ofs); BOA(__VA_ARGS__)
#define UNCEREAL(filepath, ...)	std::ifstream ifs((const char*)filepath, std::ios::binary); cereal::BinaryInputArchive  BIA(ifs); BIA(__VA_ARGS__)