

#include "ARUCOWrapper.h"

#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>

#include <iostream>

ARUCOWrapper::ARUCOWrapper()
{
	using namespace std;

	cout << "ARUCOWrapper\n";
}

ARUCOWrapper::~ARUCOWrapper()
{
}

/*
const char* keys  =
        "{@outfile |<none> | Output image }"
        "{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
        "DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
        "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
        "DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{id       |       | Marker id in the dictionary }"
        "{ms       | 200   | Marker size in pixels }"
        "{bb       | 1     | Number of bits in marker borders }"
        "{si       | false | show generated image }";

 */
void ARUCOWrapper::CreateMarker()
{
    using namespace std;
	
	//int dictionaryId = parser.get<int>("d"); //d
	//int markerId = parser.get<int>("id"); //id
	//int borderBits = 1; //bb
	//int markerSize = 200; //ms

    cv::Mat markerImage;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::drawMarker(dictionary, 23, 1000, markerImage, 1);
    cv::imwrite("marker.png", markerImage);

    cout << "Marker created\n";

	
}

// to do: 
// https://docs.opencv.org/3.4.1/d5/dae/tutorial_aruco_detection.html
// // Calibration: https://docs.opencv.org/3.4/da/d13/tutorial_aruco_calibration.html
// readCameraParameters https://docs.opencv.org/3.4/df/d4a/tutorial_charuco_detection.html

void ARUCOWrapper::DetectMarkerFromAnImage()
{
    std::string img_path = "C://Users//azer//workspace//opencv_ar//x64//Release//pic1.jpg";
    cv::Mat inputImage = cv::imread(img_path, cv::IMREAD_COLOR);

    if (inputImage.empty())
    {
        std::cout << "Could not read the image: " << img_path << std::endl;
        return;
    }

	//cv::imshow("Display window", inputImage);
    //int k = cv::waitKey(0);

    //std::vector<int> markerIds;
    //std::vector<std::vector<cv::Point2f>> markerCorners;
    //std::vector<std::vector<cv::Point2f>> rejectedCandidates;
    //cv::Ptr<cv::aruco::DetectorParameters> parameters;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    /*cv::aruco::detectMarkers(inputImage, 
        dictionary, 
        markerCorners, 
        markerIds, 
        parameters, 
        rejectedCandidates);*/
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f> > corners;
    cv::aruco::detectMarkers(inputImage, dictionary, corners, ids);

    // draw
    cv::Mat imageCopy;
    inputImage.copyTo(imageCopy);

    if (ids.size() > 0)
    {
        cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);
    	
        cv::imwrite("detected_marker.png", imageCopy);
    }

    //cv::imshow("Display window", imageCopy);
    //int k = cv::waitKey(0);
	
    std::cout << "DetectMarkerFromAnImage\n";
}