/*
 * constants.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Rubber-Duck-999
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <string>

const std::string kMotionResponse   = "Motion.Response";
const std::string kFailureCamera    = "Failure.Camera";
const std::string kFailureComponent = "Failure.Component";
const std::string kMsg = "Blank";

const int kMaxSize = 100;
const int kPort = 5672;
const std::string kUsername = "guest";
const std::string kPassword = "guest";
const int kMinContours = 1500;
const unsigned int kMicroseconds = 2000;

#endif /* CONSTANTS_H_ */