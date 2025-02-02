/*
 * libdatachannel streamer example
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#if RTC_ENABLE_MEDIA

#include "nalunit.hpp"
#include "globals.hpp"

#include <cmath>

namespace rtc {

NalUnitFragmentA::NalUnitFragmentA(FragmentType type, bool forbiddenBit, uint8_t nri,
								   uint8_t unitType, binary data)
: NalUnit(data.size() + 2) {
	setForbiddenBit(forbiddenBit);
	setNRI(nri);
	fragmentIndicator()->setUnitType(NalUnitFragmentA::nal_type_fu_A);
	setFragmentType(type);
	setUnitType(unitType);
	copy(data.begin(), data.end(), begin() + 2);
}

std::vector<shared_ptr<NalUnitFragmentA>> NalUnitFragmentA::fragmentsFrom(shared_ptr<NalUnit> nalu,
																			   uint16_t maximumFragmentSize) {
	assert(nalu->size() > maximumFragmentSize);
	if (nalu->size() <= maximumFragmentSize) {
		// we need to change `maximum_fragment_size` to have at least two fragments
		maximumFragmentSize = nalu->size() / 2;
	}
	auto fragments_count = ceil(double(nalu->size()) / maximumFragmentSize);
	maximumFragmentSize = ceil(nalu->size() / fragments_count);

	// 2 bytes for FU indicator and FU header
	maximumFragmentSize -= 2;
	auto f = nalu->forbiddenBit();
	uint8_t nri = nalu->nri() & 0x03;
	uint8_t naluType = nalu->unitType() & 0x1F;
	auto payload = nalu->payload();
	vector<shared_ptr<NalUnitFragmentA>> result{};
	uint64_t offset = 0;
	while (offset < payload.size()) {
		vector<byte> fragmentData;
		FragmentType fragmentType;
		if (offset == 0) {
			fragmentType = FragmentType::Start;
		} else if (offset + maximumFragmentSize < payload.size()) {
			fragmentType = FragmentType::Middle;
		} else {
			if (offset + maximumFragmentSize > payload.size()) {
				maximumFragmentSize = payload.size() - offset;
			}
			fragmentType = FragmentType::End;
		}
		fragmentData = {payload.begin() + offset, payload.begin() + offset + maximumFragmentSize};
		auto fragment = std::make_shared<NalUnitFragmentA>(fragmentType, f, nri, naluType, fragmentData);
		result.push_back(fragment);
		offset += maximumFragmentSize;
	}
	return result;
}

void NalUnitFragmentA::setFragmentType(FragmentType type) {
	fragmentHeader()->setReservedBit6(false);
	switch (type) {
		case FragmentType::Start:
			fragmentHeader()->setStart(true);
			fragmentHeader()->setEnd(false);
			break;
		case FragmentType::End:
			fragmentHeader()->setStart(false);
			fragmentHeader()->setEnd(true);
			break;
		default:
			fragmentHeader()->setStart(false);
			fragmentHeader()->setEnd(false);
	}
}

std::vector<shared_ptr<binary>> NalUnits::generateFragments(uint16_t maximumFragmentSize) {
	vector<shared_ptr<binary>> result{};
	for (auto nalu : *this) {
		if (nalu->size() > maximumFragmentSize) {
			std::vector<shared_ptr<NalUnitFragmentA>> fragments =
			NalUnitFragmentA::fragmentsFrom(nalu, maximumFragmentSize);
			result.insert(result.end(), fragments.begin(), fragments.end());
		} else {
			result.push_back(nalu);
		}
	}
	return result;
}

} // namespace rtc

#endif /* RTC_ENABLE_MEDIA */
