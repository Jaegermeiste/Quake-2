/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/*
ref_dx12
2025 Bleeding Eye Studios
*/

#ifndef __DX12_UTILITY_HPP__
#define __DX12_UTILITY_HPP__
#pragma once

#include "dx12_local.hpp"

namespace dx12_utility
{
	extern boost::uuids::time_generator_v7 uuid7_gen;
};

inline std::wstring ToWideString(std::string inStr)
{
	if (inStr.empty())
	{
		return std::wstring();
	}

	return boost::locale::conv::utf_to_utf<wchar_t>(inStr);
}

inline std::wstring ToWideString(WCHAR* inWideStr)
{
	if (!inWideStr)
	{
		return std::wstring();
	}

	return std::wstring(inWideStr);
}

inline std::wstring ToWideString(const WCHAR* inWideStr)
{
	if (!inWideStr)
	{
		return std::wstring();
	}

	return std::wstring(inWideStr);
}

inline std::wstring ToWideString(const char* inStr)
{
	if (inStr == nullptr)
	{
		return std::wstring();
	}

	return boost::locale::conv::utf_to_utf<wchar_t>(inStr);
}

inline std::wstring& ToWideString(std::wstring& inWideStr)
{
	return inWideStr;
}

inline std::string ToString(std::wstring inWideStr)
{
	if (inWideStr.empty())
	{
		return std::string();
	}

	return boost::locale::conv::utf_to_utf<char>(inWideStr);
}

inline std::string ToString(WCHAR* inWideStr)
{
	if (!inWideStr)
	{
		return std::string();
	}

	return boost::locale::conv::utf_to_utf<char>(inWideStr);
}

template <>
struct std::formatter<const char*, wchar_t> : std::formatter<const char*, char> {
	constexpr auto parse(std::wformat_parse_context& ctx) {
		auto it = ctx.begin();

		m_format = L"";

		while (*it && *it != '}')
		{
			m_format += *it;
			++it;
		}
		return it;
	}

	template <typename FormatContext>
	auto format(std::string_view value, FormatContext& ctx) const {
		std::wstring wstr(value.begin(), value.end());

		std::wstring final_string = L"";

		if (m_format.size() > 0)
		{
			final_string = L"{:" + m_format + L"}";
		}
		else
		{
			final_string = L"{}";
		}

		return std::vformat_to(ctx.out(), final_string, std::make_wformat_args(wstr));
	}

	mutable std::wstring m_format;
};

template <>
struct std::formatter<char*, wchar_t> : std::formatter<char*, char> {
	constexpr auto parse(std::wformat_parse_context& ctx) {
		auto it = ctx.begin();

		m_format = L"";

		while (*it && *it != '}')
		{
			m_format += *it;
			++it;
		}
		return it;
	}

	template <typename FormatContext>
	auto format(std::string_view value, FormatContext& ctx) const {
		std::wstring wstr(value.begin(), value.end());

		std::wstring final_string = L"";

		if (m_format.size() > 0)
		{
			final_string = L"{:" + m_format + L"}";
		}
		else
		{
			final_string = L"{}";
		}

		return std::vformat_to(ctx.out(), final_string, std::make_wformat_args(wstr));
	}

	mutable std::wstring m_format;
};


template <>
struct std::formatter<std::string, wchar_t> : std::formatter<std::string, char> {
	constexpr auto parse(std::wformat_parse_context& ctx) {
		auto it = ctx.begin();

		m_format = L"";

		while (*it && *it != '}')
		{
			m_format += *it;
			++it;
		}
		return it;
	}

	template <typename FormatContext>
	auto format(std::string_view value, FormatContext& ctx) const {
		std::wstring wstr(value.begin(), value.end());

		std::wstring final_string = L"";

		if (m_format.size() > 0)
		{
			final_string = L"{:" + m_format + L"}";
		}
		else
		{
			final_string = L"{}";
		}

		return std::vformat_to(ctx.out(), final_string, std::make_wformat_args(wstr));
	}

	mutable std::wstring m_format;
};

template <>
struct std::formatter<std::string_view, wchar_t> : std::formatter<std::string_view, char> {
	constexpr auto parse(std::wformat_parse_context& ctx) {
		auto it = ctx.begin();

		m_format = L"";

		while (*it && *it != '}')
		{
			m_format += *it;
			++it;
		}
		return it;
	}

	template <typename FormatContext>
	auto format(std::string_view value, FormatContext& ctx) const {
		std::wstring wstr(value.begin(), value.end());

		std::wstring final_string = L"";

		if (m_format.size() > 0)
		{
			final_string = L"{:" + m_format + L"}";
		}
		else
		{
			final_string = L"{}";
		}

		return std::vformat_to(ctx.out(), final_string, std::make_wformat_args(wstr));
	}

	mutable std::wstring m_format;
};


// Wrapper to handle narrow strings properly inside wformat
template <typename T, typename... Args>
std::wstring Format(T fmt, Args&&... args) {
	return std::vformat(ToWideString(fmt), std::make_wformat_args(args...));
}


inline static bool IsValidPointer(void* ptr)
{
	LOG_FUNC();

	if (ptr == nullptr)
	{
		return false;
	}

	try {
#ifdef _WIN32
		MEMORY_BASIC_INFORMATION mbi = {};
		if (VirtualQuery(ptr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
		{
			return false; // Pointer is not valid
		}

		return mbi.State == MEM_COMMIT; // Ensures it's committed memory

#else
		long pageSize = sysconf(_SC_PAGESIZE);
		void* pageStart = (void*)((uintptr_t)ptr & ~(pageSize - 1));

		unsigned char status;
		if (mincore(pageStart, pageSize, &status) == 0)
			return true;
#endif
	}
	catch (const std::runtime_error& e) {
		LOG(error) << "Runtime Error: " << e.what();
	}
	catch (const std::exception& e) {
		LOG(error) << "General Exception: " << e.what();
	}

	return false;
}

inline static std::wstring FormatMemory(size_t usage)
{
	static float gib = 1073741824.0f;
	static float mib = 1048576.0f;
	static float kib = 1024.0f;

	if (usage > gib)
	{
		return Format(L"{:.3} GiB", usage / gib);
	}
	else if (usage > mib)
	{
		return Format(L"{:.3} MiB", usage / mib);
	}
	else if (usage > kib)
	{
		return Format(L"{:.3} KiB", usage / kib);
	}
	else
	{
		return Format(L"{} B", usage);
	}

	return L"";
}

inline std::wstring GetUUIDv7() { return boost::uuids::to_wstring(dx12_utility::uuid7_gen()); };

Vector2 GetNormalizedDeviceCoordinates(int px, int py, int windowWidth, int windowHeight);

Vector4 GetNormalizedDeviceRectangle(int px, int py, int pw, int ph, int windowWidth, int windowHeight);

#endif // __DX12_UTILITY_HPP__
