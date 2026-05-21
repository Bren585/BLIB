#pragma once
#include <string>
#include "cereal.h"

class string {
private:
	static inline		unsigned int	precision		= 2;
	static constexpr	unsigned int	precision_cap	= 10;
	
	std::string data = "";
	mutable std::wstring wide_cache = L"";
	mutable bool cached = false;

	void cache() const;

public:
	// Construction
	string() {}
	string(const string&		o)		: data(o.data) {}
	string(const std::string&	data)	: data(data) {}
	string(const char*			data)	: data(data) {}
	string(char					data)	{ this->data = data; }
	string(const std::wstring&  data)	: string(data.c_str()) {}
	string(const wchar_t*		data);
	string(wchar_t data				)	{ wchar_t d[2] = {data, 0}; *this = string(d); }
	string(int					data);
	string(float				data, int precision = string::precision);

	string(string&& o) noexcept : data(std::move(o.data)) {}

	template<typename I>
	string(const I i, typename std::enable_if_t<std::is_integral<I>::value>::type* = nullptr) : string(static_cast<int>(i)) {}

	template<typename F>
	string(const F f, typename std::enable_if_t<std::is_floating_point<F>::value>::type* = nullptr) : string(static_cast<float>(f)) {}

	string& operator= (const string&  o)			{ data  = o.data;				cached = false; return *this; }
	string& operator+=(const string&  o)			{ data += o.data;				cached = false; return *this; }
	string& operator= (		 string&& o) noexcept	{ data  = o.data;				cached = false; return *this; }

	friend string operator+ (const string& a, const string& b) { return a.data + b.data; }
	
	bool operator== (const string& o) const { return data == o.data; }
	bool operator!= (const string& o) const { return data != o.data; }
	bool operator<  (const string& o) const { return data <  o.data; }

	template <typename S, typename = std::enable_if_t<!std::is_same_v<std::decay_t<S>, string>>>
	bool operator== (const S& o) const { return operator==(string(o)); }

	template <typename S, typename = std::enable_if_t<!std::is_same_v<std::decay_t<S>, string>>>
	bool operator!= (const S& o) const { return operator!=(string(o)); }

	template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
	string(Args&&... args) { (operator+=(string(std::forward<Args>(args))), ...); }

	// Static 
	static void set_precision(int p) { precision = (p < 0) ? 0 : ((p < precision_cap ? p : precision_cap)); }

	// Iterating
	auto begin	() { return data.begin	(); }
	auto rbegin	() { return data.rbegin	(); }
	auto end	() { return data.end	(); }
	auto rend	() { return data.rend	(); }

	// Casting
	operator std::string	() const { return data; }
	operator const char*	() const { return data.c_str(); }
	operator std::wstring	() const;
	operator const wchar_t* () const { cache(); return wide_cache.c_str(); }

	const wchar_t* wide() const { return static_cast<const wchar_t*>(*this); }

	// Filepath
	string	get_filename	()					const;
	string	get_filepath	()					const;
	string	replace_ext		(const string& ext) const;
	bool	file_exists		()					const;

	// Other
	char& operator[](int i) { return data[i]; }
	size_t length() const { return data.length(); }
	void pop_back() { if (!data.empty()) data.pop_back(); }

	SERIALIZE(data)
};

namespace std {
	template<>
	struct hash<::string> {
		size_t operator()(const ::string& s) const noexcept {
			return std::hash<std::string>()(s);
		}
	};
}