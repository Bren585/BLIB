#include "pch.h"
#include <io.h>

string::string(const wchar_t* data) {
	int size = WideCharToMultiByte(CP_UTF8, 0, data, -1, nullptr, 0, nullptr, nullptr);
	this->data = std::string(size - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, data, -1, &(this->data)[0], size - 1, nullptr, nullptr);
}

constexpr int pot[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

string::string(int data) {
	if (data == 0) { this->data = "0"; return; }
	if (data < 0) { this->data = "-"; data = -data; }
	int exp = 1;
	while (exp < precision_cap && data >= pot[exp]) { exp++; }
	this->data.reserve(exp + 1);
	while (exp-- > 0) {
		const int scale = pot[exp];
		int digit = (data / scale);
		this->data += static_cast<char>(static_cast<int>('0') + digit);
		data -= digit * scale;
	}
}

string::string(float data, int precision) {
	if (data < 0) { this->data = "-"; data = -data; }
	assert(precision < precision_cap);
	int whole = static_cast<int>(data);
	int fraction = static_cast<int>((data - static_cast<int>(data)) * pot[precision] + 0.5f);
	if (fraction >= pot[precision]) { fraction = 0; whole++; }
	operator+=(whole);
	if (precision <= 0) return;
	operator+=(".");
	for (int local_precision = precision - 1; fraction < pot[local_precision] && local_precision > 0; local_precision--) { operator+=("0"); }
	operator+=(fraction);
}

void string::cache() const {
	cached = true;
	int size = MultiByteToWideChar(CP_UTF8, 0, data.c_str(), -1, nullptr, 0);
	wide_cache = std::wstring(size - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, data.c_str(), -1, &wide_cache[0], size - 1);
}

string::operator std::wstring() const {
	if (cached) { return wide_cache; }
	cache();
	return wide_cache;
}

string string::get_filename() const {
	auto rc = data.rbegin();
	while (rc != data.rend() && (*rc != '\\' && *rc != '/')) { rc++; }
	string out;
	auto fc = rc.base();
	while (fc != data.end()) { out += *fc++; }
	return out;
}

string string::get_filepath() const {
	auto rc = data.rbegin();
	while (rc != data.rend() && (*rc != '\\' && *rc != '/')) { rc++; }
	string out;
	auto fc = data.begin();
	while (fc != rc.base()) { out += *fc++; }
	return out;
}

string string::replace_ext(const string& ext) const {
	auto rc = data.rbegin();
	while (rc != data.rend() && *rc != '.') { rc++; }
	string out;
	auto fc = data.begin();
	while (fc != rc.base()) { out += *fc++; }
	return out + ext;
}

bool string::file_exists() const {
	return _access(data.c_str(), 0) == 0;
}