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
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include "PrintDebug/PrintDebug.h"

using namespace cv;
using namespace std;

void fftShift(Mat image);
void HighPassRect(Mat image, float PercentageLow1);
void CropTargets(Mat imgThresholded, Mat OriginalImage, int SizeReduced, string fileName);

int main(int argc, char ** argv)
{
	PrintDebug pdb("SaliencyDetection");
	int i=0;
	while(1){
		cout<<"loop "<<i++<<" \n";
		// Get list of images
		DIR* dirp;
		if(!(dirp = opendir("cap"))){
			cout<<"Error opening file!\n";
			return 1;
		}
		dirent* currFile;
		while(currFile = readdir(dirp)){
			char filePath[256] = {0};
			char newPath[256] = {0};
			string fileName;
			//cout<<currFile->d_name<<endl;
			if(currFile->d_name[0] == '.'){
				cout<<"DOT!\n";
				continue;
			}
			strcat(filePath, "cap/");
			strcat(newPath, "done/");
			strcat(filePath, currFile->d_name);
			strcat(newPath, currFile->d_name);
			cout<<"HERE\n";
			fileName = currFile->d_name;

			Mat I, rawr, ImageHSV, ReducedIMG, HSVplane, BorderedImage;
			vector<Mat> HSVplanes;
			Mat OriginalImage; 

			cout<<"HERE\n";
			OriginalImage = imread(filePath);
			if( !OriginalImage.data ){
				OriginalImage = imread(filePath);
			}
			cout<<"HERE\n";

			int SizeReduced = 5;
			resize(OriginalImage, 
					ReducedIMG, 
					Size(OriginalImage.cols / SizeReduced, 
							OriginalImage.rows / SizeReduced
					)
			);//resize image
			cout<<"HERE\n";

			cvtColor(ReducedIMG, ImageHSV, COLOR_BGR2HSV);
			split(ImageHSV, HSVplanes);
			HSVplane = HSVplanes[2];
			I = HSVplane;	//cvtColor(OriginalImage, I, CV_BGR2GRAY);
			Mat padded;    //expand input image to optimal size
			int m = getOptimalDFTSize(I.rows);
			// on the border add zero values
			cout<<"HERE\n";
			int n = getOptimalDFTSize(I.cols); 
			copyMakeBorder(I, 
							padded, 
							0, 
							m-I.rows, 
							0, 
							n-I.cols, 
							BORDER_CONSTANT, 
							Scalar::all(0)
			);

			Mat planes[] = { Mat_<float>(padded), 
								Mat::zeros(padded.size(), 
								CV_32F) };
			Mat complexI;
			// Add to the expanded another plane with zeros
			merge(planes, 2, complexI);  
			// this way the result may fit in the source matrix
			dft(complexI, complexI);            

			split(complexI, planes);
			Mat magI, phaseI;
			cartToPolar(planes[0], planes[1], magI, phaseI);

			magI += Scalar::all(1);// switch to logarithmic scale
			log(magI, magI);

			fftShift(magI);
			// Transform the matrix with float values into a
			normalize(magI, magI, 0, 1, NORM_MINMAX); 
			fftShift(magI);

			Mat magAverage, magRes, Sum, InverseFourier, kernel;

			medianBlur(magI, magAverage, 5);

			magRes = magI - magAverage;
			exp(magRes, magRes);

			polarToCart(magRes, phaseI, planes[0], planes[1]);


			merge(planes, 2, Sum);

			dft(Sum, InverseFourier, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
			pow(InverseFourier, 2, InverseFourier);
			GaussianBlur(InverseFourier, InverseFourier, Size(5, 5), 0);
			normalize(InverseFourier, InverseFourier, 0, 1, NORM_MINMAX);

			Mat Threshold;
			inRange(InverseFourier, .4, 1, Threshold); //Threshold the image
		

			fileName = fileName.substr(0,fileName.size()-4);
			CropTargets(Threshold, OriginalImage, SizeReduced, fileName);
			rename(filePath,newPath);

			//waitKey();
		}
		closedir(dirp);
	}
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

void CropTargets(Mat imgThresholded, Mat OriginalImage, int SizeReduced, string FileName){

	Mat BorderedImage;
	float BorderSize = 0.1;
	int top = (int)(BorderSize*OriginalImage.rows); int bottom = (int)(BorderSize*OriginalImage.rows);
	int left = (int)(BorderSize*OriginalImage.cols); int right = (int)(BorderSize*OriginalImage.cols);
	copyMakeBorder(OriginalImage, BorderedImage, top, bottom, left, right, BORDER_CONSTANT, Scalar::all(0));

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	/// Find contours
	findContours(imgThresholded, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mu[i] = moments(contours[i], false);
	}

	///  Get the mass centers:
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	/// Draw contours
	vector<Mat> ROI;
	Mat drawing = Mat::zeros(imgThresholded.size(), CV_8UC3);
	//printf("\t Info: Area and Contour Length \n");

	int index = 0;
	RNG rng;
	for (int i = 0; i < contours.size(); i++) {
		int area;
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		//printf(" * Contour[%d] - Area (M_00) = %.2f - X %.2f Y %.2f - Length: %.2f \n", i, mu[i].m00, mc[i].x, mc[i].y, arcLength(contours[i], true));

		area = contourArea(contours[i]);

		int CaptureArea = 100;

		if ((mc[i].x >= 0) && (mc[i].y >= 0)) {
			//cout << "x:" << (int)(mc[i].x * SizeReduced) - CaptureArea/2 + int(BorderSize*OriginalImage.cols) + CaptureArea << "y:" << (int)(mc[i].y * SizeReduced) - CaptureArea/2 + int(BorderSize*OriginalImage.rows) + CaptureArea << " " << BorderedImage.cols << " " << BorderedImage.rows << "\n \n";
			ROI.push_back(BorderedImage(Rect((int)(mc[i].x * SizeReduced) - (CaptureArea/2) + int(BorderSize*OriginalImage.cols),(int)(mc[i].y*SizeReduced)- (CaptureArea/2) + int(BorderSize*OriginalImage.rows), CaptureArea, CaptureArea)));
			
			index++;
		}
	}
	FileName.append(":");
	string name = "crp/";
	name.append(FileName);
	for (int i = 0; i < (index); i++) {
		ostringstream convert;
		convert << i;
		string ROIString = name;
		ROIString.append(convert.str());
		string ROIStringFileType = ROIString;
		ROIStringFileType.append(".png");
		imwrite(ROIStringFileType, ROI[i]);
		string command = "scp ";
		command.append(ROIString);
		command.append(".png ");
		//command.append("frederick@192.168.50.230:~/Hold");
		command.append("rachel@192.168.50.156:~/UAV/Payload/Payload/ADLC/Images");
		system(command.c_str());
	}
}
#include "PrintDebug/PrintDebug.cpp"
