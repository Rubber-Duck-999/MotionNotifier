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

#ifndef OBJECT_h
#define OBJECT_h

std::string captureMotion(int seconds, AMQP::TcpChannel& channel);

void run(AMQP::TcpChannel& channel);

#endif