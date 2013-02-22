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

#include "cocaine/io.hpp"

#include "cocaine/context.hpp"

using namespace cocaine::io;

socket_t::socket_t(context_t& context, int type):
    m_context(context),
    m_socket(context.io(), type)
{ }

socket_t::socket_t(context_t& context, int type, const std::string& route):
    m_context(context),
    m_socket(context.io(), type)
{
    setsockopt(ZMQ_IDENTITY, route.data(), route.size());
}

void socket_t::bind(const std::string& endpoint) {
    m_socket.bind(endpoint.c_str());

    // Try to determine the connection string for clients.
    size_t position = endpoint.find_last_of(":");

    if(position != std::string::npos) {
        ++position;
        std::string port = endpoint.substr(position, endpoint.length() - position);

        #if ZMQ_VERSION >= 30203
            if ("*" == port) {
                size_t buff_size = 512;
                char buff[512];
                memset(buff, 0, buff_size);
                
                getsockopt(ZMQ_LAST_ENDPOINT, buff, &buff_size);

                std::string last_endpoint(buff);
                size_t pos = last_endpoint.find_last_of(":");

                if (pos != std::string::npos) {
                    ++pos;
                    port = last_endpoint.substr(pos, last_endpoint.length() - pos);
                }
            }
        #endif

        m_endpoint = std::string("tcp://")
                  + m_context.config.runtime.hostname
                  + ":"
                  + port;
    } else {
        m_endpoint = "<local>";
    }
}

void socket_t::connect(const std::string& endpoint) {
    m_socket.connect(endpoint.c_str());
}

void socket_t::drop() {
    zmq::message_t null;

    while(more()) {
        recv(&null);
    }
}
