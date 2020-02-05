#ifdef __ORBIS__
#include "Net/Packet.h"
#include "enet/enet.h"
#include <cstring>
#include <cwchar>
#include <memory/string.h>

Packet::Packet() : m_readPos(0), m_sendPos(0), m_isValid(true)
{

}


Packet::~Packet()
{
}


void Packet::Append(const void* data, std::size_t sizeInBytes)
{
	if (data && (sizeInBytes > 0))
	{
		std::size_t start = m_data.size();
		m_data.resize(start + sizeInBytes);
		std::memcpy(&m_data[start], data, sizeInBytes);
	}
}


void Packet::Clear()
{
	m_data.clear();
	m_readPos = 0;
	m_isValid = true;
}

const void* Packet::GetData() const
{
	return !m_data.empty() ? &m_data[0] : NULL;
}

const void* Packet::GetData(std::size_t offset) const
{
	return (!m_data.empty() && m_data.size() > offset) ? &m_data[offset] : NULL;
}


std::size_t Packet::GetDataSize() const
{
	return m_data.size();
}

bool Packet::EndOfPacket() const
{
	return m_readPos >= m_data.size();
}

Packet::operator bool() const
{
	return m_isValid;
}


Packet& Packet::operator >>(bool& data)
{
	Uint8 value;
	if (*this >> value)
		data = (value != 0);

	return *this;
}

Packet& Packet::operator >>(Int8& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = *reinterpret_cast<const Int8*>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}


Packet& Packet::operator >>(Uint8& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = *reinterpret_cast<const Uint8*>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}


Packet& Packet::operator >>(Int16& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = sceNetNtohs(*reinterpret_cast<const Int16*>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

Packet& Packet::operator >>(Uint16& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = sceNetNtohs(*reinterpret_cast<const Uint16*>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

Packet& Packet::operator >>(Int32& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = sceNetNtohl(*reinterpret_cast<const Int32*>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

Packet& Packet::operator >>(Uint32& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = sceNetNtohl(*reinterpret_cast<const Uint32*>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

Packet& Packet::operator >>(Int64& data)
{
	if (CheckSize(sizeof(data)))
	{
		// Since sceNetNtohll is not available everywhere, we have to convert
		// to network byte order (big endian) manually
		const Uint8* bytes = reinterpret_cast<const Uint8*>(&m_data[m_readPos]);
		data = (static_cast<Int64>(bytes[0]) << 56) |
			(static_cast<Int64>(bytes[1]) << 48) |
			(static_cast<Int64>(bytes[2]) << 40) |
			(static_cast<Int64>(bytes[3]) << 32) |
			(static_cast<Int64>(bytes[4]) << 24) |
			(static_cast<Int64>(bytes[5]) << 16) |
			(static_cast<Int64>(bytes[6]) << 8) |
			(static_cast<Int64>(bytes[7]));
		m_readPos += sizeof(data);
	}

	return *this;
}


Packet& Packet::operator >>(Uint64& data)
{
	if (CheckSize(sizeof(data)))
	{
		// Since sceNetNtohll is not available everywhere, we have to convert
		// to network byte order (big endian) manually
		const Uint8* bytes = reinterpret_cast<const Uint8*>(&m_data[m_readPos]);
		data = (static_cast<Uint64>(bytes[0]) << 56) |
			(static_cast<Uint64>(bytes[1]) << 48) |
			(static_cast<Uint64>(bytes[2]) << 40) |
			(static_cast<Uint64>(bytes[3]) << 32) |
			(static_cast<Uint64>(bytes[4]) << 24) |
			(static_cast<Uint64>(bytes[5]) << 16) |
			(static_cast<Uint64>(bytes[6]) << 8) |
			(static_cast<Uint64>(bytes[7]));
		m_readPos += sizeof(data);
	}

	return *this;
}


Packet& Packet::operator >>(float& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = *reinterpret_cast<const float*>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}


Packet& Packet::operator >>(double& data)
{
	if (CheckSize(sizeof(data)))
	{
		data = *reinterpret_cast<const double*>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}


Packet& Packet::operator >>(char* data)
{
	// First extract string length
	Uint32 length = 0;
	*this >> length;

	if ((length > 0) && CheckSize(length))
	{
		// Then extract characters
		std::memcpy(data, &m_data[m_readPos], length);
		data[length] = '\0';

		// Update reading position
		m_readPos += length;
	}

	return *this;
}


Packet& Packet::operator >>(str& data)
{
	// First extract string length
	Uint32 length = 0;
	*this >> length;

	data.clear();
	if ((length > 0) && CheckSize(length))
	{
		// Then extract characters
		data.assign(&m_data[m_readPos], length);

		// Update reading position
		m_readPos += length;
	}

	return *this;
}


Packet& Packet::operator >>(wchar_t* data)
{
	// First extract string length
	Uint32 length = 0;
	*this >> length;

	if ((length > 0) && CheckSize(length * sizeof(Uint32)))
	{
		// Then extract characters
		for (Uint32 i = 0; i < length; ++i)
		{
			Uint32 character = 0;
			*this >> character;
			data[i] = static_cast<wchar_t>(character);
		}
		data[length] = L'\0';
	}

	return *this;
}


Packet& Packet::operator >>(wstr& data)
{
	// First extract string length
	Uint32 length = 0;
	*this >> length;

	data.clear();
	if ((length > 0) && CheckSize(length * sizeof(Uint32)))
	{
		// Then extract characters
		for (Uint32 i = 0; i < length; ++i)
		{
			Uint32 character = 0;
			*this >> character;
			data += static_cast<wchar_t>(character);
		}
	}

	return *this;
}


Packet& Packet::operator <<(bool data)
{
	*this << static_cast<Uint8>(data);
	return *this;
}


Packet& Packet::operator <<(Int8 data)
{
	Append(&data, sizeof(data));
	return *this;
}


Packet& Packet::operator <<(Uint8 data)
{
	Append(&data, sizeof(data));
	return *this;
}

Packet& Packet::operator <<(Int16 data)
{
	Int16 toWrite = sceNetHtons(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}


Packet& Packet::operator <<(Uint16 data)
{
	Uint16 toWrite = sceNetHtons(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}


Packet& Packet::operator <<(Int32 data)
{
	Int32 toWrite = sceNetHtonl(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}


Packet& Packet::operator <<(Uint32 data)
{
	Uint32 toWrite = sceNetHtonl(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}


Packet& Packet::operator <<(Int64 data)
{
	// Since htonll is not available everywhere, we have to convert
	// to network byte order (big endian) manually
	Uint8 toWrite[] =
	{
		static_cast<Uint8>((data >> 56) & 0xFF),
		static_cast<Uint8>((data >> 48) & 0xFF),
		static_cast<Uint8>((data >> 40) & 0xFF),
		static_cast<Uint8>((data >> 32) & 0xFF),
		static_cast<Uint8>((data >> 24) & 0xFF),
		static_cast<Uint8>((data >> 16) & 0xFF),
		static_cast<Uint8>((data >> 8) & 0xFF),
		static_cast<Uint8>((data) & 0xFF)
	};
	Append(&toWrite, sizeof(toWrite));
	return *this;
}


Packet& Packet::operator <<(Uint64 data)
{
	// Since htonll is not available everywhere, we have to convert
	// to network byte order (big endian) manually
	Uint8 toWrite[] =
	{
		static_cast<Uint8>((data >> 56) & 0xFF),
		static_cast<Uint8>((data >> 48) & 0xFF),
		static_cast<Uint8>((data >> 40) & 0xFF),
		static_cast<Uint8>((data >> 32) & 0xFF),
		static_cast<Uint8>((data >> 24) & 0xFF),
		static_cast<Uint8>((data >> 16) & 0xFF),
		static_cast<Uint8>((data >> 8) & 0xFF),
		static_cast<Uint8>((data) & 0xFF)
	};
	Append(&toWrite, sizeof(toWrite));
	return *this;
}


Packet& Packet::operator <<(float data)
{
	Append(&data, sizeof(data));
	return *this;
}


Packet& Packet::operator <<(double data)
{
	Append(&data, sizeof(data));
	return *this;
}


Packet& Packet::operator <<(const char* data)
{
	// First insert string length
	Uint32 length = static_cast<Uint32>(std::strlen(data));
	*this << length;

	// Then insert characters
	Append(data, length * sizeof(char));

	return *this;
}


Packet& Packet::operator <<(const str& data)
{
	// First insert string length
	Uint32 length = static_cast<Uint32>(data.size());
	*this << length;

	// Then insert characters
	if (length > 0)
		Append(data.c_str(), length * sizeof(ptl::string::value_type));

	return *this;
}

Packet& Packet::operator <<(const wchar_t* data)
{
	// First insert string length
	Uint32 length = static_cast<Uint32>(std::wcslen(data));
	*this << length;

	// Then insert characters
	for (const wchar_t* c = data; *c != L'\0'; ++c)
		*this << static_cast<Uint32>(*c);

	return *this;
}

Packet& Packet::operator <<(const wstr& data)
{
	// First insert string length
	Uint32 length = static_cast<Uint32>(data.size());
	*this << length;

	// Then insert characters
	if (length > 0)
	{
		for (ptl::wstring::const_iterator c = data.begin(); c != data.end(); ++c)
		{
			*this << static_cast<Uint32>(*c);
		}
	}

	return *this;
}


bool Packet::CheckSize(std::size_t size)
{
	m_isValid = m_isValid && (m_readPos + size <= m_data.size());

	return m_isValid;
}


const void* Packet::OnSend(std::size_t& size)
{
	size = GetDataSize();
	return GetData();
}


void Packet::OnReceive(const void* data, std::size_t size)
{
	Append(data, size);
}


#endif