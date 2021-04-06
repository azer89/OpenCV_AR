#ifndef __Camera_Calibrator__
#define __Camera_Calibrator__


class CameraCalibrator
{
public:
	CameraCalibrator();
	~CameraCalibrator();

	void CreateBoard();
    void CalibrateBoard();

private:
    int squaresX; // w
    int squaresY; // h
    //int markerLength; // l
    //int markerSeparation; // s
    int squareLength;
    int markerLength;
	int dictionaryId; // d
    int margins;
    int borderBits;
	
};

#endif