#pragma once

#include <parquet/arrow/util/endian.h>

extern "C"
{
#include "utils/numeric.h"
}

template<typename T>
struct IntDigitsTraits;

template<> struct IntDigitsTraits<int32_t> {
	static constexpr int digits = 10 / DEC_DIGITS + 1; // int32_digits
};

template<> struct IntDigitsTraits<int64_t> {
	static constexpr int digits = 19 / DEC_DIGITS + 1; // int64_digits
};

template<> struct IntDigitsTraits<__int128> {
	static constexpr int digits = 40 / DEC_DIGITS + 1; // int128_digits
};

int fill_numeric_result(NumericVar *var, Numeric result);
int128 FLBA_to_int128(const uint8 *bytes, int length);

template<typename T>
int int_to_numeric_with_scale(T val, int scale, Numeric dest)
{
	static constexpr __int128 powers_of_10[] = {
        1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL,
        1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL, 10000000000ULL,
        100000000000ULL, 1000000000000ULL, 10000000000000ULL, 100000000000000ULL, 1000000000000000ULL,
        10000000000000000ULL, 100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL, 
        ((__int128)10000000000000000000ULL * 10ULL), ((__int128)10000000000000000000ULL * 100ULL),
        ((__int128)10000000000000000000ULL * 1000ULL),  ((__int128)10000000000000000000ULL * 10000ULL),
        ((__int128)10000000000000000000ULL * 100000ULL),  ((__int128)10000000000000000000ULL * 1000000ULL),
        ((__int128)10000000000000000000ULL * 10000000ULL),  ((__int128)10000000000000000000ULL * 100000000ULL),
        ((__int128)10000000000000000000ULL * 1000000000ULL),  ((__int128)10000000000000000000ULL * 10000000000ULL),
        ((__int128)10000000000000000000ULL * 100000000000ULL),  ((__int128)10000000000000000000ULL * 1000000000000ULL),
        ((__int128)10000000000000000000ULL * 10000000000000ULL),  ((__int128)10000000000000000000ULL * 100000000000000ULL),
        ((__int128)10000000000000000000ULL * 1000000000000000ULL),  ((__int128)10000000000000000000ULL * 10000000000000000ULL),
        ((__int128)10000000000000000000ULL * 100000000000000000ULL),  ((__int128)10000000000000000000ULL * 1000000000000000000ULL),
        ((__int128)10000000000000000000ULL * 10000000000000000000ULL)
	};

    static constexpr int16 scale_factors[] = {
        1, 1, 10, 100, 1000, 10000
    };

    NumericVar numeric;

    init_numeric_var(&numeric);
    alloc_numeric_var(&numeric, IntDigitsTraits<T>::digits);

	bool is_negative = val < 0;
	numeric.sign = is_negative ? NUMERIC_NEG : NUMERIC_POS;
	val = is_negative ? -val : val;
	numeric.dscale = scale;

	T temp = val;
	int nweight = 0;

    for (int i = 1; i < 39; ++i)
    {
        if (temp < powers_of_10[i])
        {
            nweight = i;
            break;
        }
    }

	NumericDigit *ptr = numeric.digits + numeric.ndigits;

    int dweight = nweight - scale - 1;
	int weight = dweight < 0 ? (dweight + 1) / DEC_DIGITS - 1 : dweight / DEC_DIGITS;
    int offset = (weight + 1) * DEC_DIGITS - (dweight + 1);
    int scale_padding = (offset + nweight) % DEC_DIGITS;
    bool padding_done = scale_padding == 0;
	int ndigits = 0;
	while (val) {
		ptr--;
		ndigits++;

        if (!padding_done)
		{
            int16 pow_padding = 10 * scale_factors[scale_padding];
            int16 pow_padding_remain = 10 * scale_factors[DEC_DIGITS - scale_padding];
            
            temp = val / pow_padding;
            *ptr = (val - (temp * pow_padding)) * pow_padding_remain;
            val = temp;
            padding_done = true;
		}
        else
        {
            temp = val / NBASE;
            *ptr = val - (temp * NBASE);
            val = temp;
        }
	}

	numeric.digits = ptr;
	numeric.ndigits = ndigits;
	numeric.weight = weight;
    int len = fill_numeric_result(&numeric, dest);
    free_numeric_var(&numeric);
    return len;
}
