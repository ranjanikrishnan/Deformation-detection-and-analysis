/*
This example describes the FIRST STEPS of handling Baumer-GAPI SDK.
The given source code applies to handling one system, one camera and eight images.
Please see "Baumer-GAPI SDK Programmer's Guide" chapter 5.5
*/

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <time.h>
#include <windows.h>

#include "mysql_connection.h"
#include "bgapi2_genicam.hpp"
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp> 
#include <opencv2\video.hpp>
#include <opencv2\imgcodecs.hpp>
#include <..\..\..\baumer\Components\Examples\C++\build_vs14_c++_WIN_64\0_Common\005_PixelTransformation\dbconnector.h>

//#include <boost/log/core.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/sinks/text_file_backend.hpp>
//#include <boost/log/utility/setup/file.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/sources/severity_logger.hpp>
//#include <boost/log/sources/record_ostream.hpp>


using namespace BGAPI2;
using namespace cv;
using namespace std;

//namespace logging = boost::log;
//namespace src = boost::log::sources;
//namespace sinks = boost::log::sinks;
//namespace keywords = boost::log::keywords;


//void init()
//{
//	logging::add_file_log
//	(
//		keywords::file_name = "image-acquisition_%N.log",
//		keywords::rotation_size = 10 * 1024 * 1024,
//		keywords::format = "[%TimeStamp%]: %Message%"
//	);
//
//	logging::core::get()->set_filter
//	(
//		logging::trivial::severity >= logging::trivial::info
//	);
//}


const std::string currentDateTime(const char* format) {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), format, &tstruct);

	return buf;
}


int main()
{
	/***********************
	 Initialize the logger
	************************/
	//init();

	// OPENCV VARIABLE DECLARATIONS
	cv::Mat openCvImage;                           

	//BAUMER VARIABLE DECLARATIONS				
	BGAPI2::ImageProcessor * imgProcessor = NULL;

	BGAPI2::SystemList *systemList = NULL;
	BGAPI2::System * pSystem = NULL;
	BGAPI2::String sSystemID;

	BGAPI2::InterfaceList *interfaceList = NULL;
	BGAPI2::Interface * pInterface = NULL;
	BGAPI2::String sInterfaceID;

	BGAPI2::DeviceList *deviceList = NULL;
	BGAPI2::Device * pDevice = NULL;
	BGAPI2::String sDeviceID;

	BGAPI2::DataStreamList *datastreamList = NULL;
	BGAPI2::DataStream * pDataStream = NULL;
	BGAPI2::String sDataStreamID;

	BGAPI2::BufferList *bufferList = NULL;
	BGAPI2::Buffer * pBuffer = NULL;
	BGAPI2::String sBufferID;
	int returncode = 0;

	//MYSQL VARIABLE DECLARATIONS
	sql::Connection *connection = getMySQLConn();
	int image_index = getNextImageId(connection) + 1;
	string image_path = "C:/Users/Ranjani/Pictures/tiff_images/" + currentDateTime("%Y-%m-%d") + "/";
	string image_name = currentDateTime("%H-%M-%S--") + to_string(image_index);
	string tiff_image_path = image_path + image_name + ".tiff";
	cout << "path: " << tiff_image_path << endl;

	//Create Windows Directory
	std::wstring stemp = std::wstring(image_path.begin(), image_path.end());
	LPCWSTR directory_path = stemp.c_str();

	

	//ImageDetails(connection, tiff_image_path);

	std::cout << std::endl;
	std::cout << "##########################################################" << std::endl;
	std::cout << "#	IMAGE ACQUISITION				 #" << std::endl;
	std::cout << "##########################################################" << std::endl;
	std::cout << std::endl << std::endl;

	/***********************************************************************************************************************/
	/************************************************
		TEMPORARY CODE
	************************************************/
	// OpenCV code to process an existing image
	string imageName =  "croppedTest03.jpg"; //"IMG0006-old.bmp";
	string imagePath = "C:/Users/Ranjani/Desktop/baumer/Components/Examples/C++/build_vs14_c++_WIN_64/0_Common/005_PixelTransformation/baumerimages/" + imageName;
	string jpeg_image_path = image_path + image_name + ".jpg";
	Mat image = imread(imagePath.c_str(), IMREAD_COLOR); // Read the file

	if (image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	namedWindow("Display window", WINDOW_NORMAL); // Create a window for display.
	imshow("Display window", image);              // Show our image inside it.

	/*
	*****************************************************
	Store Image using OPENCV and write image path to DB
	*****************************************************
	*/
	//Create directory if it doesn't exist
	if (CreateDirectory(directory_path, NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		//Write the image to the directory
		if (imwrite(tiff_image_path, image)) {
			cout << "Image successfully stored in directory. Writing image path to DB." << endl;
			ImageDetails(connection, image_index, tiff_image_path);
		}
		else {
			cout << "Unable to store tiff the image." << endl;
		}
	}
	
	Mat src, src_gray;
	src = image;
	cvtColor(src, src_gray, CV_BGR2GRAY);	/// Convert it to gray

	/// Reduce the noise so we avoid false circle detection
	GaussianBlur(src_gray, src_gray, Size(9, 9), 2, 2);

	vector<Vec3f> circles;

	/// Apply the Hough Transform to find the circles
	HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, 20, 90, 30, 0, 0);
	//45-50 works well for param1 and param2 respectively for on-site images

	vector<Vec3f> circles_new;
	for (size_t i = 0; i < 18; i++) {
		circles_new.push_back(Vec3f(circles[0][0] + (i * 10), circles[0][1] + (i * 10), circles[0][2]));
	}
	for (size_t i = 0; i < circles_new.size(); i++) {
		Point center(cvRound(circles_new[i][0]), cvRound(circles_new[i][1]));
		int radius = cvRound(circles_new[i][2]);
		// circle center
		circle(image, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		// circle outline
		circle(image, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	}
	/// Draw the circles detected
	//for (size_t i = 0; i < circles.size(); i++)
	//{
	//	cout << i << "." << endl;
	//	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	//	int radius = cvRound(circles[i][2]);
	//	// circle center
	//	circle(image, center, 3, Scalar(0, 255, 0), -1, 8, 0);
	//	// circle outline
	//	circle(image, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	//}

	/// Show your results
	namedWindow("Hough Circle Transform Demo", WINDOW_NORMAL);
	imshow("Hough Circle Transform Demo", src);

	for (size_t i = 0; i < circles_new.size(); i++)
	{
		Point center(cvRound(circles_new[i][0]), cvRound(circles_new[i][1]));
		cout << center <<endl;
	}
	/************************************************
	Store the coordinates into the DB 
	Here we store the following coordinates temporarily
	[200,600],[250,600],[300,600]
	[200,500],[250,500],[300,500]
	[200,400],[250,400],[300,400]

	[600,600],[650,600],[700,600]
	[600,500],[650,500],[700,500]
	[600,400],[650,400],[700,400]
	************************************************/
	vector<Vec3i> coordinates;
	//left window
	coordinates.push_back(Vec3i(206, 600, circles[0][2]));
	coordinates.push_back(Vec3i(250, 600, circles[0][2]));
	coordinates.push_back(Vec3i(300, 600, circles[0][2]));
	coordinates.push_back(Vec3i(200, 500, circles[0][2]));
	coordinates.push_back(Vec3i(250, 500, circles[0][2]));
	coordinates.push_back(Vec3i(300, 500, circles[0][2]));
	coordinates.push_back(Vec3i(200, 400, circles[0][2]));
	coordinates.push_back(Vec3i(250, 400, circles[0][2]));
	coordinates.push_back(Vec3i(300, 400, circles[0][2]));
	//right window
	coordinates.push_back(Vec3i(600, 600, circles[0][2]));
	coordinates.push_back(Vec3i(650, 600, circles[0][2]));
	coordinates.push_back(Vec3i(700, 600, circles[0][2]));
	coordinates.push_back(Vec3i(600, 500, circles[0][2]));
	coordinates.push_back(Vec3i(650, 500, circles[0][2]));
	coordinates.push_back(Vec3i(700, 500, circles[0][2]));
	coordinates.push_back(Vec3i(600, 400, circles[0][2]));
	coordinates.push_back(Vec3i(650, 400, circles[0][2]));
	coordinates.push_back(Vec3i(700, 400, circles[0][2]));
	
	coordinatesDetails(connection, image_index, coordinates);

	/*
	****************************************************************
	coordinate differences are calculated
	****************************************************************
	*/
	updateCoordinateDiff(connection, image_index, image_index - 1);
	/*
	****************************************************************
	coordinate distances are calculated
	****************************************************************
	*/

	waitKey(0); // Wait for a keystroke in the window

	/***********************************************************************************************************************/
	//Load image processor
	try
	{
		imgProcessor = BGAPI2::ImageProcessor::GetInstance();
		if (imgProcessor->GetNodeList()->GetNodePresent("DemosaicingMethod") == true)
		{
			imgProcessor->GetNodeList()->GetNode("DemosaicingMethod")->SetString("NearestNeighbor"); // NearestNeighbor, Bilinear3x3, Baumer5x5
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	/*
	***************************************
		Capture Image through Baumer Camera
	****************************************
	*/
	//COUNTING AVAILABLE SYSTEMS (TL producers) 
	try
	{
		systemList = SystemList::GetInstance();
		systemList->Refresh();
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}


	//OPEN THE FIRST SYSTEM IN THE LIST WITH A CAMERA CONNECTED
	try
	{
		for (SystemList::iterator sysIterator = systemList->begin(); sysIterator != systemList->end(); sysIterator++)
		{

			try
			{
				sysIterator->second->Open();
				sSystemID = sysIterator->first;
				
				try
				{
					interfaceList = sysIterator->second->GetInterfaces();
					//COUNT AVAILABLE INTERFACES
					interfaceList->Refresh(100); // timeout of 100 msec
				}
				catch (BGAPI2::Exceptions::IException& ex)
				{
					returncode = 0 == returncode ? 1 : returncode;
					std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
					std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
					std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
				}


				//OPEN THE NEXT INTERFACE IN THE LIST
				try
				{
					for (InterfaceList::iterator ifIterator = interfaceList->begin(); ifIterator != interfaceList->end(); ifIterator++)
					{
						try
						{
							ifIterator->second->Open();
							//search for any camera is connetced to this interface
							deviceList = ifIterator->second->GetDevices();
							deviceList->Refresh(100);
							if (deviceList->size() == 0)
							{
								ifIterator->second->Close();
							}
							else
							{
								sInterfaceID = ifIterator->first;
								if (ifIterator->second->GetTLType() == "GEV")
								{
									bo_int64 iIpAddress = ifIterator->second->GetNode("GevInterfaceSubnetIPAddress")->GetInt();
									bo_int64 iSubnetMask = ifIterator->second->GetNode("GevInterfaceSubnetMask")->GetInt();
								}
								if (ifIterator->second->GetTLType() == "U3V")
								{
									//do nothing
								}
								break;
							}
						}
						catch (BGAPI2::Exceptions::ResourceInUseException& ex)
						{
							returncode = 0 == returncode ? 1 : returncode;
							std::cout << " Interface " << ifIterator->first << " already opened " << std::endl;
							std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
						}
					}
				}
				catch (BGAPI2::Exceptions::IException& ex)
				{
					returncode = 0 == returncode ? 1 : returncode;
					std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
					std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
					std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
				}


				//if a camera is connected to the system interface then leave the system loop
				if (sInterfaceID != "")
				{
					break;
				}
			}
			catch (BGAPI2::Exceptions::ResourceInUseException& ex)
			{
				returncode = 0 == returncode ? 1 : returncode;
				std::cout << " System " << sysIterator->first << " already opened " << std::endl;
				std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
			}
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	if (sSystemID == "")
	{
		std::cout << " No System found " << std::endl;
		std::cout << std::endl << "End" << std::endl << "Input any number to close the program:";
		int endKey = 0;
		std::cin >> endKey;
		BGAPI2::SystemList::ReleaseInstance();
		//RELEASE IMAGE PROCESSOR
		BGAPI2::ImageProcessor::ReleaseInstance();
		return returncode;
	}
	else
	{
		pSystem = (*systemList)[sSystemID];
	}


	if (sInterfaceID == "")
	{
		std::cout << " No camera found " << sInterfaceID << std::endl;
		std::cout << std::endl << "End" << std::endl << "Input any number to close the program:";
		int endKey = 0;
		std::cin >> endKey;
		pSystem->Close();
		BGAPI2::SystemList::ReleaseInstance();
		//RELEASE IMAGE PROCESSOR
		BGAPI2::ImageProcessor::ReleaseInstance();
		return returncode;
	}
	else
	{
		pInterface = (*interfaceList)[sInterfaceID];
	}


	try
	{
		//COUNTING AVAILABLE CAMERAS
		deviceList = pInterface->GetDevices();
		deviceList->Refresh(100);
		std::cout << "5.1.6   Detected devices:         " << deviceList->size() << std::endl;

		//DEVICE INFORMATION BEFORE OPENING
		for (DeviceList::iterator devIterator = deviceList->begin(); devIterator != deviceList->end(); devIterator++)
		{
			std::cout << "  5.2.3   Device DeviceID:        " << devIterator->first << std::endl;
			std::cout << "          Device Model:           " << devIterator->second->GetModel() << std::endl;
			std::cout << "          Device SerialNumber:    " << devIterator->second->GetSerialNumber() << std::endl;
			std::cout << "          Device Vendor:          " << devIterator->second->GetVendor() << std::endl;
			std::cout << "          Device TLType:          " << devIterator->second->GetTLType() << std::endl;
			std::cout << "          Device AccessStatus:    " << devIterator->second->GetAccessStatus() << std::endl;
			std::cout << "          Device UserID:          " << devIterator->second->GetDisplayName() << std::endl << std::endl;
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}


	std::cout << "DEVICE" << std::endl;
	std::cout << "######" << std::endl << std::endl;

	//OPEN THE FIRST CAMERA IN THE LIST
	try
	{
		for (DeviceList::iterator devIterator = deviceList->begin(); devIterator != deviceList->end(); devIterator++)
		{
			try
			{
				std::cout << "5.1.7   Open first device " << std::endl;
				std::cout << "          Device DeviceID:        " << devIterator->first << std::endl;
				std::cout << "          Device Model:           " << devIterator->second->GetModel() << std::endl;
				std::cout << "          Device SerialNumber:    " << devIterator->second->GetSerialNumber() << std::endl;
				std::cout << "          Device Vendor:          " << devIterator->second->GetVendor() << std::endl;
				std::cout << "          Device TLType:          " << devIterator->second->GetTLType() << std::endl;
				std::cout << "          Device AccessStatus:    " << devIterator->second->GetAccessStatus() << std::endl;
				std::cout << "          Device UserID:          " << devIterator->second->GetDisplayName() << std::endl << std::endl;
				devIterator->second->Open();
				sDeviceID = devIterator->first;
				std::cout << "        Opened device - RemoteNodeList Information " << std::endl;
				std::cout << "          Device AccessStatus:    " << devIterator->second->GetAccessStatus() << std::endl;

				//SERIAL NUMBER
				if (devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceSerialNumber"))
					std::cout << "          DeviceSerialNumber:     " << devIterator->second->GetRemoteNode("DeviceSerialNumber")->GetValue() << std::endl;
				else if (devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceID"))
					std::cout << "          DeviceID (SN):          " << devIterator->second->GetRemoteNode("DeviceID")->GetValue() << std::endl;
				else
					std::cout << "          SerialNumber:           Not Available " << std::endl;

				//DISPLAY DEVICEMANUFACTURERINFO
				if (devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceManufacturerInfo"))
					std::cout << "          DeviceManufacturerInfo: " << devIterator->second->GetRemoteNode("DeviceManufacturerInfo")->GetValue() << std::endl;


				//DISPLAY DEVICEFIRMWAREVERSION OR DEVICEVERSION
				if (devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceFirmwareVersion"))
					std::cout << "          DeviceFirmwareVersion:  " << devIterator->second->GetRemoteNode("DeviceFirmwareVersion")->GetValue() << std::endl;
				else if (devIterator->second->GetRemoteNodeList()->GetNodePresent("DeviceVersion"))
					std::cout << "          DeviceVersion:          " << devIterator->second->GetRemoteNode("DeviceVersion")->GetValue() << std::endl;
				else
					std::cout << "          DeviceVersion:          Not Available " << std::endl;

				if (devIterator->second->GetTLType() == "GEV")
				{
					std::cout << "          GevCCP:                 " << devIterator->second->GetRemoteNode("GevCCP")->GetValue() << std::endl;
					std::cout << "          GevCurrentIPAddress:    " << ((devIterator->second->GetRemoteNode("GevCurrentIPAddress")->GetInt() & 0xff000000) >> 24) << "." << ((devIterator->second->GetRemoteNode("GevCurrentIPAddress")->GetInt() & 0x00ff0000) >> 16) << "." << ((devIterator->second->GetRemoteNode("GevCurrentIPAddress")->GetInt() & 0x0000ff00) >> 8) << "." << (devIterator->second->GetRemoteNode("GevCurrentIPAddress")->GetInt() & 0x0000ff) << std::endl;
					std::cout << "          GevCurrentSubnetMask:   " << ((devIterator->second->GetRemoteNode("GevCurrentSubnetMask")->GetInt() & 0xff000000) >> 24) << "." << ((devIterator->second->GetRemoteNode("GevCurrentSubnetMask")->GetInt() & 0x00ff0000) >> 16) << "." << ((devIterator->second->GetRemoteNode("GevCurrentSubnetMask")->GetInt() & 0x0000ff00) >> 8) << "." << (devIterator->second->GetRemoteNode("GevCurrentSubnetMask")->GetInt() & 0x0000ff) << std::endl;
				}
				std::cout << "  " << std::endl;
				break;
			}
			catch (BGAPI2::Exceptions::ResourceInUseException& ex)
			{
				returncode = 0 == returncode ? 1 : returncode;
				std::cout << " Device  " << devIterator->first << " already opened " << std::endl;
				std::cout << " ResourceInUseException: " << ex.GetErrorDescription() << std::endl;
			}
			catch (BGAPI2::Exceptions::AccessDeniedException& ex)
			{
				returncode = 0 == returncode ? 1 : returncode;
				std::cout << " Device  " << devIterator->first << " already opened " << std::endl;
				std::cout << " AccessDeniedException " << ex.GetErrorDescription() << std::endl;
			}
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	if (sDeviceID == "")
	{
		std::cout << " No Device found " << sDeviceID << std::endl;
		std::cout << std::endl << "End" << std::endl << "Input any number to close the program:";
		int endKey = 0;
		std::cin >> endKey;
		pInterface->Close();
		pSystem->Close();
		BGAPI2::SystemList::ReleaseInstance();
		//RELEASE IMAGE PROCESSOR
		BGAPI2::ImageProcessor::ReleaseInstance();
		return returncode;
	}
	else
	{
		pDevice = (*deviceList)[sDeviceID];
	}


	std::cout << "DEVICE PARAMETER SETUP" << std::endl;
	std::cout << "######################" << std::endl << std::endl;

	try
	{
		//SET TRIGGER MODE OFF (FreeRun)
		pDevice->GetRemoteNode("TriggerMode")->SetString("Off");
		std::cout << "         TriggerMode:             " << pDevice->GetRemoteNode("TriggerMode")->GetValue() << std::endl;
		std::cout << std::endl;
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}


	std::cout << "DATA STREAM LIST" << std::endl;
	std::cout << "################" << std::endl << std::endl;

	try
	{
		//COUNTING AVAILABLE DATASTREAMS
		datastreamList = pDevice->GetDataStreams();
		datastreamList->Refresh();
		std::cout << "5.1.8   Detected datastreams:     " << datastreamList->size() << std::endl;

		//DATASTREAM INFORMATION BEFORE OPENING
		for (DataStreamList::iterator dstIterator = datastreamList->begin(); dstIterator != datastreamList->end(); dstIterator++)
		{
			std::cout << "  5.2.4   DataStream ID:          " << dstIterator->first << std::endl << std::endl;
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}


	std::cout << "DATA STREAM" << std::endl;
	std::cout << "###########" << std::endl << std::endl;

	//OPEN THE FIRST DATASTREAM IN THE LIST
	try
	{
		for (DataStreamList::iterator dstIterator = datastreamList->begin(); dstIterator != datastreamList->end(); dstIterator++)
		{
			std::cout << "5.1.9   Open first datastream " << std::endl;
			std::cout << "          DataStream ID:          " << dstIterator->first << std::endl << std::endl;
			dstIterator->second->Open();
			sDataStreamID = dstIterator->first;
			std::cout << "        Opened datastream - NodeList Information" << std::endl;
			std::cout << "          StreamAnnounceBufferMinimum:  " << dstIterator->second->GetNode("StreamAnnounceBufferMinimum")->GetValue() << std::endl;
			if (dstIterator->second->GetTLType() == "GEV")
			{
				std::cout << "          StreamDriverModel:            " << dstIterator->second->GetNode("StreamDriverModel")->GetValue() << std::endl;
			}
			std::cout << "  " << std::endl;
			break;
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	if (sDataStreamID == "")
	{
		std::cout << " No DataStream found " << sDataStreamID << std::endl;
		std::cout << std::endl << "End" << std::endl << "Input any number to close the program:";
		int endKey = 0;
		std::cin >> endKey;
		pDevice->Close();
		pInterface->Close();
		pSystem->Close();
		BGAPI2::SystemList::ReleaseInstance();
		//RELEASE IMAGE PROCESSOR
		BGAPI2::ImageProcessor::ReleaseInstance();
		return returncode;
	}
	else
	{
		pDataStream = (*datastreamList)[sDataStreamID];
	}


	std::cout << "BUFFER LIST" << std::endl;
	std::cout << "###########" << std::endl << std::endl;

	try
	{
		//BufferList
		bufferList = pDataStream->GetBufferList();

		// 4 buffers using internal buffer mode
		for (int i = 0; i<4; i++)
		{
			pBuffer = new BGAPI2::Buffer();
			bufferList->Add(pBuffer);
		}
		std::cout << "5.1.10   Announced buffers:       " << bufferList->GetAnnouncedCount() << " using " << pBuffer->GetMemSize() * bufferList->GetAnnouncedCount() << " [bytes]" << std::endl;
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	try
	{
		for (BufferList::iterator bufIterator = bufferList->begin(); bufIterator != bufferList->end(); bufIterator++)
		{
			bufIterator->second->QueueBuffer();
		}
		std::cout << "5.1.11   Queued buffers:          " << bufferList->GetQueuedCount() << std::endl;
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}
	std::cout << " " << std::endl;


	std::cout << "CAMERA START" << std::endl;
	std::cout << "############" << std::endl << std::endl;

	//START DataStream acquisition
	try
	{
		pDataStream->StartAcquisitionContinuous();
		std::cout << "5.1.12   DataStream started " << std::endl;
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	//START CAMERA
	try
	{

		std::cout << "5.1.12   " << pDevice->GetModel() << " started " << std::endl;
		pDevice->GetRemoteNode("AcquisitionStart")->Execute();
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	//CAPTURE 8 IMAGES
	std::cout << " " << std::endl;
	std::cout << "CAPTURE & TRANSFORM 8 IMAGES" << std::endl;
	std::cout << "############################" << std::endl << std::endl;

	BGAPI2::Buffer * pBufferFilled = NULL;
	bo_double fExposureValue = 20000;
	try
	{
		//while (pDataStream->GetIsGrabbing())
		for (int i = 0; i <= 1; i++)
		{
			pBufferFilled = pDataStream->GetFilledBuffer(1000); //timeout 1000 msec
			if (pBufferFilled == NULL)
			{
				std::cout << "Error: Buffer Timeout after 1000 msec" << std::endl << std::endl;
			}
			else if (pBufferFilled->GetIsIncomplete() == true)
			{
				std::cout << "Error: Image is incomplete" << std::endl << std::endl;
				// queue buffer again
				pBufferFilled->QueueBuffer();
			}
			else
			{
				std::cout << " Image " << std::setw(5) << pBufferFilled->GetFrameID() << " received in memory address " << std::hex << pBufferFilled->GetMemPtr() << std::dec << std::endl;

				//create an image object from the filled buffer and convert it
				BGAPI2::Image * pTransformImage = NULL;
				BGAPI2::Image * pImage = imgProcessor->CreateImage((bo_uint)pBufferFilled->GetWidth(), (bo_uint)(int)pBufferFilled->GetHeight(), pBufferFilled->GetPixelFormat(), pBufferFilled->GetMemPtr(), pBufferFilled->GetMemSize());
				std::cout << "  pImage.Pixelformat:             " << pImage->GetPixelformat() << std::endl;
				std::cout << "  pImage.Width:                   " << pImage->GetWidth() << std::endl;
				std::cout << "  pImage.Height:                  " << pImage->GetHeight() << std::endl;
				std::cout << "  pImage.Buffer:                  " << std::hex << pImage->GetBuffer() << std::dec << std::endl;

				//  SET EXPOSURE TIME
				pDevice->GetRemoteNode("ExposureTime")->SetDouble(fExposureValue);
				std::cout << "Set Exposure to: ";
				std::cout << pDevice->GetRemoteNode("ExposureTime")->GetDouble();
				std::cout << " [usec]" << std::endl;

				double fBytesPerPixel = pImage->GetNode("PixelFormatBytes")->GetDouble();
				std::cout << "  Bytes per image:                " << (long)((pImage->GetWidth())*(pImage->GetHeight())*fBytesPerPixel) << std::endl;
				std::cout << "  Bytes per pixel:                " << fBytesPerPixel << std::endl;

				// display first 6 pixel values of first 6 lines of the image
				//========================================================================
				unsigned char* imageBuffer = (unsigned char *)pImage->GetBuffer();
				unsigned long imageBufferAddress = (unsigned long)pImage->GetBuffer();

				std::cout << "  Address" << std::endl;
				// set display for uppercase hex numbers filled with '0'
				std::cout << std::uppercase << std::setfill('0') << std::hex;
				for (int j = 0; j < 6; j++) // first 6 lines
				{
					imageBufferAddress = (unsigned long)&imageBuffer[(int)(pImage->GetWidth()*j*fBytesPerPixel)];
					std::cout << "  " << std::setw(8) << imageBufferAddress << " ";
					for (int i = 0; i <(int)(6 * fBytesPerPixel); i++) // bytes of first 6 pixels 
					{
						std::cout << " " << std::setw(2) << (int)imageBuffer[(int)(pImage->GetWidth()*j*fBytesPerPixel) + i];
					}
					std::cout << "  ..." << std::endl;
				}
				// set display for lowercase dec numbers filled with ' '
				std::cout << std::nouppercase << std::setfill(' ') << std::dec;

				if (std::string(pImage->GetPixelformat()).substr(0, 4) == "Mono") // if pixel format starts with "Mono"
				{
					//transform to Mono8
					pImage->TransformImage("Mono8", &pTransformImage);
					std::cout << " Image " << std::setw(5) << pBufferFilled->GetFrameID() << " transformed to Mono8" << std::endl;
					std::cout << "  pTransformImage.Pixelformat:    " << pTransformImage->GetPixelformat() << std::endl;
					std::cout << "  pTransformImage.Width:          " << pTransformImage->GetWidth() << std::endl;
					std::cout << "  pTransformImage.Height:         " << pTransformImage->GetHeight() << std::endl;
					std::cout << "  pTransformImage.Buffer:         " << std::hex << pTransformImage->GetBuffer() << std::dec << std::endl;
					std::cout << "  Bytes per image:                " << (long)((pTransformImage->GetWidth())*(pTransformImage->GetHeight())*1.0) << std::endl;
					std::cout << "  Bytes per pixel:                " << 1.0 << std::endl;

					unsigned char* transformBuffer = (unsigned char *)pTransformImage->GetBuffer();
					unsigned long transformBufferAddress = (unsigned long)pTransformImage->GetBuffer();

					// display first 6 pixel values of first 6 lines of the transformed image
					//========================================================================
					std::cout << "  Address    Y  Y  Y  Y  Y  Y " << std::endl;

					// set display for uppercase hex numbers filled with '0'
					std::cout << std::uppercase << std::setfill('0') << std::hex;
					for (int j = 0; j < 6; j++) // first 6 lines
					{
						transformBufferAddress = (unsigned long)&transformBuffer[pTransformImage->GetWidth() * 1 * j];
						std::cout << "  " << std::setw(8) << std::setfill('0') << std::hex << transformBufferAddress << " ";
						for (int i = 0; i < 6; i++) // first 6 Pixel with Mono8 (1 Byte per Pixel)
						{
							std::cout << " " << std::setw(2) << (int)transformBuffer[pTransformImage->GetWidth()*j + i]; // value of pixel
						}
						std::cout << " ..." << std::endl;
					}
					// set display for lowercase dec numbers filled with ' '
					std::cout << std::nouppercase << std::setfill(' ') << std::dec;
					std::cout << " " << std::endl;
				}
				else // if color format
				{
					//transform to BGR8
					pImage->TransformImage("BGR8", &pTransformImage);
					std::cout << " Image " << std::setw(5) << pBufferFilled->GetFrameID() << " transformed to BGR8" << std::endl;
					std::cout << "  pTransformImage.Pixelformat:    " << pTransformImage->GetPixelformat() << std::endl;
					std::cout << "  pTransformImage.Width:          " << pTransformImage->GetWidth() << std::endl;
					std::cout << "  pTransformImage.Height:         " << pTransformImage->GetHeight() << std::endl;
					std::cout << "  pTransformImage.Buffer:         " << std::hex << pTransformImage->GetBuffer() << std::dec << std::endl;
					std::cout << "  Bytes per image:                " << (long)((pTransformImage->GetWidth())*(pTransformImage->GetHeight())*3.0) << std::endl;
					std::cout << "  Bytes per pixel:                " << 3.0 << std::endl;


					unsigned char* transformBuffer = (unsigned char *)pTransformImage->GetBuffer();
					unsigned long transformBufferAddress = (unsigned long)pTransformImage->GetBuffer();

					// display first 6 pixel values of first 6 lines of the transformed image
					//========================================================================
					std::cout << "  Address    B  G  R  B  G  R  B  G  R  B  G  R  B  G  R  B  G  R" << std::endl;

					// set display for uppercase hex numbers filled with '0'
					std::cout << std::uppercase << std::setfill('0') << std::hex;
					for (int j = 0; j < 6; j++) // 6 lines
					{
						transformBufferAddress = (unsigned long)&transformBuffer[pTransformImage->GetWidth() * 3 * j];
						std::cout << "  " << std::setw(8) << std::setfill('0') << std::hex << transformBufferAddress << " ";
						for (int i = 0; i < 6; i++) // first 6 Pixel with BGR8 (3 Bytes per Pixel)
						{
							std::cout << " " << std::setw(2) << (int)transformBuffer[pTransformImage->GetWidth() * 3 * j + i * 3 + 0]; // Blue value of pixel
							std::cout << " " << std::setw(2) << (int)transformBuffer[pTransformImage->GetWidth() * 3 * j + i * 3 + 1]; // Green value of pixel
							std::cout << " " << std::setw(2) << (int)transformBuffer[pTransformImage->GetWidth() * 3 * j + i * 3 + 2]; // Red value of pixel
																																	   //std::cout << " ";
						}
						std::cout << " ..." << std::endl;
					}
					// set display for lowercase dec numbers filled with ' '
					std::cout << std::nouppercase << std::setfill(' ') << std::dec;
					std::cout << " " << std::endl;
				}

				/*
				*****************************************************
				Store Image using OPENCV and write image path to DB
				*****************************************************
				*/
				openCvImage = cv::Mat(pTransformImage->GetHeight(), pTransformImage->GetWidth(), CV_8U, (int *)pTransformImage->GetBuffer());
				if (cv::imwrite(tiff_image_path, openCvImage)) {
					cout << "Image successfully stored in directory. Writing image path to DB." << endl;
					ImageDetails(connection, image_index, tiff_image_path);
				}
				else {
					cout << "LOG SEV HIGH: Unable to store image through OpenCV";
				}
				
				if (CreateDirectory(directory_path, NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
					//Write the image to the directory
					if (imwrite(tiff_image_path, image)) {
						
						ImageDetails(connection, image_index, tiff_image_path);
					}
					else {
						cout << "LOG SEV HIGH: OPENCV : Unable to store tiff the image" << endl;
					}
				}
				pImage->Release();
				pTransformImage->Release();
				//delete [] transformBuffer;

				//QUEUE BUFFER AFTER USE
				pBufferFilled->QueueBuffer();
			}
		}
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}
	std::cout << " " << std::endl;


	std::cout << "CAMERA STOP" << std::endl;
	std::cout << "###########" << std::endl << std::endl;

	//STOP CAMERA
	try
	{
		//SEARCH FOR 'AcquisitionAbort'
		if (pDevice->GetRemoteNodeList()->GetNodePresent("AcquisitionAbort"))
		{
			pDevice->GetRemoteNode("AcquisitionAbort")->Execute();
			std::cout << "5.1.12   " << pDevice->GetModel() << " aborted " << std::endl;
		}

		pDevice->GetRemoteNode("AcquisitionStop")->Execute();
		std::cout << "5.1.12   " << pDevice->GetModel() << " stopped " << std::endl;
		std::cout << std::endl;

		BGAPI2::String sExposureNodeName = "";
		if (pDevice->GetRemoteNodeList()->GetNodePresent("ExposureTime")) {
			sExposureNodeName = "ExposureTime";
		}
		else if (pDevice->GetRemoteNodeList()->GetNodePresent("ExposureTimeAbs")) {
			sExposureNodeName = "ExposureTimeAbs";
		}
		std::cout << "         ExposureTime:                   " << std::fixed << std::setprecision(0) << pDevice->GetRemoteNode(sExposureNodeName)->GetDouble() << " [" << pDevice->GetRemoteNode(sExposureNodeName)->GetUnit() << "]" << std::endl;
		if (pDevice->GetTLType() == "GEV")
		{
			if (pDevice->GetRemoteNodeList()->GetNodePresent("DeviceStreamChannelPacketSize"))
				std::cout << "         DeviceStreamChannelPacketSize:  " << pDevice->GetRemoteNode("DeviceStreamChannelPacketSize")->GetInt() << " [bytes]" << std::endl;
			else
				std::cout << "         GevSCPSPacketSize:              " << pDevice->GetRemoteNode("GevSCPSPacketSize")->GetInt() << " [bytes]" << std::endl;
			std::cout << "         GevSCPD (PacketDelay):          " << pDevice->GetRemoteNode("GevSCPD")->GetInt() << " [tics]" << std::endl;
		}
		std::cout << std::endl;
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	//STOP DataStream acquisition
	try
	{
		if (pDataStream->GetTLType() == "GEV")
		{
			//DataStream Statistic
			std::cout << "         DataStream Statistics " << std::endl;
			std::cout << "           DataBlockComplete:              " << pDataStream->GetNodeList()->GetNode("DataBlockComplete")->GetInt() << std::endl;
			std::cout << "           DataBlockInComplete:            " << pDataStream->GetNodeList()->GetNode("DataBlockInComplete")->GetInt() << std::endl;
			std::cout << "           DataBlockMissing:               " << pDataStream->GetNodeList()->GetNode("DataBlockMissing")->GetInt() << std::endl;
			std::cout << "           PacketResendRequestSingle:      " << pDataStream->GetNodeList()->GetNode("PacketResendRequestSingle")->GetInt() << std::endl;
			std::cout << "           PacketResendRequestRange:       " << pDataStream->GetNodeList()->GetNode("PacketResendRequestRange")->GetInt() << std::endl;
			std::cout << "           PacketResendReceive:            " << pDataStream->GetNodeList()->GetNode("PacketResendReceive")->GetInt() << std::endl;
			std::cout << "           DataBlockDroppedBufferUnderrun: " << pDataStream->GetNodeList()->GetNode("DataBlockDroppedBufferUnderrun")->GetInt() << std::endl;
			std::cout << "           Bitrate:                        " << pDataStream->GetNodeList()->GetNode("Bitrate")->GetDouble() << std::endl;
			std::cout << "           Throughput:                     " << pDataStream->GetNodeList()->GetNode("Throughput")->GetDouble() << std::endl;
			std::cout << std::endl;
		}
		if (pDataStream->GetTLType() == "U3V")
		{
			//DataStream Statistic
			std::cout << "         DataStream Statistics " << std::endl;
			std::cout << "           GoodFrames:            " << pDataStream->GetNodeList()->GetNode("GoodFrames")->GetInt() << std::endl;
			std::cout << "           CorruptedFrames:       " << pDataStream->GetNodeList()->GetNode("CorruptedFrames")->GetInt() << std::endl;
			std::cout << "           LostFrames:            " << pDataStream->GetNodeList()->GetNode("LostFrames")->GetInt() << std::endl;
			std::cout << std::endl;
		}

		pDataStream->StopAcquisition();
		std::cout << "5.1.12   DataStream stopped " << std::endl;
		bufferList->DiscardAllBuffers();
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}
	std::cout << std::endl;


	std::cout << "RELEASE" << std::endl;
	std::cout << "#######" << std::endl << std::endl;

	//Release buffers
	std::cout << "5.1.13   Releasing the resources " << std::endl;
	try
	{
		while (bufferList->size() > 0)
		{
			pBuffer = bufferList->begin()->second;
			bufferList->RevokeBuffer(pBuffer);
			delete pBuffer;
		}
		std::cout << "         buffers after revoke:    " << bufferList->size() << std::endl;

		pDataStream->Close();
		pDevice->Close();
		pInterface->Close();
		pSystem->Close();
		BGAPI2::SystemList::ReleaseInstance();
		//RELEASE IMAGE PROCESSOR
		BGAPI2::ImageProcessor::ReleaseInstance();
		std::cout << "         ImageProcessor released" << std::endl << std::endl;
	}
	catch (BGAPI2::Exceptions::IException& ex)
	{
		returncode = 0 == returncode ? 1 : returncode;
		std::cout << "ExceptionType:    " << ex.GetType() << std::endl;
		std::cout << "ErrorDescription: " << ex.GetErrorDescription() << std::endl;
		std::cout << "in function:      " << ex.GetFunctionName() << std::endl;
	}

	std::cout << std::endl;
	std::cout << "End" << std::endl << std::endl;


	/*
	****************************************************************
		Image is cropped, processed, coordinates are identified
	****************************************************************
	*/
	//openCVImage


	/*
	****************************************************************
	coordinates are filtered, sorted as per left and right window
	****************************************************************
	*/

	/*
	****************************************************************
	coordinates are stored
	****************************************************************
	*/

	/*
	****************************************************************
	coordinate differences are calculated
	****************************************************************
	*/

	/*
	****************************************************************
	coordinate distances are calculated
	****************************************************************
	*/

	/*
	****************************************************************
	End mySQL connection
	****************************************************************
	*/
	endMySQLConn(connection);

	std::cout << "Input any number to close the program:";
	int endKey = 0;
	std::cin >> endKey;
	return returncode;
}
