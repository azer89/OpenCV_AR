
#include <iostream>

#include "ARUCOWrapper.h"
#include "CameraCalibrator.h"

int main()
{
	using namespace std;

	//cout << "hello world";
	//ARUCOWrapper ar_wrapper;
	//ar_wrapper.CreateMarker();
	//ar_wrapper.DetectMarkerFromAnImage();

	CameraCalibrator cam_calib;
	cam_calib.CalibrateBoard();
	
	cin.get();
	
	
	return 0;
}