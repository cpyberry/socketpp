/*
Copyright 2021 cpyberry
https://github.com/cpyberry/socketpp

cpyberry
email: cpyberry222@gmail.com
github: https://github.com/cpyberry
*/


#ifndef _WINSOCK_ERROR_HPP
#define _WINSOCK_ERROR_HPP


#include <map>
#include <stdexcept>
#include <string>

#include <WinSock2.h>


namespace winsock_error {
const std::map<const int, const std::string> ERROR_CONTENT = {
};


std::string get_error_content(const int& error_code)
{
	try {
		const std::string& content = ERROR_CONTENT.at(error_code);
		return content;
	} catch(std::out_of_range& error) {
		return std::to_string(error_code);
	}
}


[[noreturn]] void throw_winsock_error()
{
	// Throw an appropriate exception if an error occurs in winsock.
	int error_code = ::WSAGetLastError();
	throw std::runtime_error(get_error_content(error_code));
}
}  // namespace winsock_error


#endif  // _WINSOCK_ERROR_HPP
