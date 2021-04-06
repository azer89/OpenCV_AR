
#include "CameraCalibrator.h"

#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>



// https://docs.opencv.org/3.4/df/d4a/tutorial_charuco_detection.html

/*
"{@outfile |<none> | Output image }"
"{w        |       | Number of squares in X direction }"
"{h        |       | Number of squares in Y direction }"
"{sl       |       | Square side length (in pixels) }"
"{ml       |       | Marker side length (in pixels) }"
"{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
"DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
"DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
"DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
"{m        |       | Margins size (in pixels). Default is (squareLength-markerLength) }"
"{bb       | 1     | Number of bits in marker borders }"
"{si       | false | show generated image }";
 */
CameraCalibrator::CameraCalibrator()
{
    // -w=5 -h=7 -sl=200 -ml=120 -d=10
    this->squaresX = 5;           // w
    this->squaresY = 7;          // h
    this->squareLength = 200;
    this->markerLength = 120;    // l
    //this->markerSeparation = 10; // s
    this->dictionaryId = 10;     // d
    this->margins = this->squareLength - this->markerLength;
    this->borderBits = 1;
}
CameraCalibrator::~CameraCalibrator()
{	
}


void CameraCalibrator::CreateBoard()
{
    using namespace cv;

    Size imageSize;
    imageSize.width = squaresX * squareLength + 2 * margins;
    imageSize.height = squaresY * squareLength + 2 * margins;


    //Ptr<aruco::Dictionary> dictionary =
    //    aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
    Ptr<aruco::Dictionary> dictionary =
        aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
	
    /*Ptr<aruco::GridBoard> board = 
        aruco::GridBoard::create(markersX, 
								 markersY, 
                                 float(markerLength),
			                     float(markerSeparation), 
                                 dictionary);*/
    Ptr<aruco::CharucoBoard> board = 
        aruco::CharucoBoard::create(squaresX, 
									squaresY, 
									(float)squareLength,
									(float)markerLength, 
									dictionary);

    // show created board
    Mat boardImage;
    board->draw(imageSize, boardImage, margins, borderBits);


    cv::imwrite("board.png", boardImage);
    //imshow("board", boardImage);
    //waitKey(0);

}


void CameraCalibrator::CalibrateBoard()
{
    cv::VideoCapture inputVideo;
    int camId = 0;
    inputVideo.open(camId, cv::CAP_DSHOW);
    int waitTime = 10;

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    // create charuco board object
    cv::Ptr<cv::aruco::CharucoBoard> charucoboard =
        cv::aruco::CharucoBoard::create(squaresX, 
										squaresY, 
										squareLength, 
										markerLength, 
										dictionary);
    cv::Ptr<cv::aruco::Board> board = charucoboard.staticCast<cv::aruco::Board>();

    cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();

    while (inputVideo.grab())
    {
        cv::Mat image;
        cv::Mat imageCopy;
        inputVideo.retrieve(image);

        std::vector< int > ids;
        std::vector< std::vector< cv::Point2f > > corners, rejected;

        cv::aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);

        cv::Mat currentCharucoCorners;
    	cv::Mat currentCharucoIds;
        if (ids.size() > 0)
            cv::aruco::interpolateCornersCharuco(corners, 
                ids, 
                image, 
                charucoboard, 
                currentCharucoCorners,
                currentCharucoIds);
    	
        // draw results
        image.copyTo(imageCopy);

        if (ids.size() > 0) cv::aruco::drawDetectedMarkers(imageCopy, corners);

        if (currentCharucoCorners.total() > 0)
            cv::aruco::drawDetectedCornersCharuco(imageCopy, currentCharucoCorners, currentCharucoIds);

        putText(imageCopy, "Press 'c' to add current frame. 'ESC' to finish and calibrate",
            cv::Point(10, 20), 
            cv::FONT_HERSHEY_SIMPLEX, 
            0.5, 
            cv::Scalar(255, 0, 0), 
            2);

        imshow("Calibration", imageCopy);
        char key = (char)cv::waitKey(waitTime);
        if (key == 27) // ESC
        {
            break;
        }
        else if(key == 'c' || key == 'C')
        {
            std::cout << "c\n";
        }
        else if (key == 's' || key == 'S')
        {
            cv::imwrite("calibration_board.png", imageCopy);
        }
    }

    inputVideo.release();
    cv::destroyAllWindows();
}
