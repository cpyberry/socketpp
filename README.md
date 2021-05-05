# socketpp

Simple and modern socket library for C++

## Requirement

* C++17

## Usage

When the server receives the message `{1, 2, 3, 4, 5}` from the client and then sends the message `{1, 2, 3, 4, 5}` to the client.

For example, We set the IP address and port of server and client as follows.

```c++
std::string_view server_ip = "0.0.0.0";
std::uint16_t server_port = 50000;

std::string_view client_ip = "0.0.0.0";
std::uint16_t client_port = 50000;

// data to send to each other
std::vector<char> data = {1, 2, 3, 4, 5};

// buffer size when sending and receiving
int size = 1024
```

TCP server

```c++
#include <array>
#include <iostream>
#include <string_view>
#include <cstdint>
#include <vector>

#include "winsock2.h"

#include "socketpp.hpp"


socketpp::Socket server(PF_INET, SOCK_STREAM);
server.bind(server_ip, server_port);
server.listen(1);
auto [client, address] = server.accept();

std::array<char, size> recved_data = client.recv<size>();
int sended_size = client.send(data);

client.close();
```

TCP client

```c++
socketpp::Socket sock(PF_INET, SOCK_STREAM);
sock.bind("0.0.0.0", 50000);
sock.connect(server_ip, server_port);

sock.send(data);
sock.recv<size>();

sock.close();
```

UDP server

```c++
socketpp::Socket server(PF_INET, SOCK_STREAM);
server.bind(server_ip, server_port);

server.recv_from<size>();
server.sendto(data, client_ip, client_port);

server.close();
```

UDP client

```c++
socketpp::Socket sock(PF_INET, SOCK_STREAM);
sock.bind(client_ip, client_port);

sock.sendto(data, server_ip, server_port);
sock.recv_from<size>();

sock.close();
```

## Founder

* [cpyberry](https://github.com/cpyberry)

	email: cpyberry222@gmail.com
