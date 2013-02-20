/*
    Copyright (c) 2011-2012 Andrey Sibiryov <me@kobology.ru>
    Copyright (c) 2011-2012 Other contributors as noted in the AUTHORS file.

    This file is part of Cocaine.

    Cocaine is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Cocaine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cocaine/asio/acceptor.hpp"
#include "cocaine/asio/pipe.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace cocaine::io;

acceptor_t::~acceptor_t() {
    if(m_fd == -1) {
        return;
    }

    if(::close(m_fd) != 0) {
        // Log.
    }
}

acceptor_t::acceptor_t(acceptor_t&& other):
    m_fd(-1)
{
    *this = std::move(other);
}

acceptor_t&
acceptor_t::operator=(acceptor_t&& other) {
    std::swap(m_fd, other.m_fd);
    return *this;
}

std::shared_ptr<pipe_t>
acceptor_t::accept() {
#ifdef _GNU_SOURCE
    struct sockaddr_in address = { AF_INET, 0, { 0 }, { 0 } };
#else
    struct sockaddr_in address = { sizeof(sockaddr_in), AF_INET, 0, { 0 }, { 0 } };
#endif

    socklen_t length = sizeof(address);

    int fd = ::accept(m_fd, reinterpret_cast<sockaddr*>(&address), &length);

    if(fd == -1) {
        switch(errno) {
            case EAGAIN:
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK:
#endif
            case EINTR:
                return std::shared_ptr<pipe_t>();

            default:
                throw io_error_t("unable to accept a connection");
        }
    }

    ::fcntl(m_fd, F_SETFD, FD_CLOEXEC);
    ::fcntl(m_fd, F_SETFL, O_NONBLOCK);

    int enable = 1;

    // Disable Nagle's algorithm.
    ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

    return std::make_shared<pipe_t>(fd);
}

pipe_link_t
cocaine::io::link() {
    int fds[2] = { -1, -1 };

    if(::socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) != 0) {
        throw io_error_t("unable to create a link");
    }

    return std::make_pair(
        std::make_shared<pipe_t>(fds[0]),
        std::make_shared<pipe_t>(fds[1])
    );
}