#ifndef _SOCKETPP_HPP
#define _SOCKETPP_HPP


#include <cstdint>
#include <string_view>

#include <winsock2.h>

#include "socketpp/winsock_error.hpp"


namespace socketpp {
class Socket
{
private:
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
