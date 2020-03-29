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

template<class InIter> inline std::string
join(InIter begin, InIter end, std::string delim)
{
    std::stringstream ss;
    std::copy(begin, end,
            std::ostream_iterator<const char*>(ss, delim.c_str()));
    return ss.str();
}

int main(int argc, const char* argv[])
{
    init_log();
    BOOST_LOG_TRIVIAL(trace) << "this is a trace message";
    BOOST_LOG_TRIVIAL(debug) << "this is a debug message";
    BOOST_LOG_TRIVIAL(info) << "this is a info message";
    BOOST_LOG_TRIVIAL(warning) << "this is a warning message";
    BOOST_LOG_TRIVIAL(error) << "this is an error message";
    BOOST_LOG_TRIVIAL(fatal) << "this is a fatal error message";

    const std::string routing_key = "motion.response";
    const std::string msg = "Motion Detected on CM";

    auto evbase = event_base_new();
    LibEventHandlerMyError handler(evbase);
    AMQP::TcpConnection connection(&handler,
            AMQP::Address("localhost", 5672,
                          AMQP::Login("guest", "guest"), "/"));

    AMQP::TcpChannel channel(&connection);

    channel.onError([&evbase](const char* message)
        {
            std::cout << "Channel error: " << message << std::endl;
            event_base_loopbreak(evbase);
        });

    unsigned int microseconds = 2000;

    usleep(microseconds);

    channel.declareExchange("topics", AMQP::topic, true)
        .onError([&](const char* msg)
        {
            std::cout << "ERROR: " << msg << std::endl;
        })
        .onSuccess
        (
            [&]()
            {
                channel.publish("topics", kFailureComponent, msg);
                std::cout << "Sent " << std::endl;
                channel.publish("topics", kMotionResponse, msg);
                channel.publish("topics", kFailureCamera, msg);
                usleep(microseconds);
                //event_base_loopbreak(evbase);
            }
        );
    event_base_dispatch(evbase);
    event_base_free(evbase);
    return 0;
}