

#include "ARUCOWrapper.h"

#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>

ARUCOWrapper::ARUCOWrapper()
{
	using namespace std;

	cout << "ARUCOWrapper\n";
}

ARUCOWrapper::~ARUCOWrapper()
{
}

bool ReadCameraParameters(std::string filename, cv::Mat& camMatrix, cv::Mat& distCoeffs)
{
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    if (!fs.isOpened())
        return false;
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    return true;
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
void ARUCOWrapper::CreateArucoMarker()
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


void ARUCOWrapper::DetectCharucoBoardFromVideo()
{
    int squaresX = 5;
    int squaresY = 7;
    int squareLength = 200;
    int markerLength = 120;
    int dictionaryId = 10;
    int margins = squareLength - markerLength;
    int borderBits = 1;

    cv::Mat camMatrix;
    cv::Mat distCoeffs;
    std::string calibFile = "calib_camera";
    bool readOk = ReadCameraParameters(calibFile, camMatrix, distCoeffs);
    if (!readOk) 
    {
        std::cerr << "Invalid camera file" << std::endl;
        return;
    }

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    cv::VideoCapture inputVideo;
    int camId = 0;
    inputVideo.open(camId, cv::CAP_DSHOW);
    int waitTime = 10;

    float axisLength = 0.5f * ((float)std::min(squaresX, squaresY) * (squareLength));

    // create charuco board object
    cv::Ptr<cv::aruco::CharucoBoard> charucoboard =
        cv::aruco::CharucoBoard::create(squaresX, squaresY, squareLength, markerLength, dictionary);
    cv::Ptr<cv::aruco::Board> board = charucoboard.staticCast<cv::aruco::Board>();

    cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();

    double totalTime = 0;
    int totalIterations = 0;
	
    while (inputVideo.grab())
    {
        cv::Mat image;
        cv::Mat imageCopy;
        inputVideo.retrieve(image);
    	
        double tick = (double)cv::getTickCount();

        std::vector< int > markerIds, charucoIds;
        std::vector< std::vector< cv::Point2f > > markerCorners, rejectedMarkers;
        std::vector< cv::Point2f > charucoCorners;
        cv::Vec3d rvec, tvec;

        // detect markers
        cv::aruco::detectMarkers(image, dictionary, markerCorners, markerIds, detectorParams,
            rejectedMarkers);

        // interpolate charuco corners
        int interpolatedCorners = 0;
        if (markerIds.size() > 0)
            interpolatedCorners =
            cv::aruco::interpolateCornersCharuco(markerCorners, markerIds, image, charucoboard,
                charucoCorners, charucoIds, camMatrix, distCoeffs);

        // estimate charuco board pose
        bool validPose = false;
        if (camMatrix.total() != 0)
        {
            validPose = cv::aruco::estimatePoseCharucoBoard(charucoCorners, charucoIds, charucoboard,
                camMatrix, distCoeffs, rvec, tvec);
        }


        double currentTime = ((double)cv::getTickCount() - tick) / cv::getTickFrequency();
        totalTime += currentTime;
        totalIterations++;
        if (totalIterations % 30 == 0) 
        {
            std::cout << "Detection Time = " << currentTime * 1000 << " ms "
                << "(Mean = " << 1000 * totalTime / double(totalIterations) << " ms)" << std::endl;
        }

        // draw results
        image.copyTo(imageCopy);
        if (markerIds.size() > 0) {
            cv::aruco::drawDetectedMarkers(imageCopy, markerCorners);
        }

        if (rejectedMarkers.size() > 0)
            cv::aruco::drawDetectedMarkers(imageCopy, rejectedMarkers, cv::noArray(), cv::Scalar(100, 0, 255));

        if (interpolatedCorners > 0) 
        {
            cv::Scalar color;
            color = cv::Scalar(255, 0, 0);
            cv::aruco::drawDetectedCornersCharuco(imageCopy, charucoCorners, charucoIds, color);
        }

        if (validPose)
        {
            cv::aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rvec, tvec, axisLength);
        }
    	
        imshow("out", imageCopy);
        char key = (char)cv::waitKey(waitTime);
        if (key == 27) break;
    }

    // close video
    inputVideo.release();
    cv::destroyAllWindows();
}

// to do: 
// https://docs.opencv.org/3.4.1/d5/dae/tutorial_aruco_detection.html
// // Calibration: https://docs.opencv.org/3.4/da/d13/tutorial_aruco_calibration.html
// readCameraParameters https://docs.opencv.org/3.4/df/d4a/tutorial_charuco_detection.html

void ARUCOWrapper::DetectArucoMarkerFromAnImage()
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