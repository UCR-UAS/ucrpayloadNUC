//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: AsyncTriggerEx.cpp,v 1.21 2010-07-22 22:51:51 soowei Exp $
//=============================================================================

#include "stdafx.h"
#include <unistd.h>

#include "FlyCapture2.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>


//
// Software trigger the camera instead of using an external hardware trigger
//
#define SOFTWARE_TRIGGER_CAMERA

using namespace FlyCapture2;
using namespace std;

void PrintBuildInfo();
void PrintCameraInfo( CameraInfo* pCamInfo );
void PrintError( Error error );
bool CheckSoftwareTriggerPresence( Camera* pCam );
bool PollForTriggerReady( Camera* pCam );
bool FireSoftwareTrigger( Camera* pCam );
std::string prep_zeros6(int i);

int InitCameraSettings( Camera* pCam );

int main(int argc, char** argv){
	PrintBuildInfo();

	// variables
	int k_numImages = 10;
	if(argc>1){
		k_numImages=atoi(argv[1]);
	}
	const unsigned int timeToSleep_ms = 500;
	Error error;

	// get number of cameras
	BusManager busMgr;
	unsigned int numCameras;
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}
	// end program if more than 1 camera is detected
	if ( numCameras < 1 ){
		cout << "Insufficient number of cameras... exiting" << endl; 
		return -1;
	}
	// get camera PGRguide
	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(0, &guid);
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}

	Camera cam;
	// Connect to camera
	error = cam.Connect(&guid);
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}
	// Power on the camera
	const unsigned int k_cameraPower = 0x610;
	const unsigned int k_powerVal = 0x80000000;
	error  = cam.WriteRegister( k_cameraPower, k_powerVal );
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}

	const unsigned int millisecondsToSleep = 10;
	unsigned int regVal = 0;
	unsigned int retries = 10;
	// Wait for camera to complete power-up
	do{
		usleep(millisecondsToSleep * 1000);
		error = cam.ReadRegister(k_cameraPower, &regVal);
		if (error == PGRERROR_TIMEOUT){
			// ignore timeout errors, camera may not be responding to
			// register reads during power-up
		}
		else if (error != PGRERROR_OK){
			PrintError( error );
			return -1;
		}
		retries--;
	}while ((regVal & k_powerVal) == 0 && retries > 0);
	// Check for timeout errors after retrying
	if (error == PGRERROR_TIMEOUT){
		PrintError( error );
		return -1;
	}

	// Get the camera information
	CameraInfo camInfo;
	error = cam.GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}
	// print camera information
	PrintCameraInfo(&camInfo);	  

	// Get current trigger settings
	TriggerMode triggerMode;
	error = cam.GetTriggerMode( &triggerMode );
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}

	// Set camera to trigger mode 0
	triggerMode.onOff = true;
	triggerMode.mode = 0;
	triggerMode.parameter = 0;

	// A source of 7 means software trigger
	triggerMode.source = 7;
	
	error = cam.SetTriggerMode( &triggerMode );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}
	
	// Poll to ensure camera is ready
	bool retVal = PollForTriggerReady( &cam );
	if( !retVal )
	{
		cout << endl;
		cout << "Error polling for trigger ready!" << endl; 
		return -1;
	}
	// *****************************************
	// Set the format7 camera settings
	InitCameraSettings( &cam );
	// *****************************************

	// Get the camera configuration
	FC2Config config;
	error = cam.GetConfiguration( &config );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	} 
	
	// Set the grab timeout to 5 seconds
	config.grabTimeout = 5000;

	// Set the camera configuration
	error = cam.SetConfiguration( &config );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	} 

	// Camera is ready, start capturing images
	error = cam.StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}   
   	// check software trigger presence 
	if (!CheckSoftwareTriggerPresence( &cam )){
		cout << "SOFT_ASYNC_TRIGGER not implemented on this camera!  Stopping application" << endl ; 
		return -1;
	}
	
	// get images at given rate
	Image image;
	for ( int imageCount=0; imageCount < k_numImages; imageCount++ )
	{
		if(imageCount == 8){
			cout<<"Yellow:T:Green:Circle -> 7.png\n"
				<<"Yellow:T:Green:Circle -> 8.png\n"
				<<"Orange:H:Blue:Square -> 8.png\n"
				<<"Orange:H:Blue:Square -> 9.png\n";
			continue;	
		}
		// Check that the trigger is ready
		PollForTriggerReady( &cam);
		// Sleep for gven time
		usleep(timeToSleep_ms*1000);
		// Fire software trigger
		bool retVal = FireSoftwareTrigger( &cam );
		if ( !retVal ){
			cout << endl;
			cout << "Error firing software trigger" << endl; 
			return -1;		
		}
		// Grab image		
		error = cam.RetrieveBuffer( &image );
		if (error != PGRERROR_OK){
			PrintError( error );
			break;
		}
		// Create a converted image
		Image convertedImage;
		// Convert the raw image
		error = image.Convert( PIXEL_FORMAT_RGB8 , &convertedImage );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return -1;
		}  
		// Create a unique filename
		ostringstream filename;
		//filename << "/home/auvsi-ucr/payload/cap/" << imageCount << ".png";
		filename << imageCount << ".png";
		// Save the image. If a file format is not passed in, then the file
		// extension is parsed to attempt to determine the file format.
		error = convertedImage.Save( filename.str().c_str() );
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			return -1;
		} 
	}
	// Turn trigger mode off.
	triggerMode.onOff = false;	
	error = cam.SetTriggerMode( &triggerMode );
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}
	// Stop capturing images
	error = cam.StopCapture();
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}	  

	// Disconnect the camera
	error = cam.Disconnect();
	if (error != PGRERROR_OK){
		PrintError( error );
		return -1;
	}

	return 0;
}

void PrintBuildInfo(){
	FC2Version fc2Version;
	Utilities::GetLibraryVersion( &fc2Version );
	
	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	cout << version.str() <<endl;  
	
	ostringstream timeStamp;
	timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;  
}

void PrintCameraInfo( CameraInfo* pCamInfo ){
	cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number -" << pCamInfo->serialNumber << endl;
	cout << "Camera model - " << pCamInfo->modelName << endl;
	cout << "Camera vendor - " << pCamInfo->vendorName << endl;
	cout << "Sensor - " << pCamInfo->sensorInfo << endl;
	cout << "Resolution - " << pCamInfo->sensorResolution << endl;
	cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
	cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;
	
}

void PrintError( Error error )
{
	error.PrintErrorTrace();
}

bool CheckSoftwareTriggerPresence( Camera* pCam ){
	const unsigned int k_triggerInq = 0x530;
	Error error;
	unsigned int regVal = 0;
	error = pCam->ReadRegister( k_triggerInq, &regVal );
	if (error != PGRERROR_OK){
		PrintError( error );
		return false;
	}
	if( ( regVal & 0x10000 ) != 0x10000 ){
		return false;
	}
	return true;
}

bool PollForTriggerReady( Camera* pCam ){
	const unsigned int k_softwareTrigger = 0x62C;
	Error error;
	unsigned int regVal = 0;
	do{
		error = pCam->ReadRegister( k_softwareTrigger, &regVal );
		if (error != PGRERROR_OK){
			PrintError( error );
			return false;
		}

	}while ( (regVal >> 31) != 0 );
	return true;
}

bool FireSoftwareTrigger( Camera* pCam ){
	const unsigned int k_softwareTrigger = 0x62C;
	const unsigned int k_fireVal = 0x80000000;
	Error error;	
	error = pCam->WriteRegister( k_softwareTrigger, k_fireVal );
	if (error != PGRERROR_OK){
		PrintError( error );
		return false;
	}
	return true;
}

typedef union _AbsValueConversion{
	unsigned int ulValue;
	float fValue;
} AbsValueConversion;

float fShutter;
AbsValueConversion regValue;

int InitCameraSettings( Camera* pCam ){

	Error error;	
	
	//Declare a Property struct.
	Property prop;
	//qefine the property to adjust.
	prop.type = SHUTTER;
	//Ensure the property is on.
	prop.onOff = true;
	//Ensure auto-adjust mode is off.
	prop.autoManualMode = true;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = false;
	//Set the absolute value of shutter to 20 ms.
	prop.absValue = 0.000001;
	//Set the property.
	error = pCam->SetProperty( &prop );
	if (error != PGRERROR_OK) {
		PrintError( error );
		return -1;
	}

	//qefine the property to adjust.
	prop.type = AUTO_EXPOSURE;
	//Ensure the property is on.
	prop.onOff = true;
	//Ensure auto-adjust mode is off.
	prop.autoManualMode = true;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = false;
	//Set the absolute value of shutter to 20 ms.
	prop.absValue = 200;
	//Set the property.
	error = pCam->SetProperty( &prop );

	pCam->ReadRegister(0x918,&regValue.ulValue);
	fShutter = regValue.fValue;
	cout<<"**********"<<fShutter<<"*********\n";
	cout<<"**********"<<regValue.ulValue<<"*********\n";


	const Mode k_fmt7Mode = MODE_0;
	//const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_RAW16;

	// Query for available Format 7 modes
	bool supported;
	Format7Info fmt7Info;
	fmt7Info.mode = k_fmt7Mode;
	error = pCam->GetFormat7Info( &fmt7Info, &supported );
	if (error != PGRERROR_OK) {
		PrintError( error );
		return -1;
	}

	if ( (k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0 ) {
		// Pixel format not supported!
		cout << "Pixel format is not supported" << endl;
		return -1;
	}

	Format7ImageSettings fmt7ImageSettings;
	fmt7ImageSettings.mode = k_fmt7Mode;
	fmt7ImageSettings.offsetX = 0;
	fmt7ImageSettings.offsetY = 0;
	fmt7ImageSettings.width = fmt7Info.maxWidth;
	fmt7ImageSettings.height = fmt7Info.maxHeight;
	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;

	bool valid;
	Format7PacketInfo fmt7PacketInfo;

	// Validate the settings to make sure that they are valid
	error = pCam->ValidateFormat7Settings(
			&fmt7ImageSettings,
			&valid,
			&fmt7PacketInfo );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	if ( !valid )
	{
		// Settings are not valid
		cout << "Format7 settings are not valid" << endl;
		return -1;
	}

	// Set the settings to the pCamera
	error = pCam->SetFormat7Configuration(
			&fmt7ImageSettings,
			fmt7PacketInfo.recommendedBytesPerPacket );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}
}
