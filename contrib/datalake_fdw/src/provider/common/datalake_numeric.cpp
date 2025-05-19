#include "datalake_numeric.h"

#define NUMERIC_CAN_BE_SHORT(scale,weight) \
	((scale) <= NUMERIC_SHORT_DSCALE_MAX && \
	(weight) <= NUMERIC_SHORT_WEIGHT_MAX && \
	(weight) >= NUMERIC_SHORT_WEIGHT_MIN)

int fill_numeric_result(NumericVar *var, Numeric result)
{
	NumericDigit *digits = var->digits;
	int			weight = var->weight;
	int			sign = var->sign;
	uint32_t 	n;
	Size		len;
	n = var->ndigits;

	/* truncate leading zeroes */
	while (n > 0 && *digits == 0)
	{
		digits++;
		weight--;
		n--;
	}
	/* truncate trailing zeroes */
	while (n > 0 && digits[n - 1] == 0)
		n--;

	/* If zero result, force to weight=0 and positive sign */
	if (n == 0)
	{
		weight = 0;
		sign = NUMERIC_POS;
	}

	/* Build the result */
	if (NUMERIC_CAN_BE_SHORT(var->dscale, weight))
	{
		len = NUMERIC_HDRSZ_SHORT + n * sizeof(NumericDigit);
		SET_VARSIZE(result, len);
		result->choice.n_short.n_header =
			(sign == NUMERIC_NEG ? (NUMERIC_SHORT | NUMERIC_SHORT_SIGN_MASK)
			 : NUMERIC_SHORT)
			| (var->dscale << NUMERIC_SHORT_DSCALE_SHIFT)
			| (weight < 0 ? NUMERIC_SHORT_WEIGHT_SIGN_MASK : 0)
			| (weight & NUMERIC_SHORT_WEIGHT_MASK);
	}
	else
	{
		len = NUMERIC_HDRSZ + n * sizeof(NumericDigit);
		SET_VARSIZE(result, len);
		result->choice.n_long.n_sign_dscale =
			sign | (var->dscale & NUMERIC_DSCALE_MASK);
		result->choice.n_long.n_weight = weight;
	}

	Assert(NUMERIC_NDIGITS(result) == n);
	if (n > 0)
		memcpy(NUMERIC_DIGITS(result), digits, n * sizeof(NumericDigit));

	/* Check for overflow of int16 fields */
	if (NUMERIC_WEIGHT(result) != weight ||
		NUMERIC_DSCALE(result) != var->dscale)
	{
		ereport(ERROR,
				(errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
					errmsg("value overflows numeric format")));
	}
    return len;
}

int128 FLBA_to_int128(const uint8 *bytes, int length)
{
	static constexpr int32_t kMinDecimalBytes = 1;
	static constexpr int32_t kMaxDecimalBytes = 16;

	if (length < kMinDecimalBytes || length > kMaxDecimalBytes)
	{
		elog(ERROR, "overflow");
	}

	const bool is_negative = static_cast<int8_t>(bytes[0]) < 0;
	int128 result = 0;
	if (is_negative)
	{
		memset(&result, 0xff, sizeof(result));
	}
	memcpy(reinterpret_cast<uint8_t*>(&result) + kMaxDecimalBytes - length, bytes, length);
	int128 high_bits = PARQUET_ARROW_BYTE_SWAP64((result >> 64) & 0xffffffffffffffffL);
	int128 low_bits = PARQUET_ARROW_BYTE_SWAP64(result & 0xffffffffffffffffL);
	result = high_bits + (low_bits << 64);
	return result;
}
