#include"dbconnector.h"



sql::Connection* getMySQLConn()
{
	try {
		sql::Driver *driver;
		sql::Connection *connection;
		/* Create a connection */
		driver = get_driver_instance();
		connection = driver->connect("tcp://127.0.0.1:3306", "geodesy", "vTYV8cZLhf8bpm4bJyWufb4Z2");
		/* Connect to the MySQL geodesy database */
		connection->setSchema("geodesy");
		return connection;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

int getNextImageId(sql::Connection* connection)
{
	int currentID;
	sql::PreparedStatement *prep_stmt;
	sql::ResultSet *res;
	try{
		prep_stmt = connection->prepareStatement("SELECT MAX(imageid) AS currentID FROM image_paths GROUP BY imageid");
		res = prep_stmt->executeQuery();
		while (res->next())
			currentID = res->getInt("currentID");
		delete prep_stmt;
		delete res;
		return currentID;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		delete prep_stmt;
		delete res;
		return 0;
	}
}

void ImageDetails(sql::Connection* connection, int image_index, string image_path) {
	
	try {
		sql::PreparedStatement *prep_stmt;

		prep_stmt = connection->prepareStatement("INSERT INTO image_paths (imageid, image_path_tiff) VALUES (?, ?)");
		prep_stmt->setInt(1, image_index);
		prep_stmt->setString(2, image_path.c_str());
		prep_stmt->executeQuery();

		//while (res->next()) {
		//	cout << "\t... MySQL replies: ";
		//	/* Access column data by alias or column name */
		//	/* Access column fata by numeric offset, 1 is the first column */
		//	cout << endl;
		//	cout << "id: " << res->getInt("idtest") << endl;
		//	cout << "name: " << res->getString("name").c_str() << endl;
		//	cout << "timestamp: " << res->getString("timestamp").c_str() << endl;
		//}

		delete prep_stmt;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void processedImageDetails(sql::Connection * connection, int image_id, string processed_image_path)
{
	try {
		sql::PreparedStatement *prep_stmt;

		prep_stmt = connection->prepareStatement("INSERT INTO processed_images (imageid, processed_image_path) VALUES (?, ?)");
		prep_stmt->setInt(1, image_id);
		prep_stmt->setString(2, processed_image_path.c_str());
		prep_stmt->executeQuery();

		delete prep_stmt;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void updateCoordinateDiff(sql::Connection * connection, int image_id_first, int image_id_second)
{
	int i = 0;
	int coordinate_delta_left[9][2];
	int coordinate_delta_right[9][2];
	sql::PreparedStatement *prep_stmt_first;	// Current Image Index
	sql::PreparedStatement *prep_stmt_second;	// Previous Image Index
	sql::PreparedStatement *prep_stmt_insert;
	sql::ResultSet *res_first;
	sql::ResultSet *res_second;
	try {
		prep_stmt_first = connection->prepareStatement("SELECT \
		ST_X(left_00) AS left_00_x, ST_X(left_01) AS left_01_x, ST_X(left_02) AS left_02_x, \
		ST_X(left_10) AS left_10_x, ST_X(left_11) AS left_11_x, ST_X(left_12) AS left_12_x, \
		ST_X(left_20) AS left_20_x, ST_X(left_21) AS left_21_x, ST_X(left_22) AS left_22_x, \
		ST_Y(left_00) AS left_00_y, ST_Y(left_01) AS left_01_y, ST_Y(left_02) AS left_02_y, \
		ST_Y(left_10) AS left_10_y, ST_Y(left_11) AS left_11_y, ST_Y(left_12) AS left_12_y, \
		ST_Y(left_20) AS left_20_y, ST_Y(left_21) AS left_21_y, ST_Y(left_22) AS left_22_y, \
		ST_X(right_00) AS right_00_x, ST_X(right_01) AS right_01_x, ST_X(right_02) AS right_02_x, \
		ST_X(right_10) AS right_10_x, ST_X(right_11) AS right_11_x, ST_X(right_12) AS right_12_x, \
		ST_X(right_20) AS right_20_x, ST_X(right_21) AS right_21_x, ST_X(right_22) AS right_22_x, \
		ST_Y(right_00) AS right_00_y, ST_Y(right_01) AS right_01_y, ST_Y(right_02) AS right_02_y, \
		ST_Y(right_10) AS right_10_y, ST_Y(right_11) AS right_11_y, ST_Y(right_12) AS right_12_y, \
		ST_Y(right_20) AS right_20_y, ST_Y(right_21) AS right_21_y, ST_Y(right_22) AS right_22_y \
		FROM geodesy.coordinates WHERE imageid = ?");
		prep_stmt_second = connection->prepareStatement("SELECT \
		ST_X(left_00) AS left_00_x, ST_X(left_01) AS left_01_x, ST_X(left_02) AS left_02_x, \
		ST_X(left_10) AS left_10_x, ST_X(left_11) AS left_11_x, ST_X(left_12) AS left_12_x, \
		ST_X(left_20) AS left_20_x, ST_X(left_21) AS left_21_x, ST_X(left_22) AS left_22_x, \
		ST_Y(left_00) AS left_00_y, ST_Y(left_01) AS left_01_y, ST_Y(left_02) AS left_02_y, \
		ST_Y(left_10) AS left_10_y, ST_Y(left_11) AS left_11_y, ST_Y(left_12) AS left_12_y, \
		ST_Y(left_20) AS left_20_y, ST_Y(left_21) AS left_21_y, ST_Y(left_22) AS left_22_y, \
		ST_X(right_00) AS right_00_x, ST_X(right_01) AS right_01_x, ST_X(right_02) AS right_02_x, \
		ST_X(right_10) AS right_10_x, ST_X(right_11) AS right_11_x, ST_X(right_12) AS right_12_x, \
		ST_X(right_20) AS right_20_x, ST_X(right_21) AS right_21_x, ST_X(right_22) AS right_22_x, \
		ST_Y(right_00) AS right_00_y, ST_Y(right_01) AS right_01_y, ST_Y(right_02) AS right_02_y, \
		ST_Y(right_10) AS right_10_y, ST_Y(right_11) AS right_11_y, ST_Y(right_12) AS right_12_y, \
		ST_Y(right_20) AS right_20_y, ST_Y(right_21) AS right_21_y, ST_Y(right_22) AS right_22_y \
		FROM geodesy.coordinates WHERE imageid = ?");
		prep_stmt_first->setInt(1, image_id_first);
		prep_stmt_second->setInt(1, image_id_second);

		res_first = prep_stmt_first->executeQuery();
		res_second = prep_stmt_second->executeQuery();

		while (res_first->next() && res_second->next()) {
			//left window
			coordinate_delta_left[i][0] = res_first->getInt("left_00_x") - res_second->getInt("left_00_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_00_y") - res_second->getInt("left_00_y");
			coordinate_delta_left[i][0] = res_first->getInt("left_01_x") - res_second->getInt("left_01_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_01_y") - res_second->getInt("left_01_y");
			coordinate_delta_left[i][0] = res_first->getInt("left_02_x") - res_second->getInt("left_02_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_02_y") - res_second->getInt("left_02_y");

			coordinate_delta_left[i][0] = res_first->getInt("left_10_x") - res_second->getInt("left_10_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_10_y") - res_second->getInt("left_10_y");
			coordinate_delta_left[i][0] = res_first->getInt("left_11_x") - res_second->getInt("left_11_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_11_y") - res_second->getInt("left_11_y");
			coordinate_delta_left[i][0] = res_first->getInt("left_12_x") - res_second->getInt("left_12_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_12_y") - res_second->getInt("left_12_y");
			
			coordinate_delta_left[i][0] = res_first->getInt("left_20_x") - res_second->getInt("left_20_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_20_y") - res_second->getInt("left_20_y");
			coordinate_delta_left[i][0] = res_first->getInt("left_21_x") - res_second->getInt("left_21_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_21_y") - res_second->getInt("left_21_y");
			coordinate_delta_left[i][0] = res_first->getInt("left_22_x") - res_second->getInt("left_22_x");
			coordinate_delta_left[i++][1] = res_first->getInt("left_22_y") - res_second->getInt("left_22_y");

			//right window
			i = 0;
			coordinate_delta_right[i][0] = res_first->getInt("right_00_x") - res_second->getInt("right_00_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_00_y") - res_second->getInt("right_00_y");
			coordinate_delta_right[i][0] = res_first->getInt("right_01_x") - res_second->getInt("right_01_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_01_y") - res_second->getInt("right_01_y");
			coordinate_delta_right[i][0] = res_first->getInt("right_02_x") - res_second->getInt("right_02_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_02_y") - res_second->getInt("right_02_y");

			coordinate_delta_right[i][0] = res_first->getInt("right_10_x") - res_second->getInt("right_10_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_10_y") - res_second->getInt("right_10_y");
			coordinate_delta_right[i][0] = res_first->getInt("right_11_x") - res_second->getInt("right_11_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_11_y") - res_second->getInt("right_11_y");
			coordinate_delta_right[i][0] = res_first->getInt("right_12_x") - res_second->getInt("right_12_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_12_y") - res_second->getInt("right_12_y");

			coordinate_delta_right[i][0] = res_first->getInt("right_20_x") - res_second->getInt("right_20_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_20_y") - res_second->getInt("right_20_y");
			coordinate_delta_right[i][0] = res_first->getInt("right_21_x") - res_second->getInt("right_21_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_21_y") - res_second->getInt("right_21_y");
			coordinate_delta_right[i][0] = res_first->getInt("right_22_x") - res_second->getInt("right_22_x");
			coordinate_delta_right[i++][1] = res_first->getInt("right_22_y") - res_second->getInt("right_22_y");
		}

		for (i = 0; i < 9; i++) {
			cout << "Left Window: " << i << " - " << coordinate_delta_left[i][0] << " : " << coordinate_delta_left[i][1] << endl;
			cout << "Right Window: " << i << " - " << coordinate_delta_right[i][0] << " : " << coordinate_delta_right[i][1] << endl;
		}

		prep_stmt_insert = connection->prepareStatement("INSERT INTO coordinate_delta (imageid, \
		left_00_x, left_00_y, left_01_x, left_01_y, left_02_x, left_02_y, \
		left_10_x, left_10_y, left_11_x, left_11_y, left_12_x, left_12_y, \
		left_20_x, left_20_y, left_21_x, left_21_y, left_22_x, left_22_y, \
		right_00_x, right_00_y, right_01_x, right_01_y, right_02_x, right_02_y, \
		right_10_x, right_10_y, right_11_x, right_11_y, right_12_x, right_12_y, \
		right_20_x, right_20_y, right_21_x, right_21_y, right_22_x, right_22_y) \
		VALUES (?, \
		?, ?, ?, ?, ?, ?, \
		?, ?, ?, ?, ?, ?, \
		?, ?, ?, ?, ?, ?, \
		?, ?, ?, ?, ?, ?, \
		?, ?, ?, ?, ?, ?, \
		?, ?, ?, ?, ?, ?)");

		prep_stmt_insert->setInt(1, image_id_first);
		//left window
		prep_stmt_insert->setInt(2, coordinate_delta_left[0][0]);
		prep_stmt_insert->setInt(3, coordinate_delta_left[0][1]);
		prep_stmt_insert->setInt(4, coordinate_delta_left[1][0]);
		prep_stmt_insert->setInt(5, coordinate_delta_left[1][1]);
		prep_stmt_insert->setInt(6, coordinate_delta_left[2][0]);
		prep_stmt_insert->setInt(7, coordinate_delta_left[2][1]);
		prep_stmt_insert->setInt(8, coordinate_delta_left[3][0]);
		prep_stmt_insert->setInt(9, coordinate_delta_left[3][1]);
		prep_stmt_insert->setInt(10, coordinate_delta_left[4][0]);
		prep_stmt_insert->setInt(11, coordinate_delta_left[4][1]);
		prep_stmt_insert->setInt(12, coordinate_delta_left[5][0]);
		prep_stmt_insert->setInt(13, coordinate_delta_left[5][1]);
		prep_stmt_insert->setInt(14, coordinate_delta_left[6][0]);
		prep_stmt_insert->setInt(15, coordinate_delta_left[6][1]);
		prep_stmt_insert->setInt(16, coordinate_delta_left[7][0]);
		prep_stmt_insert->setInt(17, coordinate_delta_left[7][1]);
		prep_stmt_insert->setInt(18, coordinate_delta_left[8][0]);
		prep_stmt_insert->setInt(19, coordinate_delta_left[8][1]);
		//right window
		prep_stmt_insert->setInt(20, coordinate_delta_right[0][0]);
		prep_stmt_insert->setInt(21, coordinate_delta_right[0][1]);
		prep_stmt_insert->setInt(22, coordinate_delta_right[1][0]);
		prep_stmt_insert->setInt(23, coordinate_delta_right[1][1]);
		prep_stmt_insert->setInt(24, coordinate_delta_right[2][0]);
		prep_stmt_insert->setInt(25, coordinate_delta_right[2][1]);
		prep_stmt_insert->setInt(26, coordinate_delta_right[3][0]);
		prep_stmt_insert->setInt(27, coordinate_delta_right[3][1]);
		prep_stmt_insert->setInt(28, coordinate_delta_right[4][0]);
		prep_stmt_insert->setInt(29, coordinate_delta_right[4][1]);
		prep_stmt_insert->setInt(30, coordinate_delta_right[5][0]);
		prep_stmt_insert->setInt(31, coordinate_delta_right[5][1]);
		prep_stmt_insert->setInt(32, coordinate_delta_right[6][0]);
		prep_stmt_insert->setInt(33, coordinate_delta_right[6][1]);
		prep_stmt_insert->setInt(34, coordinate_delta_right[7][0]);
		prep_stmt_insert->setInt(35, coordinate_delta_right[7][1]);
		prep_stmt_insert->setInt(36, coordinate_delta_right[8][0]);
		prep_stmt_insert->setInt(37, coordinate_delta_right[8][1]);

		prep_stmt_insert->executeQuery();

		delete prep_stmt_first;
		delete prep_stmt_second;
		delete prep_stmt_insert;
		delete res_first;
		delete res_second;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		delete prep_stmt_first;
		delete prep_stmt_second;
		delete res_first;
		delete res_second;
	}
}

void coordinatesDetails(sql::Connection * connection, int image_id, vector<Vec3i> clean_coordinates)
{
	try {
		sql::PreparedStatement *prep_stmt;

		prep_stmt = connection->prepareStatement("INSERT INTO coordinates(imageid, \
		left_00, left_01, left_02, left_10, left_11, left_12, left_20, left_21, left_22, \
		right_00, right_01, right_02, right_10, right_11, right_12,	right_20, right_21, right_22) \
		VALUES (?, point(?,?) , point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?), point(?,?))");
		prep_stmt->setInt(1, image_id);
		//left window
		//left_00
		prep_stmt->setInt(2, clean_coordinates[0][0]);
		prep_stmt->setInt(3, clean_coordinates[0][1]);
		//left_01
		prep_stmt->setInt(4, clean_coordinates[1][0]);
		prep_stmt->setInt(5, clean_coordinates[1][1]);
		//left_02
		prep_stmt->setInt(6, clean_coordinates[2][0]);
		prep_stmt->setInt(7, clean_coordinates[2][1]);
		//left_10
		prep_stmt->setInt(8, clean_coordinates[3][0]);
		prep_stmt->setInt(9, clean_coordinates[3][1]);
		//left_11
		prep_stmt->setInt(10, clean_coordinates[4][0]);
		prep_stmt->setInt(11, clean_coordinates[4][1]);
		//left_12
		prep_stmt->setInt(12, clean_coordinates[5][0]);
		prep_stmt->setInt(13, clean_coordinates[5][1]);
		//left_20
		prep_stmt->setInt(14, clean_coordinates[6][0]);
		prep_stmt->setInt(15, clean_coordinates[6][1]);
		//left_21
		prep_stmt->setInt(16, clean_coordinates[7][0]);
		prep_stmt->setInt(17, clean_coordinates[7][1]);
		//left_22
		prep_stmt->setInt(18, clean_coordinates[8][0]);
		prep_stmt->setInt(19, clean_coordinates[8][1]);
		//right window
		//right_00
		prep_stmt->setInt(20, clean_coordinates[9][0]);
		prep_stmt->setInt(21, clean_coordinates[9][1]);
		//right_01
		prep_stmt->setInt(22, clean_coordinates[10][0]);
		prep_stmt->setInt(23, clean_coordinates[10][1]);
		//right_02
		prep_stmt->setInt(24, clean_coordinates[11][0]);
		prep_stmt->setInt(25, clean_coordinates[11][1]);
		//right_10
		prep_stmt->setInt(26, clean_coordinates[12][0]);
		prep_stmt->setInt(27, clean_coordinates[12][1]);
		//right_11
		prep_stmt->setInt(28, clean_coordinates[13][0]);
		prep_stmt->setInt(29, clean_coordinates[13][1]);
		//right_12
		prep_stmt->setInt(30, clean_coordinates[14][0]);
		prep_stmt->setInt(31, clean_coordinates[14][1]);
		//right_20
		prep_stmt->setInt(32, clean_coordinates[15][0]);
		prep_stmt->setInt(33, clean_coordinates[15][1]);
		//right_21
		prep_stmt->setInt(34, clean_coordinates[16][0]);
		prep_stmt->setInt(35, clean_coordinates[16][1]);
		//right_22
		prep_stmt->setInt(36, clean_coordinates[17][0]);
		prep_stmt->setInt(37, clean_coordinates[17][1]);

		prep_stmt->executeQuery();

		delete prep_stmt;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void endMySQLConn(sql::Connection * connection)
{
	delete connection;
}

void compareCoordinates() {
	cout << "compare Coordinates!" << endl;
}
