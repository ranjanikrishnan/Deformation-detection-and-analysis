# May 2018
------------------

* Camera setup with lan cable on windows.
* Issues faced--
	* datastream not accessible.
	* camera not fully connected.
* Fixed connectivity issue.
* Connect only to the camera explorer. close the ipconfig tool. Then connects and all the functionalities are available. Also,images were not getting saved which got fixed after fixing connectivity issue.


* Tried image acquisition.
* Image quality was poor. Improved the image quality a little. Tried out all the possible features offered by the camera explorer.


#June 2018
Initial setup for running examples:
* Copy all the Baumer files to the windows system.
* Run the provided baumer SDK.
* If cmake not properly installed from the SDK, then install cmake.
* Install Visual studio.
* Follow the steps mentioned in the following file:
```
C:\Users\Ranjani\Desktop\baumer\Components\Examples\C++\README
```

- Once the build is completed, right click on the project you want to run and click on start debug.
- image buffer is created by the baumer code. find way to take the image buffer and put it in mysql. also find what format the image is saved in.
- windows laptop has no enough space. NOt able to install visual studio. Trying connecting camera in my laptop with windows vm.
- connected camera to my laptop in windows vm.
- image analysis with opencv2 possible using hough circle transform.
- Install opencv2 in the system.
- can detect circles. Find a way to get the coordinates of the circles in the image.
- To detect circles and get the x,y coordinates of circles in the image:
	-run the following:
    ```
	python <program-name.py (test-1.py) > | tee coordinates.txt

	```

-





TO-DO
-----
- Try connecting camera to the VM. Get the images and store in MySQl.
* ~~Setup camera with lan cable on ubuntu.~~
* Read programmers guide.
* Start writing script for automating. The script has to take pictures at least 4 every day at certain times. Then process the images to find a pattern.
* The images should be captured as a time series at regular intervals(say 4 times a day) and stored in a database.
* ~~Find information regd database already in use in the department.Meeting with Ahsan.~~
* Using Opencv library, image analysis is to be done.
* ~~The task is to identify the image-coordinates of the circles in the picture.~~
* ~~So for each circle detected you should save the x,y coordinate of the picture.~~
* Then from the coordinate differences between two successive pictures you should compute the coordinate change, i.e. the displacement vector from frame 2 – frame 1.
* image from camera--calibration.
* draw and chcek the coordinates. uncertainty to be checked.
* along with camera, there will be a light. So, light has to be on only when image capture mode is on, else light should stay off.

### Today

~~Check out the output of the following fuctions: GetPixelFormat, GetImageOffset, GetWidth, GetHeight, GetMemSize~~

- deamon to check if images are being captured. else send alert.
- 