/*
    Copyright (c) 2011-2013 Andrey Sibiryov <me@kobology.ru>
    Copyright (c) 2011-2013 Other contributors as noted in the AUTHORS file.

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

#ifndef COCAINE_ACTOR_HPP
#define COCAINE_ACTOR_HPP

#include "cocaine/common.hpp"

#include "cocaine/asio/reactor.hpp"
#include "cocaine/asio/tcp.hpp"

#include <list>

#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>

namespace cocaine {

class dispatch_t;

class actor_t {
    COCAINE_DECLARE_NONCOPYABLE(actor_t)

    public:
        actor_t(context_t& context,
                std::shared_ptr<io::reactor_t> reactor,
                std::unique_ptr<dispatch_t>&& dispatch);

       ~actor_t();

        void
        run(std::vector<io::tcp::endpoint> endpoints);

        void
        terminate();

    public:
        std::vector<io::tcp::endpoint>
        endpoints() const;

        dispatch_t&
        dispatch();

    private:
        void
        on_connection(const std::shared_ptr<io::socket<io::tcp>>& socket);

        void
        on_message(int fd, const io::message_t& message);

        void
        on_failure(int fd, const std::error_code& ec);

    private:
        const std::unique_ptr<logging::log_t> m_log;
        const std::shared_ptr<io::reactor_t> m_reactor;

        // Actor I/O channels

        struct lockable_type;
        struct upstream_t;

        std::map<
            int,
            std::shared_ptr<lockable_type>
        > m_channels;

        std::unique_ptr<dispatch_t> m_dispatch;

        // Actor I/O connectors

        std::list<
            io::connector<io::acceptor<io::tcp>>
        > m_connectors;

        // Execution context

        std::unique_ptr<boost::thread> m_thread;
};

} // namespace cocaine

#endif
