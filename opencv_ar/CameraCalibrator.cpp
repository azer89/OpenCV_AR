
#include "CameraCalibrator.h"

#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <iostream>
#include <ctime>

/**
 */
bool SaveCameraParams(const std::string& filename, 
    cv::Size imageSize, 
    float aspectRatio, 
    int flags,
    const cv::Mat& cameraMatrix, 
    const cv::Mat& distCoeffs, 
    double totalAvgErr)
{
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);
    if (!fs.isOpened())
        return false;

    time_t tt;
    time(&tt);
    struct tm* t2 = localtime(&tt);
    char buf[1024];
    strftime(buf, sizeof(buf) - 1, "%c", t2);

    fs << "calibration_time" << buf;

    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;

    if (flags & cv::CALIB_FIX_ASPECT_RATIO) fs << "aspectRatio" << aspectRatio;

    if (flags != 0) {
        sprintf(buf, "flags: %s%s%s%s",
            flags & cv::CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
            flags & cv::CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
            flags & cv::CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
            flags & cv::CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "");
    }

    fs << "flags" << flags;

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;

    fs << "avg_reprojection_error" << totalAvgErr;

    return true;
}



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

	// for capture
    std::vector< std::vector< std::vector< cv::Point2f > > > allCorners;
    std::vector< std::vector< int > > allIds;
    std::vector< cv::Mat > allImgs;
    cv::Size imgSize;


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
            std::cout << "Frame captured" << std::endl;
            allCorners.push_back(corners);
            allIds.push_back(ids);
            allImgs.push_back(image);
            imgSize = image.size();
        }
        else if (key == 's' || key == 'S')
        {
            cv::imwrite("calibration_board.png", imageCopy);
        }
    }

	// close video
    inputVideo.release();
    cv::destroyAllWindows();

    if (allIds.size() < 1) {
        std::cerr << "Not enough captures for calibration" << std::endl;
        return;
    }

    cv::Mat cameraMatrix;
	cv::Mat distCoeffs;
    std::vector< cv::Mat > rvecs;
    std::vector< cv::Mat > tvecs;
    double repError;

    int calibrationFlags = 0;
    float aspectRatio = 1;
    //if (calibrationFlags & CALIB_FIX_ASPECT_RATIO) {
    //cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    //cameraMatrix.at< double >(0, 0) = aspectRatio;
    //}

    // prepare data for calibration
    std::vector< std::vector< cv::Point2f > > allCornersConcatenated;
    std::vector< int > allIdsConcatenated;
    std::vector< int > markerCounterPerFrame;
    markerCounterPerFrame.reserve(allCorners.size());
    for (unsigned int i = 0; i < allCorners.size(); i++) 
    {
        markerCounterPerFrame.push_back((int)allCorners[i].size());
        for (unsigned int j = 0; j < allCorners[i].size(); j++) 
        {
            allCornersConcatenated.push_back(allCorners[i][j]);
            allIdsConcatenated.push_back(allIds[i][j]);
        }
    }

    // calibrate camera using aruco markers
    double arucoRepErr;
    arucoRepErr = cv::aruco::calibrateCameraAruco(allCornersConcatenated, allIdsConcatenated,
        markerCounterPerFrame, 
        board, 
        imgSize, 
        cameraMatrix,
        distCoeffs, 
        cv::noArray(), 
        cv::noArray(), 
        calibrationFlags);

    // prepare data for charuco calibration
    int nFrames = (int)allCorners.size();
    std::vector< cv::Mat > allCharucoCorners;
    std::vector< cv::Mat > allCharucoIds;
    std::vector< cv::Mat > filteredImages;
    allCharucoCorners.reserve(nFrames);
    allCharucoIds.reserve(nFrames);

    for (int i = 0; i < nFrames; i++) {
        // interpolate using camera parameters
        cv::Mat currentCharucoCorners, currentCharucoIds;
        cv::aruco::interpolateCornersCharuco(allCorners[i], allIds[i], allImgs[i], charucoboard,
            currentCharucoCorners, currentCharucoIds, cameraMatrix,
            distCoeffs);

        allCharucoCorners.push_back(currentCharucoCorners);
        allCharucoIds.push_back(currentCharucoIds);
        filteredImages.push_back(allImgs[i]);
    }

    if (allCharucoCorners.size() < 4) 
    {
        std::cerr << "Not enough corners for calibration" << std::endl;
        return;
    }

    // calibrate camera using charuco
    repError =
        cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, charucoboard, imgSize,
            cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);

    std::string outputFile = "calib_camera";
    bool saveOk = SaveCameraParams(outputFile,
        imgSize, 
        aspectRatio, 
        calibrationFlags,
        cameraMatrix, 
        distCoeffs, 
        repError);
    if (!saveOk) {
        std::cerr << "Cannot save output file" << std::endl;
        return;
    }

    std::cout << "Rep Error: " << repError << std::endl;
    std::cout << "Rep Error Aruco: " << arucoRepErr << std::endl;
    std::cout << "Calibration saved to " << outputFile << std::endl;
}

