#include <iostream>
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

cv::Mat preProcess(cv::Mat);
std::vector<cv::Point> getBoarder(cv::Mat, cv::Mat);
void drawPoints(cv::Mat, std::vector<cv::Point>, cv::Scalar);
std::vector<cv::Point> reorderPoints(std::vector<cv::Point>);
cv::Mat getWarp(cv::Mat, std::vector<cv::Point>, float, float);

int main() {
	cv::Mat img = cv::imread("Resources/paper.jpg");
	//cv::resize(img, img, cv::Size(640, 480));

	// PreProcessing
	cv::Mat imgProcessed = preProcess(img);

	// Get Boarders
	std::vector<cv::Point> boarder = getBoarder(imgProcessed, img);

	// Draw Points
	//drawPoints(img, boarder, cv::Scalar(0, 0, 255));

	// Reorder Boarder Points
	boarder = reorderPoints(boarder);
	//drawPoints(img, boarder, cv::Scalar(0, 255, 0));

	// Warp
	float width = 420, height = 596;
	img = getWarp(img, boarder, width, height);

	// Crop
	int cropValue = 5;
	cv::Rect roi(cropValue, cropValue, width - (2 * cropValue), height - (2 * cropValue));
	img = img(roi);
	
	cv::namedWindow("Image");
	cv::imshow("Image", img);
	//cv::namedWindow("Processed Image", cv::WINDOW_FREERATIO);
	//cv::imshow("Processed Image", imgProcessed);
	cv::waitKey(0);

	return 0;
}

cv::Mat preProcess(cv::Mat img) {
	cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(img, img, cv::Size(3, 3), 3, 0);
	cv::Canny(img, img, 25, 75);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::dilate(img, img, kernel);

	return img;
}

std::vector<cv::Point> getBoarder(cv::Mat imgProcessed, cv::Mat img) {
	std::vector<cv::Point> boarder;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(imgProcessed, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>> conPoly(contours.size());
	int maxArea = 0;
	int maxIndex = 0;

	for (int i = 0; i < contours.size(); i++) {
		int area = cv::contourArea(contours[i]);
		float peri = cv::arcLength(contours[i], true);
		cv::approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

		if (area > maxArea && conPoly[i].size() == 4) {
			maxArea = area;
			maxIndex = i;			
		}

	}

	//cv::drawContours(img, contours, maxIndex, cv::Scalar(255, 0, 255), 4);
	boarder = {{conPoly[maxIndex][0]}, {conPoly[maxIndex][1]}, {conPoly[maxIndex][2]}, {conPoly[maxIndex][3]} };

	return boarder;
}

void drawPoints(cv::Mat img, std::vector<cv::Point> boarder,cv::Scalar color) {
	for (int i = 0; i < boarder.size(); i++) {
		cv::circle(img, boarder[i], 10, color, cv::FILLED);
		cv::putText(img, std::to_string(i), boarder[i], cv::FONT_HERSHEY_PLAIN, 5, color, 5);
	}
}

std::vector<cv::Point> reorderPoints(std::vector<cv::Point> boarder) {
	std::vector<cv::Point> newBoarder;
	std::vector<int> sum, sub;

	for (int i = 0; i < boarder.size(); i++) {
		sum.push_back(boarder[i].x + boarder[i].y);
		sub.push_back(boarder[i].x - boarder[i].y);
	}

	newBoarder.push_back(boarder[std::min_element(sum.begin(), sum.end()) - sum.begin()]); // top left
	newBoarder.push_back(boarder[std::max_element(sub.begin(), sub.end()) - sub.begin()]); // top right
	newBoarder.push_back(boarder[std::min_element(sub.begin(), sub.end()) - sub.begin()]); // bottom left
	newBoarder.push_back(boarder[std::max_element(sum.begin(), sum.end()) - sum.begin()]); // bottom right

	return newBoarder;
}

cv::Mat getWarp(cv::Mat img, std::vector<cv::Point> boarder, float width, float height) {
	cv::Mat imgWarp;
	cv::Point2f src[4] = { boarder[0], boarder[1], boarder[2], boarder[3] };
	cv::Point2f dest[4] = { {0, 0}, {width, 0}, {0, height}, {width, height} };

	cv::Mat matrix = cv::getPerspectiveTransform(src, dest);
	cv::warpPerspective(img, imgWarp, matrix, cv::Size(width, height));

	return imgWarp;
}