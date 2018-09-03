#ifndef DBCONNECTOR_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define DBCONNECTOR_H

#include <iostream>
#include <stdio.h>
#include <string>
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp> 
#include <opencv2\video.hpp>
#include <opencv2\imgcodecs.hpp>



using namespace std;
using namespace sql;
using namespace cv;

// connect to DB
sql::Connection* getMySQLConn();
// fetch the next valid imageId for storing image
int getNextImageId(sql::Connection* connection);

// store time_stamp, image path in DB along with a surrogate key called imageid
void ImageDetails(sql::Connection* connection, int image_index, string image_path);

//store processed image in DB with foreign key imageid
void processedImageDetails(sql::Connection* connection, int image_id, string processed_image_path);

//void compareCoordinates(int id_one, int id_two);

// we find the difference between the current and previous coordinates. 
void updateCoordinateDiff(sql::Connection* connection, int image_id_first, int image_id_second);

//void sortCoordinates();

void coordinatesDetails(sql::Connection* connection, int image_id, vector<Vec3i> coordinates);

void endMySQLConn(sql::Connection *conn);
// 
// get previous coordinates
// get coordinates for a date

#endif
