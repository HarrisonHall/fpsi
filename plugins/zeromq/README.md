# ZeroMQ-Socket
This plugin implements basic communication via sockets and zmq.

## Config
- `is_client` should be true for the client in the socket connection
- `is_server` should be true for the socket in the socket connection
- `ip` determines the target ip address
  - should be `"*"` for the server
- `port` determines the port for the connection

## Requirements
- libsodium
- gnu tls
