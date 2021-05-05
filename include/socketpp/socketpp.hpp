/*
Copyright 2021 cpyberry
https://github.com/cpyberry/socketpp

cpyberry
email: cpyberry222@gmail.com
github: https://github.com/cpyberry
*/


#ifndef _SOCKETPP_HPP
#define _SOCKETPP_HPP


#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

#include <winsock2.h>

#include "socketpp/winsock_error.hpp"


namespace socketpp {
class Address
{
public:
	Address(const std::string_view& new_ip_address, const std::uint16_t& new_port, const int& new_address_family) :
		_ip_address(new_ip_address), _port(new_port), _address_family(new_address_family) {}

	std::string_view ip_address() const noexcept
	{
		return this->_ip_address;
	}

	void ip_address(const std::string_view& new_ip_address) noexcept
	{
		this->_ip_address = new_ip_address;
	}

	std::uint16_t port() const noexcept
	{
		return this->_port;
	}

	void port(const std::uint16_t& new_port) noexcept
	{
		this->_port = new_port;
	}

	int address_family() const noexcept
	{
		return this->_address_family;
	}

	void address_family(const int& new_address_family) noexcept
	{
		this->_address_family = new_address_family;
	}

	unsigned long inet_addr() const noexcept
	{
		return ::inet_addr(this->_ip_address.data());
	}

	unsigned short htons() const noexcept
	{
		return ::htons(this->_port);
	}

	sockaddr_in to_sockaddr_in() const noexcept
	{
		sockaddr_in address;
		address.sin_addr.S_un.S_addr = this->inet_addr();
		address.sin_family = this->address_family();
		address.sin_port = this->htons();
		return address;
	}

private:
	std::string_view _ip_address;
	std::uint16_t _port;
	int _address_family;
};


class Socket
{
public:
	Socket(const int& protocol_family, const int& socket_type) : protocol_family(protocol_family)
	{
		// When using this library, use this constructor.
		this->wsa_data = this->_initial_wsadata();
		this->sock = this->_create_socket(protocol_family, socket_type);
		this->address = {};
		this->cleanuped = false;
	}

	Socket(SOCKET&& sock, sockaddr_in&& address) : sock(sock), address(address)
	{
		// when creating a client socket object, use this constructor.
		this->wsa_data = this->_initial_wsadata();
		this->cleanuped = false;
	}

	~Socket()
	{
		if (!this->cleanuped) {
			this->cleanup();
		}
	}

	Socket(const Socket& sock)
	{
		this->sock = sock.sock;
		this->address = sock.address;
		this->protocol_family = sock.protocol_family;
		this->cleanuped = sock.cleanuped;
		this->wsa_data = this->_initial_wsadata();
	}

	Socket(Socket&& sock)
	{
		this->sock = std::move(sock.sock);
		this->address = std::move(sock.address);
		this->protocol_family = std::move(sock.protocol_family);
		this->cleanuped = std::move(sock.cleanuped);
		this->wsa_data = this->_initial_wsadata();
	}

	void bind(const std::string_view& ip_address, const std::uint16_t& port)
	{
		this->_set_address(ip_address, port);

		int result = ::bind(this->sock, reinterpret_cast<sockaddr*>(&this->address), sizeof(this->address));
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
	}

	void connect(const std::string_view& ip_address, const std::uint16_t& port)
	{
		this->_set_address(ip_address, port);

		int result = ::connect(this->sock, reinterpret_cast<sockaddr*>(&this->address), sizeof(this->address));
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
	}

	void listen(const int& backlog) const
	{
		int result = ::listen(this->sock, backlog);
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
	}

	template <class BufferType, class BufferPointerType = decltype(std::declval<BufferType>().data())>
	auto setsockopt(const int& level, const int& optname, const BufferType& buffer) -> decltype(buffer.size(), void())
	{
		// If buffer argument is an instance of a class whose member functions are data() and size()
		int result = ::setsockopt(this->sock, level, optname, buffer.data(), buffer.size());
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
	}

	template <class BufferType>
	auto setsockopt(const int& level, const int& optname, const BufferType& buffer) -> decltype(static_cast<char>(buffer), void())
	{
		// If the type specified in buffer can be converted to char.
		int result = ::setsockopt(this->sock, level, optname, reinterpret_cast<const char*>(&buffer), sizeof(buffer));
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
	}

	[[noreturn]] auto setsockopt(...) -> void
	{
		// If anything else is stored in the buffer argument.
		throw std::invalid_argument("The buffer argument must be of type castable to const char* or an instance of a class with data and size member functions.");
	}

	std::pair<Socket, sockaddr_in> accept() const
	{
		sockaddr_in client_address = {};
		int size = static_cast<int>(sizeof(client_address));

		SOCKET client_sock = ::accept(this->sock, reinterpret_cast<sockaddr*>(&client_address), &size);

		if (client_sock == INVALID_SOCKET) {
			winsock_error::throw_winsock_error();
		}

		Socket client(std::move(client_sock), std::move(client_address));
		return std::make_pair(client, client_address);
	}

	template <class BufferType>
	int send(const BufferType& buffer) const
	{
		int sended_size = ::send(this->sock, buffer.data(), buffer.size(), 0);
		if (sended_size == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
		return sended_size;
	}

	template <class BufferType>
	int sendto(const BufferType& buffer, const std::string_view& ip_address, const std::uint16_t& port)
	{
		sockaddr_in address_to = this->_get_address(ip_address, port);
		size_t size = sizeof(address_to);

		int sended_size = ::sendto(
			this->sock, buffer.data(), buffer.size(), 0,
			reinterpret_cast<sockaddr*>(&address_to), size);
		if (sended_size == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
		return sended_size;
	}

	template <int size>
	std::array<char, size> recv() const
	{
		std::array<char, size> buffer;
		int recved_size = ::recv(this->sock, buffer.data(), size, 0);
		if (recved_size == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
		return buffer;
	}

	template <int size>
	std::pair<std::array<char, size>, sockaddr_in> recvfrom() const
	{
		std::array<char, size> buffer;
		sockaddr_in client_address;
		int address_size = static_cast<int>(sizeof(client_address));

		int recved_size = ::recvfrom(
			this->sock, buffer.data(), size, 0,
			reinterpret_cast<sockaddr*>(&client_address), &address_size);
		if (recved_size == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
		return std::make_pair(buffer, client_address);
	}

	void close() const
	{
		int result = ::closesocket(this->sock);
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
	}

	void cleanup()
	{
		int result = ::WSACleanup();
		if (result == SOCKET_ERROR) {
			winsock_error::throw_winsock_error();
		}
		this->cleanuped = true;
	}

private:
	SOCKET sock;
	WSADATA wsa_data;
	sockaddr_in address;
	int protocol_family;
	bool cleanuped;

	WSADATA _initial_wsadata(const int& major_version = 2, const int& minor_version = 0) const
	{
		WSADATA sock;
		int result = ::WSAStartup(MAKEWORD(major_version, minor_version), &sock);
		if (result) {
			winsock_error::throw_winsock_error();
		}
		return sock;
	}

	SOCKET _create_socket(const int& protocol_family, const int& socket_type) const
	{
		SOCKET sock = ::socket(protocol_family, socket_type, 0);
		if (sock == INVALID_SOCKET) {
			winsock_error::throw_winsock_error();
		}
		return sock;
	}

	sockaddr_in _get_address(const std::string_view& ip_address, const std::uint16_t& port) const noexcept
	{
		sockaddr_in address;
		address.sin_addr.S_un.S_addr = ::inet_addr(ip_address.data());
		address.sin_family = this->protocol_family;
		address.sin_port = ::htons(port);
		return address;
	}

	void _set_address(const std::string_view& ip_address, const std::uint16_t& port) noexcept
	{
		this->address = std::move(this->_get_address(ip_address, port));
	}
};
}  // namespace socketpp


#endif  // _SOCKETPP_HPP
