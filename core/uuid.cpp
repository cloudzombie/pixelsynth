/* Core is a lightweight C++11 library to generate universally unique identificators.
* Core provides interface for UUID versions 1 and 4. Custom v0 is provided additionally.
* Copyright (c) 2013, Mario 'rlyeh' Rodriguez, zlib/libpng licensed.

* Based on code by Dmitri Bouianov, Philip O'Toole, Poco C++ libraries and
* anonymous code found on the net. Thanks guys!

* Theory: (see Hoylen's answer at [1])
* - UUID version 1 (48-bit MAC address + 60-bit clock with a resolution of 100ns)
*   Clock wraps in 3603 A.D.
*   Up to 10000000 UUIDs per second.
*   MAC address revealed.
*
* - UUID Version 4 (122-bits of randomness)
*   See [2] or other analysis that describe how very unlikely a duplicate is.
*
* - Use v1 if you need to sort or classify UUIDs per machine.
*   Use v1 if you are worried about leaving it up to probabilities (e.g. your are the
*   type of person worried about the earth getting destroyed by a large asteroid in your
*   lifetime). Just use a v1 and it is guaranteed to be unique till 3603 AD.
*
* - Use v4 if you are worried about security issues and determinism. That is because
*   v1 UUIDs reveal the MAC address of the machine it was generated on and they can be
*   predictable. Use v4 if you need more than 10 million uuids per second, or if your
*   application wants to live past 3603 A.D.

* Additionally a custom UUID v0 is provided:
* - 16-bit PID + 48-bit MAC address + 60-bit clock with a resolution of 100ns since Unix epoch
* - Format is EPOCH_LOW-EPOCH_MID-VERSION(0)|EPOCH_HI-PID-MAC
* - Clock wraps in 3991 A.D.
* - Up to 10000000 UUIDs per second.
* - MAC address and PID revealed.

* References:
* - [1] http://stackoverflow.com/questions/1155008/how-unique-is-uuid
* - [2] http://en.wikipedia.org/wiki/UUID#Random%5FUUID%5Fprobability%5Fof%5Fduplicates
* - http://en.wikipedia.org/wiki/Universally_unique_identifier
* - http://en.cppreference.com/w/cpp/numeric/random/random_device
* - http://www.itu.int/ITU-T/asn1/uuid.html f81d4fae-7dec-11d0-a765-00a0c91e6bf6

* - rlyeh ~~ listening to Hedon Cries / Until The Sun Goes up
*/

//////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <iomanip>
#include <random>
#include <sstream>

#include "uuid.h"

bool Core::Uuid::operator==(const Core::Uuid &other) const {
	return ab == other.ab && cd == other.cd;
}

bool Core::Uuid::operator!=(const Core::Uuid &other) const {
	return !operator==(other);
}

bool Core::Uuid::operator<(const Core::Uuid &other) const {
	if (ab < other.ab) return true;
	if (ab > other.ab) return false;
	if (cd < other.cd) return true;
	return false;
}

std::string Core::Uuid::str() const {
	std::stringstream ss;
	ss << std::hex << std::nouppercase << std::setfill('0');

	uint32_t a = (ab >> 32);
	uint32_t b = (ab & 0xFFFFFFFF);
	uint32_t c = (cd >> 32);
	uint32_t d = (cd & 0xFFFFFFFF);

	ss << std::setw(8) << (a) << '-';
	ss << std::setw(4) << (b >> 16) << '-';
	ss << std::setw(4) << (b & 0xFFFF) << '-';
	ss << std::setw(4) << (c >> 16) << '-';
	ss << std::setw(4) << (c & 0xFFFF);
	ss << std::setw(8) << d;

	return ss.str();
}
namespace Core {

	Uuid::Uuid(uint8_t* bytes)
	{
		ab = static_cast<uint64_t>(bytes[0]) << 56;
		ab |= static_cast<uint64_t>(bytes[1]) << 48;
		ab |= static_cast<uint64_t>(bytes[2]) << 40;
		ab |= static_cast<uint64_t>(bytes[3]) << 32;
		ab |= static_cast<uint64_t>(bytes[4]) << 24;
		ab |= static_cast<uint64_t>(bytes[5]) << 16;
		ab |= static_cast<uint64_t>(bytes[6]) << 8;
		ab |= static_cast<uint64_t>(bytes[7]);
		cd = static_cast<uint64_t>(bytes[8]) << 56;
		cd |= static_cast<uint64_t>(bytes[9]) << 48;
		cd |= static_cast<uint64_t>(bytes[10]) << 40;
		cd |= static_cast<uint64_t>(bytes[11]) << 32;
		cd |= static_cast<uint64_t>(bytes[12]) << 24;
		cd |= static_cast<uint64_t>(bytes[13]) << 16;
		cd |= static_cast<uint64_t>(bytes[14]) << 8;
		cd |= static_cast<uint64_t>(bytes[15]);
	}

	Uuid uuid4()
	{
		std::random_device rd;
		std::uniform_int_distribution<uint64_t> dist(0, static_cast<uint64_t>(~0));

		Uuid my;

		my.ab = dist(rd);
		my.cd = dist(rd);

		my.ab = (my.ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
		my.cd = (my.cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

		return my;
	}

	Uuid rebuild(uint64_t ab, uint64_t cd) {
		Uuid u;
		u.ab = ab, u.cd = cd;
		return u;
	}

	Uuid rebuild(const std::string &uustr) {
		char sep;
		uint64_t a, b, c, d, e;
		Uuid u;
		auto idx = uustr.find_first_of("-");
		if (idx != std::string::npos) {
			std::stringstream ss(uustr);
			if (ss >> std::hex >> a >> sep >> b >> sep >> c >> sep >> d >> sep >> e) {
				u.ab = (a << 32) | (b << 16) | c;
				u.cd = (d << 48) | e;
			}
		}
		return u;
	}

	template <class Archive>
	void Uuid::serialize(Archive& ar)
	{
		ar(ab);
		ar(cd);
	}

	template void Uuid::serialize<cereal::XMLOutputArchive>(cereal::XMLOutputArchive& archive);
	template void Uuid::serialize<cereal::XMLInputArchive>(cereal::XMLInputArchive& archive);

} // ::Core