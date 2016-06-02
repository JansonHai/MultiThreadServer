#ifndef HEADER_BITRECORD_H_
#define HEADER_BITRECORD_H_

#include <stdio.h>

class BitRecord
{
private:
	int len;
	unsigned char * bits;

public:
	BitRecord();
	~BitRecord();
	void SetBitsLength(int length);
	void ReleaseBits();
	void SetBit(int pos);
	void ResetBit(int pos);
	void ResetAllBit();
	bool IsSetBit(int pos);
	int GetUnSetBitPosition();
};


#endif /* HEADER_BITRECORD_H_ */
