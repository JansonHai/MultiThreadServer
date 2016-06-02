#include <string.h>
#include "BitRecord.h"

BitRecord::BitRecord()
{
	len = 0;
	bits = NULL;
}

BitRecord::~BitRecord()
{
	ReleaseBits();
}

void BitRecord::SetBitsLength(int length)
{
	if (length < 0) return;
	if (NULL == bits)
	{
		len = (length / 8) + 1;
		bits = (unsigned char *)malloc(len * sizeof(unsigned char));
		ResetAllBit();
	}
	else
	{
		int newLength = (length / 8) + 1;
		unsigned char * tmp = (unsigned char *)malloc(newLength * sizeof(unsigned char));
		for (int i=0;i<newLength;++i) tmp[i] = 0x00;
		memcpy(tmp, bits, len);
		ReleaseBits();
		len = newLength;
		bits = tmp;
	}
}

void BitRecord::ReleaseBits()
{
	if (NULL != bits)
	{
		free(bits);
	}
	bits = NULL;
	len = 0;
}

void BitRecord::SetBit(int pos)
{
	if (pos < 0) return;
	int bitPos = pos / 8;
	if (bitPos >= len) return;
	unsigned char mod = (pos % 8) & 0xff;
	unsigned char flag = 0x01;
	while (mod > 0)
	{
		flag <<= 1;
		--mod;
	}
	bits[bitPos] |= flag;
}

void BitRecord::ResetBit(int pos)
{
	if (pos < 0) return;
	int bitPos = pos / 8;
	if (bitPos >= len) return;
	unsigned char mod = (pos % 8) & 0xff;
	unsigned char flag = 0x01;
	while (mod > 0)
	{
		flag <<= 1;
		--mod;
	}
	bits[bitPos] &= ~flag;
}

void BitRecord::ResetAllBit()
{
	for (int i=0;i<len;++i)
	{
		bits[i] = 0x00;
	}
}

bool BitRecord::IsSetBit(int pos)
{
	if (pos < 0) return true;
	int bitPos = pos / 8;
	if (bitPos >= len) return true;
	unsigned char mod = (pos % 8) & 0xff;
	unsigned char flag = 1 << mod;
	return 0 != (bits[bitPos] & flag);
}

int BitRecord::GetUnSetBitPosition()
{
	int i = 0;
	while (i < len && 0xff != (bits[i] & 0xff)) ++i;
	if (i >= len)
	{
		return -1;
	}
	int pos = 8 * i;
	unsigned char bit = bits[i];
	while (0 == (bit & 0x01))
	{
		++pos;
		bit >>= 1;
	}
	return pos;
}
