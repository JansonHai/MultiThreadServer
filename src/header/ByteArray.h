#ifndef HEADER_BYTEARRAY_H_
#define HEADER_BYTEARRAY_H_

#include <stdio.h>
#include <string>
#include <stdint.h>
#include "buffer.h"

class ReadByteArray
{
private:
	struct fl_buffer * m_buffer;
	int m_size;
	int m_cur_pos;

public:
	ReadByteArray();
	~ReadByteArray();

	bool SetReadContent(const char * str, int size);
	void ResetReadPos();
	void ReadSeek(int pos);
	char * GetBuffer() const;
	int GetArraySize();
	void ReleaseBuffer();

	char ReadInt8();
	int16_t ReadInt16();
	int32_t ReadInt32();
	int64_t ReadInt64();
	unsigned char ReadUInt8();
	uint16_t ReadUInt16();
	uint32_t ReadUInt32();
	uint64_t ReadUInt64();
	float ReadFloat();
	double ReadDouble();
	std::string ReadString();

};

class WriteByteArray
{
private:
	struct fl_buffer * m_buffer;
	int m_size;

public:
	WriteByteArray();
	~WriteByteArray();

	void InitBuffer(int size = 2048);
	void ResetWrite();
	void WriteSeek(int pos);
	char * GetBuffer() const;
	int GetArraySize();
	void ReleaseBuffer();

	bool WriteInt8(char i8);
	bool WriteInt16(int16_t i16);
	bool WriteInt32(int32_t i32);
	bool WriteInt64(int64_t i64);
	bool WriteUInt8(unsigned char ui8);
	bool WriteUInt16(uint16_t ui16);
	bool WriteUInt32(uint32_t ui32);
	bool WriteUInt64(uint64_t ui64);
	bool WriteFloat(float f);
	bool WriteDouble(double d);
	bool WriteString(const char * str, int len);

};

#endif /* HEADER_BYTEARRAY_H_ */
