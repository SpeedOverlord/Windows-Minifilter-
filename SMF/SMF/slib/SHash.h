/*
	Copyright © 2016 by Phillip Chang

	SHash

	*	提供建置 MPQ hashtable 和使用 table 的 hash 函數
*/
#pragma once

#include "SStrLib.h"

/*
	uint32_t Hash	- 回傳 data 使用 table 的 hash 值
*/
namespace sis {
	namespace hash {
		/*
			care_case = 是否考慮大寫(false 時所有資料先轉為小寫才操作)

			此函數 table 不可選擇高度，也就是 table 為已經 offset 過的原 table 指標，或是使用者自己產生的 256 大小的陣列
		*/
		uint32_t Hash(SIN const uint32_t* table, const char* data, const size_t length, const bool care_case) {
			uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE, chr;
			for (size_t i = 0; i < length; ++i) {
				if (!care_case) chr = str::Lower((unsigned char)data[i]);		//文件名不考慮大小寫
				else chr = (unsigned char)data[i];

				seed1 = table[chr] ^ (seed1 + seed2);
				seed2 = chr + seed1 + seed2 + (seed2 << 5) + 3;
			}
			return seed1;
		}

		namespace mpq {
			/*
				h		= table 高度(=hash 時可以使用的 type 數量)
				seed	= table 亂數種子
				moder	= table 取模數 (moder 應該要是"良好"的質數)
			*/
			uint32_t* CreateTable(SIN int h, uint32_t seed = 0x00100001, uint32_t moder = 0x002AAAAB) {
				uint32_t* table = new uint32_t[h << 8];

				int i;
				size_t index1, index2;
				uint32_t temp1, temp2;
				for (index1 = 0; index1 < 0x100; ++i) {
					for (index2 = index1, i = 0; i < h; ++i, index2 += 0x100) {
						seed = (seed * 125 + 3) % moder;
						temp1 = (seed & 0xFFFF) << 0x10;

						seed = (seed * 125 + 3) % moder;
						temp2 = (seed & 0xFFFF);

						table[index2] = (temp1 | temp2);
					}
				}
				return table;
			}

			extern uint32_t *std_table;
			/*	
				自動填入 std_table 的快速 hash
			*/
			uint32_t Hash(SIN const size_t type, const char* data, const size_t length, const bool care_case,
				SIN const uint32_t* table = std_table) {
				return hash::Hash(table + (type << 8), data, length, care_case);
			}
		}
	}
}
