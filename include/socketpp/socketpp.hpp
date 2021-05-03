#ifndef _SOCKETPP_HPP
#define _SOCKETPP_HPP


#include <winsock2.h>

#include "socketpp/winsock_error.hpp"


namespace socketpp {
class Socket
{
private:
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
};
}  // namespace socketpp


#endif  // _SOCKETPP_HPP
