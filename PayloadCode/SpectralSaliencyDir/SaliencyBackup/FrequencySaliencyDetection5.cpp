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
	Mat OriginalImage = imread("/home/ucruas/targets/IMG_0578.JPG"); //Target1.jpg IMG_0578.JPG InternetBeach1.jpg SalientTestImage1.PNG test1small.jpg runepic3.png
	
	
	int sizereduced = 5;
	resize(OriginalImage, OriginalImage, Size(OriginalImage.cols / sizereduced, OriginalImage.rows / sizereduced));//resize image


	
	namedWindow("Original Image", WINDOW_AUTOSIZE);//WINDOW_NORMAL WINDOW_AUTOSIZE
	imshow("Original Image", OriginalImage);

	cvtColor(OriginalImage, ImageHSV, COLOR_BGR2HSV);
	split(ImageHSV, HSVplanes);
	OriginalImage = HSVplanes[2];
	//Mat GreyImage;
	//cvtColor(OriginalImage, GreyImage, CV_RGB2GRAY);
	//OriginalImage = GreyImage;

	namedWindow("HSVplane", WINDOW_AUTOSIZE);
	imshow("HSVplane", OriginalImage);

	//int sizereduced = 10;
	//resize(OriginalImage, OriginalImage, Size(OriginalImage.cols / sizereduced, OriginalImage.rows / sizereduced));//resize image
																												   //resize(OriginalImage, rawr, Size(100,100));//resize image
	I = OriginalImage;																										   //cvtColor(OriginalImage, I, CV_BGR2GRAY);
																															   //cvtColor(OriginalImage, I, CV_BGR2GRAY);

	Mat padded;                            //expand input image to optimal size
	int m = getOptimalDFTSize(I.rows);
	int n = getOptimalDFTSize(I.cols); // on the border add zero values
	copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

	dft(complexI, complexI);            // this way the result may fit in the source matrix

	split(complexI, planes);
	Mat magI, phaseI;
	cartToPolar(planes[0], planes[1], magI, phaseI);

	magI += Scalar::all(1);// switch to logarithmic scale
	log(magI, magI);

	fftShift(magI);
	normalize(magI, magI, 0, 1, NORM_MINMAX); // Transform the matrix with float values into a
	//HighPassRect(magI, .9);
	//namedWindow("magI", WINDOW_AUTOSIZE);
	//imshow("magI", magI);
	fftShift(magI);

	Mat magAverage, magRes, Sum, InverseFourier, kernel;

	medianBlur(magI, magAverage, 5);
	//int kernel_size = 5;
	//kernel = Mat::ones(kernel_size, kernel_size, CV_32F) / (float)(kernel_size*kernel_size);
	//filter2D(magI, L, -1, kernel, Point(-1, -1), 0, BORDER_DEFAULT);

	magRes = magI - magAverage;
	exp(magRes, magRes);

	polarToCart(magRes, phaseI, planes[0], planes[1]);

	merge(planes, 2, Sum);

	//dft(complexI, InverseFourier, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
	dft(Sum, InverseFourier, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
	pow(InverseFourier, 2, InverseFourier);
	GaussianBlur(InverseFourier, InverseFourier, Size(5, 5), 0);
	normalize(InverseFourier, InverseFourier, 0, 1, NORM_MINMAX);
	//imshow("Res", Res);
	namedWindow("inverse fourier transform", WINDOW_AUTOSIZE);
	imshow("inverse fourier transform", InverseFourier);

	Mat Threshold;
	inRange(InverseFourier, .4, 1, Threshold); //Threshold the image
	namedWindow("Threshold", WINDOW_AUTOSIZE);
	imshow("Threshold", Threshold);

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
