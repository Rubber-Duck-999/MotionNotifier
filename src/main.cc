#include <amqpcpp.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include "constants.h"
#include "conn_handler.h"
#include <iostream>
#include "logging.h"
#include <unistd.h>
#include "object.h"

/*
template<class InIter> inline std::string
join(InIter begin, InIter end, std::string delim)
{
    std::stringstream ss;
    std::copy(begin, end,
            std::ostream_iterator<const char*>(ss, delim.c_str()));
    return ss.str();
}*/

int main(int argc, const char* argv[])
{
    init_log();
    BOOST_LOG_TRIVIAL(trace) << "Beginning Camera Monitor Initialisation";

    auto evbase = event_base_new();
    LibEventHandlerMyError handler(evbase);
    AMQP::Address addr = AMQP::Address("localhost", kPort, 
        AMQP::Login(kUsername, kPassword), "/");
    AMQP::TcpConnection connection(&handler, addr);

    AMQP::TcpChannel channel(&connection);

    channel.onError([&evbase](const char* msg)
        {
            BOOST_LOG_TRIVIAL(error) << "Channel error: " << msg;
            event_base_loopbreak(evbase);
    });

    

    usleep(kMicroseconds);

    channel.declareExchange("topics", AMQP::topic, true).onError([&](const char* msg) 
    {
        BOOST_LOG_TRIVIAL(error) << "Error on exchange declaration: " << msg;
    }).onSuccess 
    (
        [&]()
        {
            BOOST_LOG_TRIVIAL(info) << "Success in channel";
            usleep(kMicroseconds);
            event_base_loopbreak(evbase);
        }
    );
    event_base_dispatch(evbase);
    event_base_free(evbase);
    run(channel);
    return 0;
}