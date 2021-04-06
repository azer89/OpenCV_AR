
#include <iostream>

#include "ARUCOWrapper.h"

int main()
{
	using namespace std;

	//cout << "hello world";
	ARUCOWrapper ar_wrapper;
	//ar_wrapper.CreateMarker();
	ar_wrapper.DetectMarkerFromAnImage();
	
	cin.get();
	
	
	return 0;
}