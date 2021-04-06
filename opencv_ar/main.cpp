
#include <iostream>

#include "ARUCOWrapper.h"
#include "CameraCalibrator.h"

int main()
{
	using namespace std;

	//CameraCalibrator cam_calib;
	//cam_calib.CalibrateBoard();

	ARUCOWrapper ar_wrapper;
	ar_wrapper.DetectArucoMarkerFromVideo();
	//ar_wrapper.CreateMarker();
	//ar_wrapper.DetectMarkerFromAnImage();

	
	
	cin.get();
	
	
	return 0;
}