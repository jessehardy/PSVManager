/*
 *  QCMA: Cross-platform content manager assistant for the PS Vita
 *
 *  Copyright (C) 2013  Codestation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SFOREADER_H
#define SFOREADER_H
#include <stdint.h>
#include <endian/little_endian.hpp>
#include <map>
template<class T> class uilsb
{
public:
    operator T () const { return endian::little_endian::get<T>(data); }
    const T operator=(const T v) {
		endian::little_endian::put<T>(v, data);
        return v;
    }

private:
    static const int nBytes = sizeof(T);
	uint8_t data[nBytes];
};

class SfoReader
{
public:
	std::map<std::string, std::string> data;
    SfoReader();
    bool load(const char* key_offset);
    const char *value(const char *key, const char *defaultValue);

private:
    typedef struct {
        uilsb<uint16_t> key_offset;
        uint8_t alignment;
		uint8_t data_type;
        uilsb<uint32_t> value_size;
        uilsb<uint32_t> value_size_with_padding;
        uilsb<uint32_t> data_offset;
    } sfo_index;

    typedef struct {
        char id[4];
        uilsb<uint32_t> version;
        uilsb<uint32_t> key_offset;
        uilsb<uint32_t> value_offset;
        uilsb<uint32_t> pair_count;
    } sfo_header;

    const char *key_offset;
    const sfo_header *header;
    const sfo_index *index;
};

#endif // SFOREADER_H
