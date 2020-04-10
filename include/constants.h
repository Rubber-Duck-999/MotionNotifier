/*
 * constants.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Rubber-Duck-999
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include "json.hpp"
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>

/* Topics */
const std::string kMotionResponse   = "Motion.Response";
const std::string kFailureCamera    = "Failure.Camera";
const std::string kFailureComponent = "Failure.Component";

/* Time */
/* Time */
static std::string getDateTime()
{
    time_t now = time(0);

    tm *ltm = localtime(&now);

    std::stringstream date;
    date<< ltm->tm_mday
         << "/"
         << 1 + ltm->tm_mon
         << "/"
         << 1900 + ltm->tm_year
         << " "
         << 1 + ltm->tm_hour
         << ":"
         << 1 + ltm->tm_min
         << ":"
         << 1 + ltm->tm_sec;
    return date.str();
}

static std::string getTime()
{
    time_t now = time(0);

    tm *ltm = localtime(&now);

    std::stringstream date;
    date << " "
         << 1 + ltm->tm_hour
         << ":"
         << 1 + ltm->tm_min
         << ":"
         << 1 + ltm->tm_sec;
    return date.str();
}

/* Motion message json */
const int kMotionHigh = 4;
const int kMotionMedHigh = 3; 
const int kMotionMed = 2; 
const int kMotionLow = 1; 
const int kMotionNone = 0; 

/* Failure message json */
const nlohmann::json kFailurePinUnavailable = {{"time", getTime()},
                                                {"severity", 3}};


const int kMaxSize = 100;
const int kPort = 5672;
const std::string kUsername = "guest";
const std::string kPassword = "password";


/* Camera */
const int kMinContours = 1500;
const int kCameraPin = 0;
const unsigned int kMicroseconds = 2000;

#endif /* CONSTANTS_H_ */