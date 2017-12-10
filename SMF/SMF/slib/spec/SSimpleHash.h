#pragma once

#include "slib\SBase.h"
#include "slib\SStrLib.h"

class SHasher {
public:
	SHasher(SIN int h = 3, uint32_t seed = 0x00100001, uint32_t moder = 0x002AAAAB) {
		table_ = new uint32_t[h << 8];

		int i;
		size_t index1, index2;
		uint32_t temp1, temp2;
		for (index1 = 0; index1 < 0x100; ++index1) {
			for (index2 = index1, i = 0; i < h; ++i, index2 += 0x100) {
				seed = (seed * 125 + 3) % moder;
				temp1 = (seed & 0xFFFF) << 0x10;

				seed = (seed * 125 + 3) % moder;
				temp2 = (seed & 0xFFFF);

				table_[index2] = (temp1 | temp2);
			}
		}
	}
	~SHasher() {
		delete[]table_;
	}
	uint32_t Hash(SIN const char* data, const size_t length, const bool care_case, const int type) {
		uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE, chr;
		for (size_t i = 0; i < length; ++i) {
			if (!care_case) {
				//文件名不考慮大小寫
				if (data[i] >= 'A' && data[i] <= 'Z') chr = (unsigned char)(data[i] - 'A' + 'a');
				else chr = (unsigned char)data[i];
			}
			else chr = (unsigned char)data[i];

			seed1 = table_[chr + (type << 8)] ^ (seed1 + seed2);
			seed2 = chr + seed1 + seed2 + (seed2 << 5) + 3;
		}
		return seed1;
	}
private:
	uint32_t *table_;
};
