#include <iostream>
#include <sstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include "logging.h"
using namespace cv;
using namespace std;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

int run()
{

    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> pBackSub;
    pBackSub = createBackgroundSubtractorMOG2();
    BOOST_LOG_TRIVIAL(info) << "Starting camera";    
    VideoCapture capture(0);
    if (!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open: " << endl;
        return 0;
    }
    Mat frame, fgMask;
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
            if( contourArea(contours[i])< 500)
            {
                continue;
            }
            putText(frame, "Motion Detected", Point(10,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
            BOOST_LOG_TRIVIAL(info) << "Motion detected = contours: " << contourArea(contours[i]);
            imwrite("Test.jpg", frame);
            approxPolyDP( contours[i], contours_poly[i], 3, true );
            boundRect[i] = boundingRect( contours_poly[i] );
            Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
            rectangle( frame, boundRect[i].tl(), boundRect[i].br(), color, 2 );
        }

        //imshow("Frame", frame);
        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }
    return 0;
}
