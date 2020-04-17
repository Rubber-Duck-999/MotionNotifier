#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include "logging.h"
#include "constants.h"
#include "json.hpp"
#include <ctime>  
#include <algorithm>
#include <iostream>
#include <iterator>
#include <unistd.h>

using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;


class MyHandler : public AMQP::TcpHandler
{
    /**
     *  Method that is called by the AMQP library when a new connection
     *  is associated with the handler. This is the first call to your handler
     *  @param  connection      The connection that is attached to the handler
     */
    void onAttached(AMQP::TcpConnection *connection) override
    {
        // @todo
        //  add your own implementation, for example initialize things
        //  to handle the connection.
    }

    /**
     *  Method that is called by the AMQP library when the TCP connection 
     *  has been established. After this method has been called, the library
     *  still has take care of setting up the optional TLS layer and of
     *  setting up the AMQP connection on top of the TCP layer., This method 
     *  is always paired with a later call to onLost().
     *  @param  connection      The connection that can now be used
     */
    void onConnected(AMQP::TcpConnection *connection) override
    {
        // @todo
        //  add your own implementation (probably not needed)
    }

    /**
     *  Method that is called when the secure TLS connection has been established. 
     *  This is only called for amqps:// connections. It allows you to inspect
     *  whether the connection is secure enough for your liking (you can
     *  for example check the server certicate). The AMQP protocol still has
     *  to be started.
     *  @param  connection      The connection that has been secured
     *  @param  ssl             SSL structure from openssl library
     *  @return bool            True if connection can be used
     */
    bool onSecured(AMQP::TcpConnection *connection, const SSL *ssl) override
    {
        // @todo
        //  add your own implementation, for example by reading out the
        //  certificate and check if it is indeed yours
        return true;
    }

    /**
     *  Method that is called by the AMQP library when the login attempt
     *  succeeded. After this the connection is ready to use.
     *  @param  connection      The connection that can now be used
     */
    void onReady(AMQP::TcpConnection *connection) override
    {
        // @todo
        //  add your own implementation, for example by creating a channel
        //  instance, and start publishing or consuming
    }

    /**
     *  Method that is called by the AMQP library when a fatal error occurs
     *  on the connection, for example because data received from RabbitMQ
     *  could not be recognized, or the underlying connection is lost. This
     *  call is normally followed by a call to onLost() (if the error occured
     *  after the TCP connection was established) and onDetached().
     *  @param  connection      The connection on which the error occured
     *  @param  message         A human readable error message
     */
    void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        // @todo
        //  add your own implementation, for example by reporting the error
        //  to the user of your program and logging the error
    }

    /**
     *  Method that is called when the AMQP protocol is ended. This is the
     *  counter-part of a call to connection.close() to graceful shutdown
     *  the connection. Note that the TCP connection is at this time still 
     *  active, and you will also receive calls to onLost() and onDetached()
     *  @param  connection      The connection over which the AMQP protocol ended
     */
    void onClosed(AMQP::TcpConnection *connection) override 
    {
        // @todo
        //  add your own implementation (probably not necessary, but it could
        //  be useful if you want to do some something immediately after the
        //  amqp connection is over, but do not want to wait for the tcp 
        //  connection to shut down
    }

    /**
     *  Method that is called when the TCP connection was closed or lost.
     *  This method is always called if there was also a call to onConnected()
     *  @param  connection      The connection that was closed and that is now unusable
     */
    void onLost(AMQP::TcpConnection *connection) override 
    {
        // @todo
        //  add your own implementation (probably not necessary)
    }

    /**
     *  Final method that is called. This signals that no further calls to your
     *  handler will be made about the connection.
     *  @param  connection      The connection that can be destructed
     */
    void onDetached(AMQP::TcpConnection *connection) override 
    {
        // @todo
        //  add your own implementation, like cleanup resources or exit the application
    } 

    /**
     *  Method that is called by the AMQP-CPP library when it wants to interact
     *  with the main event loop. The AMQP-CPP library is completely non-blocking,
     *  and only make "write()" or "read()" system calls when it knows in advance
     *  that these calls will not block. To register a filedescriptor in the
     *  event loop, it calls this "monitor()" method with a filedescriptor and
     *  flags telling whether the filedescriptor should be checked for readability
     *  or writability.
     *
     *  @param  connection      The connection that wants to interact with the event loop
     *  @param  fd              The filedescriptor that should be checked
     *  @param  flags           Bitwise or of AMQP::readable and/or AMQP::writable
     */
    void monitor(AMQP::TcpConnection *connection, int fd, int flags) override
    {
        // @todo
        //  add your own implementation, for example by adding the file
        //  descriptor to the main application event loop (like the select() or
        //  poll() loop). When the event loop reports that the descriptor becomes
        //  readable and/or writable, it is up to you to inform the AMQP-CPP
        //  library that the filedescriptor is active by calling the
        //  connection->process(fd, flags) method.
    }
};

int main(int argc, const char* argv[])
{
    init_log();
    BOOST_LOG_TRIVIAL(trace) << "Beginning Camera Monitor Initialisation";

    MyHandler myHandler;

    AMQP::Address addr = AMQP::Address("localhost", kPort, 
        AMQP::Login(kUsername, kPassword), "/");
    AMQP::TcpConnection connection(&myHandler, addr);

    AMQP::TcpChannel channel(&connection);

    int current = 0;
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2();
    BOOST_LOG_TRIVIAL(info) << "Starting camera";    
    VideoCapture capture(kCameraPin);
    if (!capture.isOpened())
    {
        //error in opening the video input
        BOOST_LOG_TRIVIAL(error) << "Unable to open: " << endl;
        channel.publish("topics", kFailureCamera, kFailurePinUnavailable);
        return 0;
    }
    Mat frame, fgMask;
    auto start_time = std::chrono::system_clock::now();
    auto current_time = std::chrono::system_clock::now();
    static int startCount = 0;
    channel.declareExchange("topics", AMQP::topic, true);
    //channel.declareQueue("");
    //channel.bindQueue("topics", "", "my-routing-key");
    while (true) 
    {
        capture >> frame;
        if (frame.empty())
            break;
        //update the background model
        pBackSub->apply(frame, fgMask);
        RNG rng(12345);
        findContours(fgMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE,Point(0, 0));

        vector<Rect>boundRect (contours.size());
        vector<vector<Point>> contours_poly( contours.size() );

        for (int i = 0; i < contours.size();i++) 
        {
            if(contourArea(contours[i]) < kMinContours)
            {
                continue;
            }
            BOOST_LOG_TRIVIAL(trace) << "Motion found, checking timeframe";
            current_time = std::chrono::system_clock::now();
            //putText(frame, "Motion Detected", Point(10,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
            std::chrono::duration<double> seconds = current_time - start_time;
            //
            std::time_t start = std::chrono::system_clock::to_time_t(start_time);
            BOOST_LOG_TRIVIAL(trace) << "Started checks at " << std::ctime(&start);
            //
            std::time_t current = std::chrono::system_clock::to_time_t(current_time);
            BOOST_LOG_TRIVIAL(trace) << "Next found at " << std::ctime(&current);
            //
            BOOST_LOG_TRIVIAL(trace) << "Difference in time: " << seconds.count() << std::endl;
            if(seconds.count() >= 3 && startCount >= 2)
            {
                int count = seconds.count();
                BOOST_LOG_TRIVIAL(trace) << "Program has been running for " << count << " seconds";
                BOOST_LOG_TRIVIAL(warning) << "Motion detected, making message for Motion.Response ";
                std::string filename = getDateTime() + "_" + std::to_string(current) + ".jpg";
                nlohmann::json tempJson;
                tempJson["file"] = filename;
                tempJson["time"] = getTime();
                tempJson["severity"] = count;
                std::string temp = tempJson.dump();
                BOOST_LOG_TRIVIAL(debug) << "Message: " << temp;
                channel.publish("topics", kMotionResponse, temp);
                BOOST_LOG_TRIVIAL(trace) << "File to be saved: " << filename;
                ofstream fileStream;
                try 
                {
                    BOOST_LOG_TRIVIAL(trace) << "File is not there, creating: " + filename;
                    imwrite(filename, frame);
                }
                catch(runtime_error& e)
                {
                    const char* err_msg = e.what();
                    BOOST_LOG_TRIVIAL(error) << "Exception caught: " << err_msg;
                }
                approxPolyDP( contours[i], contours_poly[i], 3, true );
                boundRect[i] = boundingRect( contours_poly[i] );
                Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
                rectangle( frame, boundRect[i].tl(), boundRect[i].br(), color, 2 );
                start_time = std::chrono::system_clock::now();
                BOOST_LOG_TRIVIAL(trace) << "Restarting clock";
            }
        }
        startCount++;
    }
    return 0;
}