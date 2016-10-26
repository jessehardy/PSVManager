#ifndef UTF8_h__
#define UTF8_h__
#include <string>

inline std::wstring widen(const char *s) {
	std::wstring dst;
	auto l = strlen(s);
	dst.resize(l);
	auto rl = MultiByteToWideChar(CP_UTF8, 0, s, l, &dst[0], l);
	dst.resize(rl);
	return dst;
}
inline std::string narrow(const wchar_t *s) {
	auto l = wcslen(s);
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, s, l, nullptr, 0, nullptr, nullptr);
	std::string dst(size_needed, 0);
	auto rl = WideCharToMultiByte(CP_UTF8, 0, s, l, &dst[0], size_needed, nullptr, nullptr);
	return dst;
}

#endif // UTF8_h__
