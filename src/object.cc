#include <chrono>
#include <iostream>
#include <sstream>
#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include "object.h"
#include "logging.h"
#include "constants.h"
#include <ctime>  
using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

void run(AMQP::TcpChannel& channel)
{
    AMQP::TcpChannel& _channel = channel;
    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2();
    BOOST_LOG_TRIVIAL(info) << "Starting camera";    
    VideoCapture capture(0);
    if (!capture.isOpened()){
        //error in opening the video input
        BOOST_LOG_TRIVIAL(error) << "Unable to open: " << endl;
        channel.publish("topics", kFailureCamera, kMsg);
        return;
    }
    Mat frame, fgMask;
    auto start_time = std::chrono::system_clock::now();
    auto current_time = std::chrono::system_clock::now();
    while (true) 
    {
        capture >> frame;
        if (frame.empty())
            break;
        //update the background model
        pBackSub->apply(frame, fgMask);
        
        //imshow("FG Mask", fgMask);

        RNG rng(12345);
        findContours(fgMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE,Point(0, 0));

        vector<Rect>boundRect (contours.size());
        vector<vector<Point> > contours_poly( contours.size() );

        for (int i = 0; i < contours.size();i++) 
        {
            if(contourArea(contours[i]) < kMinContours)
            {
                continue;
            }
            BOOST_LOG_TRIVIAL(trace) << "Motion found, checking timeframe";
            current_time = std::chrono::system_clock::now();
            putText(frame, "Motion Detected", Point(10,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
            std::chrono::duration<double> seconds = current_time - start_time;
            //
            std::time_t start = std::chrono::system_clock::to_time_t(start_time);
            BOOST_LOG_TRIVIAL(trace) << "Started checks at " << std::ctime(&start);
            //
            std::time_t current = std::chrono::system_clock::to_time_t(current_time);
            BOOST_LOG_TRIVIAL(trace) << "Next found at " << std::ctime(&current);
            //
            BOOST_LOG_TRIVIAL(trace) << "Difference in time: " << seconds.count() << std::endl;
            if(seconds.count() >= 2)
            {
                BOOST_LOG_TRIVIAL(info) << "Program has been running for " << seconds.count()
                    << " seconds";
                BOOST_LOG_TRIVIAL(warning) << "Motion detected, publishing topic ";
                imwrite("Test.jpg", frame);
                _channel.publish("topics", kMotionResponse, kMsg);
                //channel.publish("topics", kMotionResponse, kMsg);
                //
                approxPolyDP( contours[i], contours_poly[i], 3, true );
                boundRect[i] = boundingRect( contours_poly[i] );
                Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
                rectangle( frame, boundRect[i].tl(), boundRect[i].br(), color, 2 );
                start_time = std::chrono::system_clock::now();
            }
        }

        //imshow("Frame", frame);
    }
}
