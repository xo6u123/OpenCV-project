#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include<map>
#include<cmath>
#include<fstream>

using namespace std;
using namespace cv;


bool isFirstFrame = true;
bool drawBox = false;
bool isChooseObj = false;
Rect box;//tracking object
Rect roi;//
float w = 0.4;//
map<string, Point2f> track;
map<string, float> path;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;


static void mouse_callback(int event, int x, int y, int, void*)
{
    switch (event)
    {
    case EVENT_MOUSEMOVE:
        if (drawBox)
        {
            box.width = x - box.x;
            box.height = y - box.y;
        }
        break;
    case EVENT_LBUTTONDOWN:
        drawBox = true;
        box = Rect(x, y, 0, 0);
        break;
    case EVENT_LBUTTONUP:
        drawBox = false;
        if (box.width < 0)
        {
            box.x += box.width;
            box.width *= -1;
        }
        if (box.height < 0)
        {
            box.y += box.height;
            box.height *= -1;
        }
        isChooseObj = true;
        break;
    }
}


int getThresholeValue(cv::Mat& image)
{
    assert(image.channels() == 1);
    int width = image.cols;
    int height = image.rows;
    int x = 0, y = 0;
    int pixelCount[256] = { 0 };
    float pixelPro[256] = { 0 };
    int i, j, pixelSum = width * height, threshold = 0;
    uchar* data = image.ptr<uchar>(0);

    for (i = y; i < height; i++)
    {
        for (j = x; j < width; j++)
        {
            pixelCount[data[i * image.step + j]]++;
        }
    }

    for (i = 0; i < 256; i++)
    {
        pixelPro[i] = (float)(pixelCount[i]) / (float)(pixelSum);
    }

    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    for (i = 0; i < 256; i++)
    {
        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (j = 0; j < 256; j++)
        {
            if (j <= i)

            {
                w0 += pixelPro[j];
                u0tmp += j * pixelPro[j];
            }
            else
            {

                w1 += pixelPro[j];
                u1tmp += j * pixelPro[j];
            }
        }
        u0 = u0tmp / w0;
        u1 = u1tmp / w1;
        u = u0tmp + u1tmp;

        deltaTmp = w0 * (u0 - u) * (u0 - u) + w1 * (u1 - u) * (u1 - u);
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = i;
        }
    }


    return threshold;
}


Point objStatPredict(Point curPoint, Point prePoint, Point prePrePoint)
{
    Point nextMassCenter;
    nextMassCenter.x = (4 * curPoint.x + prePrePoint.x - 2 * prePoint.x) / 3;
    nextMassCenter.y = (4 * curPoint.y + prePrePoint.y - 2 * prePoint.y) / 3;

    return nextMassCenter;
}
/**********************************************************
 *
 * input:
 * argc-----current mass center point
 * argv-----previous mass center point
 *
 **********************************************************/



std::string rgb2hex(int r, int g, int b, bool with_head)
{
    std::stringstream ss;
    if (with_head)
        ss << "#";
    ss << setiosflags(ios::uppercase) << std::hex << (r << 16 | g << 8 | b);
    return ss.str();
}

double calculateDistance(Point2f x, Point2f y) {
    return sqrt(pow(x.x - y.x, 2) + pow(x.y - y.y, 2));
}


int z(char x, char y) {
    char k[2];
    k[0] = x;
    k[1] = y;
    int h = 0;
    for (int i = 0; i < 2; i++) {
        if ('0' <= k[i] && k[i] <= '9') {
            h += (k[i] - 48) * pow(16, 1 - i);
        }
        else if ('A' <= k[i] && k[i] <= 'F') {
            h += (k[i] - 55) * pow(16, 1 - i);
        }
        else if ('a' <= k[i] && k[i] <= 'f') {
            h += (k[i] - 87) * pow(16, 1 - i);
        }
        else {
            return -1;
        }
    }
    return h;
}


int main(int argc, char* argv[])
{
    Mat pFrame;//read frame from video
    VideoCapture pCapture;
    pCapture.open(argv[1]);
    if (!pCapture.isOpened())
    {
        cout << "open video error" << endl;
        return -1;
    }

    //namedWindow("centroid tracking");
    //moveWindow("centroid tracking", 100, 100);
    //pCapture.read(pFrame);//read a frame from video
    /*
    if (isFirstFrame)
    {

        //remind people to choose obj to tracking
        putText(pFrame, "choose obj to tracking", Point(20, 20), \
            FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
        //Register mouse callback to draw the bounding box
        setMouseCallback("centroid tracking", mouse_callback, NULL);
        while (!isChooseObj)
        {
            imshow("centroid tracking", pFrame);
            if (waitKey(33) == 'q')
            {
                return 1;
            }
        }
        cout << "choosed tracking object" << endl;
        roi = box;
        //Remove mouse_callback
        setMouseCallback("centroid tracking", NULL, NULL);
    }*/

    Mat grayFrame;
    Mat thresholdFrame;
    Mat roiFrame;
    int segValue;//threshold value return by OSTU
    int K = 0;
    while (pCapture.read(pFrame) && !pFrame.empty())
    {
        //pCapture.read(pFrame);
        K++;
        if (K % 2 == 0)
            continue;
        box = Rect(0, 0, pFrame.cols, pFrame.rows);
        roi = box;
        roiFrame = pFrame;
        //absdiff(pFrame,bkground,roiFrame);
        cvtColor(roiFrame, grayFrame, COLOR_BGR2GRAY);
        blur(grayFrame, grayFrame, Size(5, 5));
        //segValue = getThresholeValue(grayFrame);
        threshold(grayFrame, thresholdFrame, 130, 255, THRESH_BINARY);

        //Mat element = getStructuringElement(MORPH_RECT,Size(7,7),Point(-1,-1));
        //erode(thresholdFrame,thresholdFrame,element);
        /// Find outer contours
        findContours(thresholdFrame, contours, hierarchy,
            RETR_LIST, CHAIN_APPROX_NONE);
        int min_size = 1000;
        //int max_size = 10000;
        std::vector<std::vector<cv::Point>>::iterator itc = contours.begin();
        while (itc != contours.end())
        {
            if (contourArea(*itc) < min_size)
            {
                itc = contours.erase(itc);
            }
            else
            {
                ++itc;
                //cout << contourArea(*itc) << endl;
            }
        }

        //find the max area index
        float x, y, z;
        bool click = true;
        string s;
        for (int k = 0; k < contours.size(); ++k)
        {
            Rect boundRect[1];
            boundRect[0] = boundingRect(contours[k]);

            ///Get the moments
            vector<Moments> mu(1);
            mu[0] = moments(contours[k], true);
            /// Get the mass centers:
            vector<Point2f> mc(1);

            mc[0] = Point2f(mu[0].m10 / mu[0].m00 + roi.x, mu[0].m01 / mu[0].m00 + roi.y);
            x = pFrame.at<Vec3b>(mc[0])[2];
            y = pFrame.at<Vec3b>(mc[0])[1];
            z = pFrame.at<Vec3b>(mc[0])[0];
            s = rgb2hex(x, y, z, true);
            putText(pFrame, s, mc[0], \
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));
            //if (mc[0].y < 420)
            circle(pFrame, mc[0], 4, Scalar(0, 255, 0), -1, 8, 0);
            mc[0] = Point2f(mu[0].m10 / mu[0].m00, boundRect[0].tl().y + roi.y + boundRect[0].height);
            if (mc[0].y < pFrame.rows)
                if (track.find(s) == track.end()) {
                    track.insert(pair<string, Point2f>(s, mc[0]));
                    path.insert(pair<string, float>(s, 0));
                }
                else {


                    //if(calculateDistance(track[s], mc[0])>50)
                    path[s] += calculateDistance(track[s], mc[0]);
                    if (calculateDistance(track[s], mc[0]) > 125) {

                        //track.insert(pair<string, Point2f>(s, mc[0]));
                        path[s] = 0;
                    }
                    track[s] = mc[0];


                }
            /// Draw contours
            //draw obj contour
            //drawContours(pFrame,contours, k,Scalar(0,0,255), \
                         2, 8, hierarchy, 0, Point() );
            //draw mass center

            //mc[0] = Point2f(mu[0].m10 / mu[0].m00, boundRect[0].tl().y + roi.y+ boundRect[0].height);
            cv::circle(pFrame, mc[0], 1, Scalar(0, 255, 0), -1, 8, 0);

            //calculates and returns the minimal up-right bounding
            //rectangle for the contours[maxAreaIdx] set.
            mc[0] = Point2f(mu[0].m10 / mu[0].m00 + roi.x, mu[0].m01 / mu[0].m00 + roi.y);

            rectangle(pFrame, Rect(boundRect[0].tl().x + roi.x, \
                boundRect[0].tl().y + roi.y, boundRect[0].width, \
                boundRect[0].height), Scalar(0, 0, 255), 2, 8, 0);
        }


        //isFirstFrame = false;
        //imshow("centroid tracking", pFrame);
        //imshow("centroid tracking",thresholdFrame);
       // cv::waitKey(33);
    }
    //sort(path.begin(), path.end());
    int c1, c2;
    set<string> del;
    string delword = "1";
    for (auto& i : path) {
        int r, g, b, r1, g1, b1;
        //cout << i.first;
        r = z(i.first[1], i.first[2]);
        g = z(i.first[3], i.first[4]);
        b = z(i.first[5], i.first[6]);
        c1 = r + g + b;
        for (auto& j : path) {
            if (i.second >= 0 && j.second >= 0)
                if (i != j) {
                    r1 = z(j.first[1], j.first[2]);
                    g1 = z(j.first[3], j.first[4]);
                    b1 = z(j.first[5], j.first[6]);
                    c2 = r + g + b;
                    if ((max(r, r1) - min(r, r1)) + (max(g, g1) - min(g, g1)) + (max(b, b1) - min(b, b1)) <= 3) {
                        //path[i.first] += path[j.first];
                        //cout << unsigned int(r - r1) << "¡@" << unsigned int(g - g1) << " " << unsigned int(b - b1) << endl;
                        if (path[i.first] > path[j.first])
                            path[j.first] = -1;
                        else
                            path[i.first] = -1;


                    }
                }

        }
    }
    ofstream ofs;
    ofs.open(argv[2], ios::out);
    for (const auto& s : path) {
        if (s.second > 0)
            ofs << s.first << ' ' << fixed << setprecision(2) << s.second << endl;

    }
    //ofs << endl;
    ofs.close();
    return 0;
}