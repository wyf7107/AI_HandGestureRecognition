/*	CS585_Lab2.cpp
*	CS585 Image and Video Computing Fall 2014
*	
*	used for:
*	CS440 Lab: Introduction to OpenCV
*	--------------
*	This program introduces the following concepts:
*		a) Reading a stream of images from a webcamera, and displaying the video
*		b) Skin color detection
*		c) Background differencing
*		d) Visualizing motion history
*	--------------
*/

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//C++ standard libraries
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

//function declarations

/**
Function that returns the maximum of 3 integers
@param a first integer
@param b second integer
@param c third integer
*/
int myMax(int a, int b, int c);

/**
Function that returns the minimum of 3 integers
@param a first integer
@param b second integer
@param c third integer
*/
int myMin(int a, int b, int c);

/**
Function that detects whether a pixel belongs to the skin based on RGB values
@param src The source color image
@param dst The destination grayscale image where skin pixels are colored white and the rest are colored black
*/
void mySkinDetect(Mat& src, Mat& dst);

/**
Function that does frame differencing between the current frame and the previous frame
@param src The current color image
@param prev The previous color image
@param dst The destination grayscale image where pixels are colored white if the corresponding pixel intensities in the current
and previous image are not the same
*/
void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst);

/**
Function that accumulates the frame differences for a certain number of pairs of frames
@param mh Vector of frame difference images
@param dst The destination grayscale image to store the accumulation of the frame difference images
*/
void myMotionEnergy(vector<Mat> mh, Mat& dst);

void condefects(vector<Vec4i> convexityDefectsSet, vector<Point> mycontour, Mat &frame);


int thresh = 128;
int max_thresh = 255;


int main()
{

	//----------------
	//a) Reading a stream of images from a webcamera, and displaying the video
	//----------------
	// For more information on reading and writing video: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html
	// open the video camera no. 0
	VideoCapture cap(0);

	// if not successful, exit program
	if (!cap.isOpened())
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	//create a window called "MyVideoFrame0"
	namedWindow("MyVideo0", WINDOW_AUTOSIZE);
	Mat frame0;

	// read a new frame from video
	bool bSuccess0 = cap.read(frame0);

	//if not successful, break loop
	if (!bSuccess0)
	{
		cout << "Cannot read a frame from video stream" << endl;
	}

	//show the frame in "MyVideo" window
	imshow("MyVideo0", frame0);

	//create a window called "MyVideo"
	namedWindow("MyVideo", WINDOW_AUTOSIZE);
	namedWindow("MyVideoMH", WINDOW_AUTOSIZE);
	namedWindow("Skin", WINDOW_AUTOSIZE);

	vector<Mat> myMotionHistory;
	Mat fMH1, fMH2, fMH3;
	fMH1 = Mat::zeros(frame0.rows, frame0.cols, CV_8UC1);
	fMH2 = fMH1.clone();
	fMH3 = fMH1.clone();
	myMotionHistory.push_back(fMH1);
	myMotionHistory.push_back(fMH2);
	myMotionHistory.push_back(fMH3);


	while (1)
	{
		// read a new frame from video
		Mat frame;
		bool bSuccess = cap.read(frame);

		//if not successful, break loop
		if (!bSuccess)
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		// destination frame
		Mat frameDest;
		frameDest = Mat::zeros(frame.rows, frame.cols, CV_8UC1); //Returns a zero array of same size as src mat, and of type CV_8UC1
		//----------------
		//	b) Skin color detection
		//----------------
		mySkinDetect(frame, frameDest);

		Mat thres_output;
		threshold(frameDest, thres_output, thresh, max_thresh, 0);

		
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours(thres_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		cout << "The number of contours detected is: " << contours.size() << endl;



		imshow("Skin", thres_output);

		int maxsize = 0;
		int maxind = 0;
		Rect boundrec;

		for (int i = 0; i < contours.size(); i++)
		{
			double area = contourArea(contours[i]);
			if (area > maxsize) {
				maxsize = area;
				maxind = i;
				boundrec = boundingRect(contours[i]);
			}
		}

		vector<vector<Point> >hull(contours.size());
		vector<vector<Vec4i>>defects(contours.size());
		vector<vector<int> >inthull(contours.size());

		for (int i = 0; i < contours.size(); i++)
		{
			convexHull(Mat(contours[i]), hull[i], false);
			convexHull(Mat(contours[i]), inthull[i], false);
			if (inthull[i].size() > 3) {
				convexityDefects(contours[i], inthull[i], defects[i]);
			}
		}

		
		rectangle(frame, boundrec, Scalar(0, 255, 0), 1, 8, 0);
		drawContours(frame, hull, maxind, Scalar(0, 0, 255), 2, 8, hierarchy);
		rectangle(frameDest, boundrec, Scalar(255, 255, 255), 1, 8, 0);
		drawContours(frameDest, hull, maxind, Scalar(255, 0, 255), 2, 8, hierarchy);

		condefects(defects[maxind], contours[maxind], frame);

		cout << "The area of the largest contour detected is: " << contourArea(contours[maxind]) << endl;
		cout << "-----------------------------" << endl << endl;

		imshow("RockScissorPaper", frame);
		//----------------
		//	c) Background differencing
		//----------------


		//call myFrameDifferencing function
		myFrameDifferencing(frame0, frame, frameDest);
		imshow("MyVideo", frameDest);
		myMotionHistory.erase(myMotionHistory.begin());
		myMotionHistory.push_back(frameDest);
		Mat myMH = Mat::zeros(frame0.rows, frame0.cols, CV_8UC1);

		//----------------
		//  d) Visualizing motion history
		//----------------

		//call myMotionEnergy function
		myMotionEnergy(myMotionHistory, myMH);


		imshow("MyVideoMH", myMH); //show the frame in "MyVideo" window
		frame0 = frame;
		//wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		if (waitKey(30) == 27)
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}

	}
	cap.release();
	return 0;
}

//Function that returns the maximum of 3 integers
int myMax(int a, int b, int c) {
	int m = a;
	(void)((m < b) && (m = b));
	(void)((m < c) && (m = c));
	return m;
}

//Function that returns the minimum of 3 integers
int myMin(int a, int b, int c) {
	int m = a;
	(void)((m > b) && (m = b));
	(void)((m > c) && (m = c));
	return m;
}

//Function that detects whether a pixel belongs to the skin based on RGB values
void mySkinDetect(Mat& src, Mat& dst) {
	//Surveys of skin color modeling and detection techniques:
	//Vezhnevets, Vladimir, Vassili Sazonov, and Alla Andreeva. "A survey on pixel-based skin color detection techniques." Proc. Graphicon. Vol. 3. 2003.
	//Kakumanu, Praveen, Sokratis Makrogiannis, and Nikolaos Bourbakis. "A survey of skin-color modeling and detection methods." Pattern recognition 40.3 (2007): 1106-1122.
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			//For each pixel, compute the average intensity of the 3 color channels
			Vec3b intensity = src.at<Vec3b>(i, j); //Vec3b is a vector of 3 uchar (unsigned character)
			int B = intensity[0]; int G = intensity[1]; int R = intensity[2];
			if ((R > 95 && G > 40 && B > 20) && (myMax(R, G, B) - myMin(R, G, B) > 15) && (abs(R - G) > 15) && (R > G) && (R > B)){
				dst.at<uchar>(i, j) = 255;
			}
		}
	}
}

//Function that does frame differencing between the current frame and the previous frame
void myFrameDifferencing(Mat& prev, Mat& curr, Mat& dst) {
	//For more information on operation with arrays: http://docs.opencv.org/modules/core/doc/operations_on_arrays.html
	//For more information on how to use background subtraction methods: http://docs.opencv.org/trunk/doc/tutorials/video/background_subtraction/background_subtraction.html
	absdiff(prev, curr, dst);
	Mat gs = dst.clone();
	cvtColor(dst, gs, CV_BGR2GRAY);
	dst = gs > 50;
	Vec3b intensity = dst.at<Vec3b>(100, 100);
}

//Function that accumulates the frame differences for a certain number of pairs of frames
void myMotionEnergy(vector<Mat> mh, Mat& dst) {
	Mat mh0 = mh[0];
	Mat mh1 = mh[1];
	Mat mh2 = mh[2];

	for (int i = 0; i < dst.rows; i++){
		for (int j = 0; j < dst.cols; j++){
			if (mh0.at<uchar>(i, j) == 255 || mh1.at<uchar>(i, j) == 255 || mh2.at<uchar>(i, j) == 255){
				dst.at<uchar>(i, j) = 255;
			}
		}
	}
}

void condefects(vector<Vec4i> Defects, vector<Point> contour, Mat &frame)
{
	Point2f center;
	float x;
	int fingers = 0;

	minEnclosingCircle(contour, center, x);
	circle(frame, center, 10, CV_RGB(0, 0, 255), 2, 8);

	for (int i = 0; i < Defects.size(); i++) {

		Point ptStart(contour[Defects[i].val[0]]);
		double depth = static_cast<double>(Defects[i].val[3]) / 256;

		circle(frame, ptStart, 5, CV_RGB(255, 0, 0), 2, 8);

		if (depth>11 && ptStart.y<center.y) {
			circle(frame, ptStart, 4, CV_RGB(255, 0, 0), 4);
			fingers++;
		}
	}


	if (fingers >1 && fingers <= 3) {
		putText(frame, "Scissor", Point(50, 50), 2, 2, CV_RGB(255, 255, 51), 4, 8);
	}
	else if (fingers <= 1) {
		putText(frame, "Rock", Point(50, 50), 2, 2, CV_RGB(255, 255, 51), 4, 8);
	}
	else if (fingers > 3) {
		putText(frame, "Paper", Point(50, 50), 2, 2, CV_RGB(255, 255, 51), 4, 8);
	}
}