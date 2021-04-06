#ifndef __ARUCO_Wrapper__
#define __ARUCO_Wrapper__

class ARUCOWrapper
{
public:
	ARUCOWrapper();
	~ARUCOWrapper();

	void CreateMarker();

	void DetectMarkerFromAnImage();
};

#endif