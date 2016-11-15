//This code builds on the last version. This version focuses on HSV and looking a the channels individually
//This version also adds thresholding. This version has improved the filter.
//Variables to control: Choosing HSVplane or other color scheme. Size of image. threshold value. High pass filter.
//Which high pass filter to use rect or circular

//This is for testing version 4. Variables that can be changed: HSVPlane, grayImage, SizeOfImage, ThresholdingValue, 
//filtering the fourier transform.


#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void fftShift(Mat image);
void HighPassRect(Mat image, float PercentageLow1);

int main(int argc, char ** argv)
{
	Mat I, rawr, ImageHSV;
	vector<Mat> HSVplanes;
	Mat OriginalImage = imread("/home/ucruas/hamster.jpg"); //Target1.jpg IMG_0578.JPG InternetBeach1.jpg SalientTestImage1.PNG test1small.jpg runepic3.png
	
																		

	waitKey();

	return 0;
}

void fftShift(Mat image) {
	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = image.cols / 2;
	int cy = image.rows / 2;

	Mat q0(image, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	Mat q1(image, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(image, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(image, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);
}

void HighPassRect(Mat image, float PercentageLow1) {
	float PercentageLow2 = (1 - PercentageLow1) + 1;
	Point2f PointLow1 = Point2f((image.cols / 2)*PercentageLow1, (image.rows / 2)*PercentageLow1);
	Point2f PointLow2 = Point2f((image.cols / 2)*PercentageLow2, (image.rows / 2)*PercentageLow2);
	rectangle(image, PointLow1, PointLow2, Scalar(0, 0, 0), -1, 8, 0);
}
