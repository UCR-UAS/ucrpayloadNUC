//This code builds on the last version. This version focuses on HSV and looking a the channels individually
//This version also adds thresholding. This version has improved the filter.
//Variables to control: Choosing HSVplane or other color scheme. Size of image. threshold value. High pass filter.
//Which high pass filter to use rect or circular

//This is for testing version 4. Variables that can be changed: HSVPlane, grayImage, SizeOfImage, ThresholdingValue, 
//filtering the fourier transform.
//This is a version will crop the image into many different potential targets. This version trys to crop without 
//a buffer around the original image using complicated if statements

//This version uses a buffer around the image so that there are no cropping errors on the boarder.


#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void fftShift(Mat image);
void HighPassRect(Mat image, float PercentageLow1);
void CropTargets(Mat imgThresholded, Mat OriginalImage, int SizeReduced);

int main(int argc, char ** argv)
{
	Mat I, rawr, ImageHSV, ReducedIMG, HSVplane, BorderedImage;
	vector<Mat> HSVplanes;
	Mat OriginalImage; //= imread("/home/ucruas/targets/IMG_0579.JPG"); //Target1.jpg IMG_0578.JPG InternetBeach1.jpg SalientTestImage1.PNG test1small.jpg runepic3.png
	OriginalImage = imread(argv[1], 1);
	if ( !OriginalImage.data ){
			printf("No image data \n");
			OriginalImage = imread("/home/ucruas/targets/IMG_0579.JPG");
	}

	int SizeReduced = 5;
	resize(OriginalImage, ReducedIMG, Size(OriginalImage.cols / SizeReduced, OriginalImage.rows / SizeReduced));//resize image

	//namedWindow("ReducedIMG", WINDOW_AUTOSIZE);//WINDOW_NORMAL WINDOW_AUTOSIZE
	//imshow("ReducedIMG", ReducedIMG);

	cvtColor(ReducedIMG, ImageHSV, COLOR_BGR2HSV);
	split(ImageHSV, HSVplanes);
	HSVplane = HSVplanes[2];
	//Mat GreyImage;
	//cvtColor(ReducedIMG, GreyImage, CV_RGB2GRAY);
	//OriginalImage = GreyImage;

	//namedWindow("HSVplane", WINDOW_AUTOSIZE);
	//imshow("HSVplane", HSVplane);////////////////////////////////////////////////////////////

	//int sizereduced = 10;
	//resize(OriginalImage, OriginalImage, Size(OriginalImage.cols / sizereduced, OriginalImage.rows / sizereduced));//resize image
	//resize(OriginalImage, rawr, Size(100,100));//resize image
	I = HSVplane;																										   //cvtColor(OriginalImage, I, CV_BGR2GRAY);
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
	//namedWindow("inverse fourier transform", WINDOW_AUTOSIZE);
	//imshow("inverse fourier transform", InverseFourier);///////////////////////////////////////////////////

	Mat Threshold;
	inRange(InverseFourier, .4, 1, Threshold); //Threshold the image
	//namedWindow("Threshold", WINDOW_AUTOSIZE);
	//imshow("Threshold", Threshold);

	CropTargets(Threshold, OriginalImage, SizeReduced);

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

void CropTargets(Mat imgThresholded, Mat OriginalImage, int SizeReduced) {

	Mat BorderedImage;
	float BorderSize = 0.1;
	int top = (int)(BorderSize*OriginalImage.rows); int bottom = (int)(BorderSize*OriginalImage.rows);
	int left = (int)(BorderSize*OriginalImage.cols); int right = (int)(BorderSize*OriginalImage.cols);
	copyMakeBorder(OriginalImage, BorderedImage, top, bottom, left, right, BORDER_CONSTANT, Scalar::all(0));
	//namedWindow("BoarderedImage", WINDOW_NORMAL);//WINDOW_NORMAL WINDOW_AUTOSIZE
	//imshow("BoarderedImage", BorderedImage);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	//Canny(imgThresholded, imgThresholded, thresh, thresh * 2, 3);
	/// Find contours
	findContours(imgThresholded, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  Get the mass centers:
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	/// Draw contours
	vector<Mat> ROI;
	Mat drawing = Mat::zeros(imgThresholded.size(), CV_8UC3);
	printf("\t Info: Area and Contour Length \n");

	int index = 0;
	RNG rng;
	for (int i = 0; i < contours.size(); i++) {
		int area;
		//if ((contourArea(contours[i]) < 1000) && contourArea(contours[i]) > 75) {

		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		//circle(drawing, mc[i], 4, color, -1, 8, 0);
		//printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n", i, mu[i].m00, contourArea(contours[i]), arcLength(contours[i], true));
		printf(" * Contour[%d] - Area (M_00) = %.2f - X %.2f Y %.2f - Length: %.2f \n", i, mu[i].m00, mc[i].x, mc[i].y, arcLength(contours[i], true));

		area = contourArea(contours[i]);

		int CaptureArea = 100;

		//if ((mc[i].x >= 0) && (mc[i].y >= 0) && (((int)mc[i].x - (area / 4)) * 5 + (area / 2) * 5 <= OriginalImage.cols) && (((int)mc[i].y - (area / 4)) * 5 + (area / 2) * 5 <= OriginalImage.rows)) {
		if ((mc[i].x >= 0) && (mc[i].y >= 0)) {

			//cout << ((int)mc[i].x - (area / 4)) * 5 << " " << ((int)mc[i].y - (area / 4)) * 5 << " " << OriginalImage.cols << " " << OriginalImage.rows <<"\n \n";
			
			//cout << (int)(mc[i].x * SizeReduced) - area * 10 + int(BorderSize*OriginalImage.cols) + (20 * area) << " " << ((int)mc[i].y * SizeReduced) - area * 10 + int(BorderSize*OriginalImage.rows) + (20 * area) << " " << BorderedImage.cols << " " << BorderedImage.rows << "\n \n";
			//ROI.push_back(BorderedImage(Rect((int)(mc[i].x * SizeReduced) - area*10 + int(BorderSize*OriginalImage.cols), ((int)mc[i].y *SizeReduced) - area*10 + int(BorderSize*OriginalImage.rows), (20 * area), (20 * area))));
			
			cout << "x:" << (int)(mc[i].x * SizeReduced) - CaptureArea/2 + int(BorderSize*OriginalImage.cols) + CaptureArea << "y:" << (int)(mc[i].y * SizeReduced) - CaptureArea/2 + int(BorderSize*OriginalImage.rows) + CaptureArea << " " << BorderedImage.cols << " " << BorderedImage.rows << "\n \n";
			ROI.push_back(BorderedImage(Rect((int)(mc[i].x * SizeReduced) - (CaptureArea/2) + int(BorderSize*OriginalImage.cols),(int)(mc[i].y*SizeReduced)- (CaptureArea/2) + int(BorderSize*OriginalImage.rows), CaptureArea, CaptureArea)));
			
			index++;
		}
		//ROI.push_back(OriginalImage(Rect((mc[i].x - (area / 4))*5, (mc[i].y - (area / 4))*5, (area / 2)*5, (area / 2)*5)));
		//cout << index << "\n";
		//index++;

		//}
	}

	//cout << "index: " << index << "\n \n";
	for (int i = 0; i < (index); i++) {
		ostringstream convert;
		convert << i;
		string ROIString = "RegionOfInterest";
		ROIString.append(convert.str());
		string ROIStringFileType = ROIString;
		ROIStringFileType.append(".png");
		imwrite(ROIStringFileType, ROI[i]);
		//namedWindow(ROIString, WINDOW_AUTOSIZE);
		//imshow(ROIString, ROI[i]);
	}
	/// Show in a window
	//namedWindow("Contours", WINDOW_AUTOSIZE);
	//imshow("Contours", drawing);
}
