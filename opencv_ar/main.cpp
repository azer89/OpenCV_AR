
#include <iostream>

#include "ARUCOWrapper.h"
#include "CameraCalibrator.h"

int main()
{
	using namespace std;

	//CameraCalibrator cam_calib;
	//cam_calib.CalibrateBoard();
	
	//cout << "hello world";
	ARUCOWrapper ar_wrapper;
	ar_wrapper.DetectCharucoBoardFromVideo();
	//ar_wrapper.CreateMarker();
	//ar_wrapper.DetectMarkerFromAnImage();

	
	
	cin.get();
	
	
	return 0;
}