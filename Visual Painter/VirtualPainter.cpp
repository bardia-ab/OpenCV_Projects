#include <iostream>
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void tuneColor(cv::VideoCapture cap);
cv::Point getMarkerTip(cv::Mat imgDil, cv::Mat img);
void getColorPoints(std::vector<std::vector<int>> detectedColors, cv::Mat img);
void drawOnCanvas(std::vector<cv::Scalar> drawingColors, cv::Mat img);

std::vector<std::vector<int>> markerTips;

int main() {
	//std::string url = "http://192.168.1.244:8080/video";
	cv::VideoCapture cap(1);
	cv::Mat img;

	std::vector<std::vector<int>> detectedColors = {
		{0, 91, 147, 179, 255, 255},	//red
		{109, 128, 61, 156, 255, 166},	//blue
		{0, 160, 0, 102, 255, 255}		//green
	};

	std::vector<cv::Scalar> drawingColors = {
		cv::Scalar(0, 0, 255),
		cv::Scalar(255, 0, 0),
		cv::Scalar(0, 255, 0)
	};

	//tuneColor(cap);

	cv::namedWindow("WebCam", cv::WINDOW_FREERATIO);

	while (1) {
		cap.read(img);
		cv::flip(img, img, 1);
		
		getColorPoints(detectedColors, img);
		drawOnCanvas(drawingColors, img);
		
		
		cv::imshow("WebCam", img);
		//cv::imshow("WebCam", mask);
		cv::waitKey(1);
	}

	return 0;
}

void tuneColor(cv::VideoCapture cap) {
	cv::Mat img, imgHSV, mask;

	// create a window
	cv::namedWindow("Trackbar", cv::WINDOW_FREERATIO);

	// create trackbars
	int hmin = 0, smin = 0, vmin = 0;
	int hmax = 179, smax = 255, vmax = 255;

	cv::createTrackbar("Hue Min", "Trackbar", &hmin, 179);
	cv::createTrackbar("Saturate Min", "Trackbar", &smin, 255);
	cv::createTrackbar("Value Min", "Trackbar", &vmin, 255);

	cv::createTrackbar("Hue Max", "Trackbar", &hmax, 179);
	cv::createTrackbar("Saturate Max", "Trackbar", &smax, 255);
	cv::createTrackbar("Value Max", "Trackbar", &vmax, 255);

	while (1) {
		cap.read(img);

		// conver BGR to HSV
		cv::cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

		cv::Scalar lower(hmin, smin, vmin);
		cv::Scalar upper(hmax, vmax, smax);

		cv::inRange(imgHSV, lower, upper, mask);
		std::cout << hmin << ", " << smin << ", " << vmin << ", " << hmax << ", " << smax << ", " << vmax << std::endl;

		cv::imshow("Trackbar", mask);
		cv::waitKey(1);
	}

}

cv::Point getMarkerTip(cv::Mat imgDil, cv::Mat img) {
	cv::Point markerTip(0, 0);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(imgDil, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>> conPoly(contours.size());
	std::vector<cv::Rect> boundRect(contours.size());

	for (int i = 0; i < contours.size(); i++) {
		if (cv::contourArea(contours[i]) > 1000) {
			float peri = cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			//cv::drawContours(img, contours, -1, cv::Scalar(255, 0, 255), 2);
			boundRect[i] = cv::boundingRect(conPoly[i]);
			//cv::rectangle(img, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 255, 0), 4);
			markerTip.x = boundRect[i].x + boundRect[i].width / 2;
			markerTip.y = boundRect[i].y;
		}
	}

	return markerTip;
}

 void getColorPoints(std::vector<std::vector<int>> detectedColors, cv::Mat img) {
	cv::Mat imgHSV, mask;
	int hmin = 0, smin = 0, vmin = 0;
	int hmax = 179, smax = 255, vmax = 255;
	cv::cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

	for (int i = 0; i < detectedColors.size(); i++) {
		cv::Scalar lower(detectedColors[i][0], detectedColors[i][1], detectedColors[i][2]);
		cv::Scalar upper(detectedColors[i][3], detectedColors[i][4], detectedColors[i][5]);
		cv::inRange(imgHSV, lower, upper, mask);

		cv::Point markerTip = getMarkerTip(mask, img);
		if (markerTip.x != 0 && markerTip.y != 0)
			markerTips.push_back({ markerTip.x, markerTip.y, i });
	}

	//return markerTips;
}

void drawOnCanvas(std::vector<cv::Scalar> drawingColors, cv::Mat img) {
	for (int i = 0; i < markerTips.size(); i++)
		cv::circle(img, cv::Point(markerTips[i][0], markerTips[i][1]), 10, drawingColors[markerTips[i][2]], cv::FILLED);
}
