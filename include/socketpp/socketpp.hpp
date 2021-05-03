#ifndef _SOCKETPP_HPP
#define _SOCKETPP_HPP


#include <array>
#include <cstdint>
#include <string_view>
#include <utility>

#include <winsock2.h>

#include "socketpp/winsock_error.hpp"


namespace socketpp {
class Socket
{
public:
	Socket(const int& protocol_family, const int& socket_type) : protocol_family(protocol_family)
	{
		// When using this library, use this constructor.
		this->wsa_data = this->_initial_wsadata();
		this->sock = this->_create_socket(protocol_family, socket_type);
		this->address = {};
	}

	Socket(SOCKET&& sock, sockaddr_in&& address) : sock(sock), address(address)
	{
		// when creating a client socket object, use this constructor.
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

private:
	SOCKET sock;
	WSADATA wsa_data;
	sockaddr_in address;
	int protocol_family;

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

	void _set_address(const std::string_view& ip_address, const std::uint16_t& port) noexcept
	{
		this->address.sin_addr.S_un.S_addr = ::inet_addr(ip_address.data());
		this->address.sin_family = this->protocol_family;
		this->address.sin_port = ::htons(port);
	}
};
}  // namespace socketpp


#endif  // _SOCKETPP_HPP
