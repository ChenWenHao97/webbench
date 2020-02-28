// Implementation of the ClientSocket class

#include "ClientSocket.hpp"
#include "SocketException.hpp"

ClientSocket::ClientSocket(std::string host, int port)
{
  if (!Socket::create())
  {
    throw SocketException("Could not create client socket.");
  }

  if (!Socket::connect(host, port))
  {
    throw SocketException("Could not bind to port.");
  }
}
void ClientSocket::Set_Non_Blocking(bool is)
{
  Socket::set_non_blocking(is);
}

const ClientSocket &ClientSocket::operator<<(const std::string &s) const
{
  if (!Socket::send(s))
  {
    throw SocketException("Could not write to socket.");
  }

  return *this;
}

const ClientSocket &ClientSocket::operator>>(std::string &s) const
{
  if (!Socket::recv(s))
  {
    throw SocketException("Could not read from socket.");
  }

  return *this;
}