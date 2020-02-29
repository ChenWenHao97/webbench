// Implementation of the Socket class.

#include "Socket.hpp"
// #include "string.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "ParseHttpResponse.hpp"
Socket::Socket() : m_sock(-1) { memset(&m_addr, 0, sizeof(m_addr)); }

Socket::~Socket()
{
  if (is_valid())
  {
    ::close(m_sock);
  }
  std::cout << "~socket" << std::endl;
}

bool Socket::create()
{
  m_sock = socket(AF_INET, SOCK_STREAM, 0);

  if (!is_valid())
    return false;

  // TIME_WAIT - argh
  int on = 1;
  if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on,
                 sizeof(on)) == -1)
    return false;

  return true;
}

bool Socket::bind(const int port)
{

  if (!is_valid())
  {
    return false;
  }
  std::cout << "valid" << std::endl;
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons(port);

  int bind_return = ::bind(m_sock, (struct sockaddr *)&m_addr, sizeof(m_addr));

  if (bind_return == -1)
  {
    return false;
  }

  return true;
}

bool Socket::listen() const
{
  if (!is_valid())
  {
    return false;
  }

  int listen_return = ::listen(m_sock, MAXCONNECTIONS);

  if (listen_return == -1)
  {
    return false;
  }

  return true;
}

bool Socket::accept(Socket &new_socket) const
{
  int addr_length = sizeof(m_addr);
  new_socket.m_sock =
      ::accept(m_sock, (sockaddr *)&m_addr, (socklen_t *)&addr_length);

  if (new_socket.m_sock <= 0)
    return false;
  else
    return true;
}

bool Socket::send(const std::string s) const
{
  int status = ::send(m_sock, s.c_str(), s.size(), MSG_NOSIGNAL);
  if (status == -1)
  {
    printf("send reports an error: %s\n", strerror(errno));
    return false;
  }
  else
  {
    return true;
  }
}

int Socket::recv(std::string &s) const
{
  char buf[MAXRECV + 1];

  s = "";
  int status;
  memset(buf, 0, MAXRECV + 1);
  status = ::recv(m_sock, buf, MAXRECV, 0);
  // cout<<"buf:"<<buf<<endl;
  string str = buf;
  ParseHttpResponse response;
  bool hashead = response.IsHeadEnd(str);
  if (hashead)
  {
    // cout<<"hashead!!!"<<endl;
    int hascontentlength = response.GetContentLength(str);
    if (hascontentlength == -1)
      status = -1;
    else if (hascontentlength - MAXRECV > 0)
    {
      hascontentlength -= MAXRECV;
      s += buf;
      while (hascontentlength > 0)
      {
        memset(buf, 0, MAXRECV + 1);
        int recvdata = ::recv(m_sock, buf, MAXRECV, 0);
        if (recvdata == -1)
        {
          status = -1;
          break;
        }
        s += buf;
        hascontentlength -= recvdata;
      }
    }
    // cout<<"!!!contentLength:"<<hascontentlength<<endl;
  }
  cout << "socketRECV!!!" << endl;
  if (status == -1)
  {
    std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
    return 0;
  }
  else if (status == 0)
  {
    return 0;
  }
  else
  {
    return status;
  }
}

bool Socket::connect(const std::string host, const int port)
{
  if (!is_valid())
    return false;
  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons(port);

  int status = inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr);

  if (errno == EAFNOSUPPORT)
    return false;

  status = ::connect(m_sock, (sockaddr *)&m_addr, sizeof(m_addr));
  // std::cout <<status<<std::endl;
  if (status == 0)
    return true;
  else
    return false;
}

void Socket::set_non_blocking(const bool b)
{

  int opts = fcntl(m_sock, F_GETFL);

  if (opts < 0)
  {
    return;
  }

  if (b)
    opts = (opts | O_NONBLOCK);
  else
    opts = (opts & ~O_NONBLOCK);

  fcntl(m_sock, F_SETFL, opts);
}