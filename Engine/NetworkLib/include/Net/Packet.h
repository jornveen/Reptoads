#pragma once

#include <string>
#include <vector>
#include <cstring>

#include "memory/allocator_impl.h"
#include "memory/string.h"

#ifndef PLATFORM_PS
using str = std::string;
using wstr = std::wstring;
#else
using str = ptl::string;
using wstr = ptl::wstring;
#endif

namespace net
{
	class Server;
	class Client;
}

class Packet
{
public:
	Packet();
    Packet(const Packet& a_Other) = default;
    Packet(Packet&& a_Other) noexcept = default;
    Packet& operator=(const Packet& a_Other) = default;
    //Packet& operator=(Packet&& a_Other) noexcept = default;
    virtual ~Packet();

	typedef signed   char Int8;
	typedef unsigned char Uint8;

	// 16 bits integer types
	typedef signed   short Int16;
	typedef unsigned short Uint16;

	// 32 bits integer types
	typedef signed   int Int32;
	typedef unsigned int Uint32;

	// 64 bits integer types
#if defined(_MSC_VER)
	typedef signed   __int64 Int64;
	typedef unsigned __int64 Uint64;
#else
	typedef signed	 long Int64;
	typedef unsigned long Uint64;
#endif

    using is_packet_class = std::true_type;

	friend class net::Server;
	friend class net::Client;

	////////////////////////////////////////////////////////////
	/// \brief Append data to the end of the packet
	///
	/// \param data        Pointer to the sequence of bytes to append
	/// \param sizeInBytes Number of bytes to append
	///
	/// \see clear
	///
	////////////////////////////////////////////////////////////
	void Append(const void* data, std::size_t sizeInBytes);

	////////////////////////////////////////////////////////////
	/// \brief Clear the packet
	///
	/// After calling Clear, the packet is empty.
	///
	/// \see append
	///
	////////////////////////////////////////////////////////////
	void Clear();

	////////////////////////////////////////////////////////////
	/// \brief Get a pointer to the data contained in the packet
	///
	/// Warning: the returned pointer may become invalid after
	/// you append data to the packet, therefore it should never
	/// be stored.
	/// The return pointer is NULL if the packet is empty.
	///
	/// \return Pointer to the data
	///
	/// \see getDataSize
	///
	////////////////////////////////////////////////////////////
	const void* GetData() const;

	////////////////////////////////////////////////////////////
	/// \brief Get a pointer to the data contained in the packet
	///
	/// \param offset        offset
	/// Warning: the returned pointer may become invalid after
	/// you append data to the packet, therefore it should never
	/// be stored.
	/// The return pointer is NULL if the packet is empty.
	///
	/// \return Pointer to the data
	///
	/// \see getDataSize
	///
	////////////////////////////////////////////////////////////
	const void* GetData(std::size_t offset) const;

    template<typename T>
    void Out(T& data);
    template<typename T>
    void Out(T& data, std::size_t offset);

	////////////////////////////////////////////////////////////
	/// \brief Get the size of the data contained in the packet
	///
	/// This function returns the number of bytes pointed to by
	/// what getData returns.
	///
	/// \return Data size, in bytes
	///
	/// \see getData
	///
	////////////////////////////////////////////////////////////
	std::size_t GetDataSize() const;

	////////////////////////////////////////////////////////////
	/// \brief Tell if the reading position has reached the
	///        end of the packet
	///
	/// This function is useful to know if there is some data
	/// left to be read, without actually reading it.
	///
	/// \return True if all data was read, false otherwise
	///
	/// \see operator bool
	///
	////////////////////////////////////////////////////////////
	bool EndOfPacket() const;
    void Reset();
public:

	////////////////////////////////////////////////////////////
	/// \brief Test the validity of the packet, for reading
	///
	/// This operator allows to test the packet as a boolean
	/// variable, to check if a reading operation was successful.
	///
	/// A packet will be in an invalid state if it has no more
	/// data to read.
	///
	/// This behavior is the same as standard C++ streams.
	///
	/// Usage example:
	/// \code
	/// float x;
	/// packet >> x;
	/// if (packet)
	/// {
	///    // ok, x was extracted successfully
	/// }
	///
	/// // -- or --
	///
	/// float x;
	/// if (packet >> x)
	/// {
	///    // ok, x was extracted successfully
	/// }
	/// \endcode
	///
	/// Don't focus on the return type, it's equivalent to bool but
	/// it disallows unwanted implicit conversions to integer or
	/// pointer types.
	///
	/// \return True if last data extraction from packet was successful
	///
	/// \see endOfPacket
	///
	////////////////////////////////////////////////////////////
	operator bool() const;

	////////////////////////////////////////////////////////////
	/// Overload of operator >> to read data from the packet
	///
	////////////////////////////////////////////////////////////
	Packet& operator >>(bool&         data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Int8&         data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Uint8&        data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Int16&        data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Uint16&       data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Int32&        data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Uint32&       data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Int64&        data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(Uint64&       data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(float&        data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(double&       data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(char*         data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(str&  data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(wchar_t*      data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator >>(wstr& data);

	////////////////////////////////////////////////////////////
	/// Overload of operator << to write data into the NetPacket
	///
	////////////////////////////////////////////////////////////
	Packet& operator <<(bool                data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Int8                data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Uint8               data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Int16               data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Uint16              data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Int32               data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Uint32              data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Int64               data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(Uint64              data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(float               data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(double              data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(const char*         data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(const str&  data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(const wchar_t*      data);

	////////////////////////////////////////////////////////////
	/// \overload
	////////////////////////////////////////////////////////////
	Packet& operator <<(const wstr& data);


/*
    template<typename STR_TYPE, typename = std::enable_if_t<std::is_same<typename STR_TYPE::allocator_type, ptl::Allocator<typename STR_TYPE::value_type>>::value>>
    Packet& operator >> (STR_TYPE& str) {
        using  std_str = std::basic_string<typename STR_TYPE::value_type, typename STR_TYPE::traits_type>;
        std_str str_;
        *this >> str_;
        str.assign(str_.data());
        return *this;
    }

    template<typename STR_TYPE, typename = std::enable_if_t<std::is_same<typename STR_TYPE::allocator_type, ptl::Allocator<typename STR_TYPE::value_type>>::value>>
    Packet& operator << (STR_TYPE str) {
        using  std_str = std::basic_string<typename STR_TYPE::value_type, typename STR_TYPE::traits_type>;
        std_str str_{ str.data() };
        return *this << str_;
    }

*/
protected:

	////////////////////////////////////////////////////////////
	/// \brief Called before the packet is sent over the network
	///
	/// This function can be defined by derived classes to
	/// transform the data before it is sent; this can be
	/// used for compression, encryption, etc.
	/// The function must return a pointer to the modified data,
	/// as well as the number of bytes pointed.
	/// The default implementation provides the packet's data
	/// without transforming it.
	///
	/// \param size Variable to fill with the size of data to send
	///
	/// \return Pointer to the array of bytes to send
	///
	/// \see onReceive
	///
	////////////////////////////////////////////////////////////
	virtual const void* OnSend(std::size_t& size);

	////////////////////////////////////////////////////////////
	/// \brief Called after the packet is received over the network
	///
	/// This function can be defined by derived classes to
	/// transform the data after it is received; this can be
	/// used for decompression, decryption, etc.
	/// The function receives a pointer to the received data,
	/// and must fill the packet with the transformed bytes.
	/// The default implementation fills the packet directly
	/// without transforming the data.
	///
	/// \param data Pointer to the received bytes
	/// \param size Number of bytes
	///
	/// \see onSend
	///
	////////////////////////////////////////////////////////////
	virtual void OnReceive(const void* data, std::size_t size);

private:

	////////////////////////////////////////////////////////////
	/// Disallow comparisons between packets
	///
	////////////////////////////////////////////////////////////
	bool operator ==(const Packet& right) const;
	bool operator !=(const Packet& right) const;

	////////////////////////////////////////////////////////////
	/// \brief Check if the packet can extract a given number of bytes
	///
	/// This function updates accordingly the state of the packet.
	///
	/// \param size Size to check
	///
	/// \return True if \a size bytes can be read from the packet
	///
	////////////////////////////////////////////////////////////
	bool CheckSize(std::size_t size) ;

	////////////////////////////////////////////////////////////
	// Member data
	////////////////////////////////////////////////////////////
	std::vector<char> m_data;    ///< Data stored in the packet
	std::size_t       m_readPos; ///< Current reading position in the packet
	std::size_t       m_sendPos; ///< Current send position in the packet (for handling partial sends)
	bool              m_isValid; ///< Reading state of the packet

};

template <typename T>
void Packet::Out(T& data)
{
    std::memcpy(&data, &m_data[0], sizeof(T));
}

template <typename T>
void Packet::Out(T& data, std::size_t offset)
{
    std::memcpy(&data, &m_data[offset], sizeof(T));
}
