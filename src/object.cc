#include "object.h"

using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

std::string captureMotion(int seconds, AMQP::TcpChannel& channel)
{
    static int current = 0;
    BOOST_LOG_TRIVIAL(trace) << "Program has been running for " << seconds << " seconds";
    BOOST_LOG_TRIVIAL(warning) << "Motion detected, making message for Motion.Response ";
    std::string filename = getDateTime() + "_" + std::to_string(current) + ".jpg";
    nlohmann::json tempJson;
    tempJson["file"] = filename;
    tempJson["time"] = getTime();
    current++;
    BOOST_LOG_TRIVIAL(debug) << "File to be saved: " << filename;
    switch(seconds)
    {
        case 3:
        {
            if(tempJson != NULL)
            {
                try
                {
                    tempJson["severity"] = kMotionMed;
                    std::string temp = tempJson.dump();
                    channel.publish("topics", kMotionResponse, temp);
                }
                catch (nlohmann::json::type_error& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
                }
                catch (...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Exception";
                }

            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Failure on json type";
            }
            return filename;
        }
        case 4:
        {
            if(tempJson != NULL)
            {
                try
                {
                    tempJson["severity"] = kMotionMedHigh;
                    std::string temp = tempJson.dump();
                    channel.publish("topics", kMotionResponse, temp);
                }
                catch (nlohmann::json::type_error& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
                }
                catch (...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Exception";
                }
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Failure on json type";
            }
            return filename;
        }
        case 5:
        {
            if(tempJson != NULL)
            {
                try
                {
                    tempJson["severity"] = kMotionHigh;
                    std::string temp = tempJson.dump();
                    channel.publish("topics", kMotionResponse, temp);
                }
                catch (nlohmann::json::type_error& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Exception: " << e.what();
                }
                catch (...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Exception";
                }
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Failure on json type";
            }
            return filename;
        }
        default:
        {
            BOOST_LOG_TRIVIAL(debug) << "Motion longer than 5 seconds";
            return filename;
        }
    }
}


void run(AMQP::TcpChannel& channel)
{
    AMQP::TcpChannel& _channel = channel;
    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2();
    BOOST_LOG_TRIVIAL(info) << "Starting camera";    
    VideoCapture capture(kCameraPin);
    if (!capture.isOpened())
    {
        //error in opening the video input
        BOOST_LOG_TRIVIAL(error) << "Unable to open: " << endl;
        _channel.publish("topics", kFailureCamera, kFailurePinUnavailable);
        return;
    }
    Mat frame, fgMask;
    auto start_time = std::chrono::system_clock::now();
    auto current_time = std::chrono::system_clock::now();
    static int startCount = 0;
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
                int temp = seconds.count();
                std::string filename = captureMotion(seconds.count(), _channel);
                BOOST_LOG_TRIVIAL(trace) << "File " << filename;
                ofstream fileStream;
                try 
                {
                    BOOST_LOG_TRIVIAL(debug) << "File is not there, creating: " + filename;
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
}
