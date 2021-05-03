#ifndef _WINSOCK_ERROR_HPP
#define _WINSOCK_ERROR_HPP


#include <sstream>
#include <stdexcept>

#include <WinSock2.h>


namespace winsock_error {
[[noreturn]] void throw_winsock_error()
{
	// Throw an appropriate exception if an error occurs in winsock.
	std::ostringstream message;
	message << "error_code: " << ::WSAGetLastError();
	throw std::runtime_error(message.str());
}
}  // namespace winsock_error


#endif  // _WINSOCK_ERROR_HPP
