#include<opencv2/opencv.hpp>
#include<math.h>
#include<iostream>
#include<string>
#include<fstream>
#include <vector>

using namespace std;
using namespace cv;


void drawMyContours(string winName, Mat& image, std::vector<std::vector<cv::Point>> contours, bool draw_on_blank, Mat& temp)
{
	if (draw_on_blank)
	{
		temp = cv::Mat(image.size(), CV_8U, cv::Scalar(255));
		cv::drawContours(temp, contours, -1, 0, 1);
	}
	else
	{
		temp = cv::Mat(image.size(), CV_8U, cv::Scalar(255));
		cv::drawContours(temp, contours, -1, 0, -1);
	}

	//cv::waitKey();
}

int main(int argc, char* argv[]) {

	Mat mSrc = imread(argv[1]);
	Mat gray, binary, element, output;
	cvtColor(mSrc, gray, COLOR_BGR2GRAY);
	resize(gray, output, Size(554, 738));
	//imshow("gray", output);

	GaussianBlur(gray, gray, Size(3, 3), 1);
	threshold(gray, binary, 80, 255, cv::THRESH_BINARY);
	resize(binary, output, Size(554, 738));
	//imshow("binary", output);

	element = getStructuringElement(MORPH_RECT, Size(1, 1));
	morphologyEx(binary, binary, cv::MORPH_CLOSE, element);
	resize(binary, output, Size(554, 738));
	//cv::imshow("morphology", output);



	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	Mat temp;
	cv::findContours(binary, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	drawMyContours("contours", mSrc, contours, true, temp);
	int min_size = 40;
	int max_size = 100;

	std::vector<std::vector<cv::Point>>::iterator itc = contours.begin();
	std::vector<cv::Vec4i>::iterator itc_hierarchy = hierarchy.begin();
	itc = contours.begin();
	itc_hierarchy = hierarchy.begin();
	while (itc != contours.end())
	{
		if (itc->size() < min_size || itc->size() > max_size)
		{
			itc = contours.erase(itc);
			itc_hierarchy = hierarchy.erase(itc_hierarchy);
		}
		else
		{
			++itc;
			++itc_hierarchy;
		}
	}
	drawMyContours("contours after length filtering", mSrc, contours, true, temp);
	//cout << contours[1] << endl;
	resize(temp, output, Size(554, 738));
	//cv::imshow("1", output);
	//cv::waitKey();
	/// ///////////////////////////////////////////

	Point2d tmp[4]{};
	int min[4]{};
	for (int i = 0; i < 4; i++) {
		min[i] = 1447 + 1108;
	}
	min[1] = 0;
	min[3] = 0;
	Mat temp1;
	//cout << temp;

	temp1 = cv::Mat(mSrc.size(), CV_8U, cv::Scalar(255));

	for (int i = 0; i < temp.rows; i++) {
		for (int j = 0; j < temp.cols; j++) {
			if (temp.at<bool>(i, j) != 255) {

				if ((i + j) < min[0]) {
					min[0] = (i + j);
					tmp[0] = Point2d(j, i);
					temp1.at<bool>(i, j) = temp.at<bool>(i, j);
				}
				if (j - i > min[1]) {
					min[1] = j - i;
					tmp[1] = Point2d(j, i);
					temp1.at<bool>(i, j) = temp.at<bool>(i, j);
				}
				if (j - i < min[2]) {
					min[2] = j - i;
					tmp[2] = Point2d(j, i);
					temp1.at<bool>(i, j) = temp.at<bool>(i, j);
				}
				if (j + i > min[3]) {
					min[3] = j + i;
					tmp[3] = Point2d(j, i);
					temp1.at<bool>(i, j) = temp.at<bool>(i, j);
				}

			}


		}
	}

	resize(temp1, output, Size(554, 738));
	//imshow("2", output);
	vector<Point2f> src_corners(4);
	src_corners[0] = tmp[0];
	src_corners[1] = tmp[1];
	src_corners[2] = tmp[2];
	src_corners[3] = tmp[3];
	vector<Point2f> dst_corners(4);
	dst_corners[0] = Point(0, 0);
	dst_corners[1] = Point(mSrc.cols - 1, 0);
	dst_corners[2] = Point(0, mSrc.rows - 1);
	dst_corners[3] = Point(mSrc.cols - 1, mSrc.rows - 1);

	Mat result_images = Mat::zeros(295.5 * 5, 221 * 5, CV_8UC3);
	Mat warpmatrix = getPerspectiveTransform(src_corners, dst_corners);
	warpPerspective(mSrc, result_images, warpmatrix, result_images.size());
	resize(result_images, output, Size(554, 738));
	//imshow("final result", output);
	Rect rect(220, 525, 800, 800);
	Rect rect2(80, 525, 60, 800);
	Mat image_ori = result_images(rect);
	Mat image_ori2 = result_images(rect2);
	Mat gray2, binary2, binary3;
	//imshow("Â^¨ú", image_ori);
	//imshow("Â^¨ú2", image_ori);
	cvtColor(image_ori, gray, COLOR_BGR2GRAY);
	cvtColor(image_ori2, gray2, COLOR_BGR2GRAY);
	//imshow("gray2", gray2);

	//GaussianBlur(gray, gray, Size(1, 1), 10);
	//GaussianBlur(gray2, gray2, Size(3, 3), 1);
	threshold(gray, binary, 120, 255, cv::THRESH_BINARY);
	threshold(gray, binary3, 130, 255, cv::THRESH_BINARY);
	threshold(gray2, binary2, 110, 255, cv::THRESH_BINARY);
	//imshow("binary", binary);


	///////////////1111111111111111111111111111
	Mat temp3;
	findContours(binary3, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	drawMyContours("contours", binary3, contours, false, temp3);
	//cv::imshow("tmp", temp3);
	/////////////////////////////11111111111111111111


	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(binary, binary, cv::MORPH_CLOSE, element);
	//morphologyEx(binary2, binary2, cv::MORPH_CLOSE, element);
	//cv::imshow("morphology2", binary);

	Mat final, final2;
	findContours(binary, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	drawMyContours("contours", image_ori, contours, true, temp);
	//cv::imshow("temp", temp);
	min_size = 110;
	max_size = 300;

	itc = contours.begin();
	itc_hierarchy = hierarchy.begin();

	while (itc != contours.end())
	{
		if (contourArea(*itc) < min_size || contourArea(*itc) > max_size)
		{
			itc = contours.erase(itc);
			itc_hierarchy = hierarchy.erase(itc_hierarchy);
		}
		else
		{
			++itc;
			++itc_hierarchy;
		}
	}
	drawMyContours("contours after length filtering", image_ori, contours, false, final);

	//cv::imshow("3", final);
	//cv::imshow("2", temp);

	findContours(binary2, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	drawMyContours("contours", image_ori2, contours, true, final2);
	itc = contours.begin();
	itc_hierarchy = hierarchy.begin();
	min_size = 100;
	max_size = 300;
	while (itc != contours.end())
	{
		if (contourArea(*itc) < min_size || contourArea(*itc) > max_size)
		{
			itc = contours.erase(itc);
			itc_hierarchy = hierarchy.erase(itc_hierarchy);
		}
		else
		{
			++itc;
			++itc_hierarchy;
		}
	}
	drawMyContours("contours after length filtering", image_ori2, contours, true, final2);


	resize(final2, output, Size(40, 600));
	//cv::imshow("4", output);

	//cout << final2.size();
	int N = 0;
	int N2 = 0;
	int Qy[30]{};
	int T[2]{};
	for (int i = 0; i < final2.rows; i++) {

		if (final2.at<bool>(i, 30) != 255) {
			if (N > 0) {
				T[N] = i;
				N = 0;
				Qy[N2] = (T[0] + T[1] + 2) / 2;
				N2++;
			}
			else {
				T[N] = i;
				N = 1;
			}
			//cout << "(" << 30 << "," << i << ")" << endl;

		}

	}
	for (int i = 0; i < 24; i++) {
		//cout << Qy[i] << endl;
		//line(temp3, Point(0,Qy[i]), Point(705,Qy[i]), 233, 2);
	}
	//imshow("line", temp3);
	int Qx[15]{};
	N2 = 0;
	//cout << endl;
	for (int i = 0; i < temp3.cols; i++) {

		if (temp3.at<bool>(Qy[19], i) == 255 && N2 == 0) {
			Qx[N2] = i + 10;
			//i = i + 25;
			//cout << Qx[N2] << endl;
			N2++;
		}

	}
	if (Qx[N2 - 1] > 65)
		Qx[N2 - 1] = 53;
	for (int i = 0; i < 12; i++) {
		Qx[N2] = Qx[N2 - 1] + 61;
		N2++;
	}


	int F[24];
	N2 = 0;
	char T2;
	ofstream ofs;
	ofs.open(argv[2], ios::out);
	for (int i = 0; i < 24; i++) {
		N2 = 0;
		for (int j = 0; j < 12; j++) {

			if (final.at<bool>(Qy[i], Qx[j]) != 255) {
				if (j > 8) {
					if (j == 10) {
						T2 = 'A';
					}
					else if (j == 11) {
						T2 = 'B';
					}
					else if (j == 9) {
						T2 = '0';
					}
				}
				else {
					T2 = '0' + j + 1;
				}
				N2++;
			}
		}
		if (N2 == 0)
			ofs << 'X';
		else if (N2 == 1)
			ofs << T2;
		else
			ofs << 'M';
	}
	for (int i = 0; i < 12; i++) {
		line(temp3, Point(Qx[i], 0), Point(Qx[i], 800), 243, 2);
	}
	for (int i = 0; i < 24; i++) {
		line(temp3, Point(0, Qy[i]), Point(800, Qy[i]), 233, 2);
	}
	//imshow("line", temp3);
	ofs << endl;
	ofs.close();
	//cv::waitKey();
	return 0;

}