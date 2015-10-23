#ifndef __BR_H__
#define __BR_H__
#include <opencv.hpp>

#define COLOR_RED Scalar(0,0,255)
#define COLOR_LIGHT_RED Scalar(102,102,255)
#define COLOR_DARK_RED Scalar(0,0,204)
#define COLOR_YELLOW Scalar(0,255,255)
#define COLOR_LIGHT_YELLOW Scalar(0,0,255)
#define COLOR_DARK_YELLOW Scalar(0,0,255)
#define COLOR_GREEN Scalar(0,255,0)
#define COLOR_LIGHT_GREEN Scalar(0,255,204)
#define COLOR_DARK_GREEN Scalar(0,153,0)
#define COLOR_ORANGLE Scalar(0,102,255)
#define COLOR_BLUE Scalar(255,102,0)
#define COLOR_LIGHT_BLUE Scalar(255,153,0)
#define COLOR_DARK_BLUE Scalar(204,51,0)
#define COLOR_BLACK Scalar(0,0,0)
#define COLOR_WHITE Scalar(255,255,255)

using namespace cv;
using namespace std;
namespace br{
	class BR
	{
	public:
		BR(Size size);
		void BR_Correction(Mat src, Mat &dst, bool flag);
		int verifySizes(const Rect &rect);
		int verifySizes(const RotatedRect &rect);
		int SecverifySizes(const Rect &rect);
		int SecverifySizes(const RotatedRect &rect);
		int BR::Sobel_Thresh(const Mat &src, Mat &sobel);
		int preprocess(const Mat &src, Mat &gray);
		void verifyContour(Mat &src, vector<vector<Point>> &contour, vector<Vec4i> &hierarchy);
		int digitOne(RotatedRect &rorect, vector<Point> contour, double innerArea);
		vector<vector<Point>> hanzi(Mat &src, vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, vector<RotatedRect>&rorects);
		void getRects(Mat &src, const vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, vector<Rect> &rects);
		Point2f computeIntersect(const Vec4f &a, const Vec4f &b);
		void getLine(Vec4f &line, const vector<Point> &points);
		void drawLine(Mat &src, Vec4f line){
			cv::line(src, Point((int)line[0], (int)line[1]), Point((int)line[2], (int)line[3]), Scalar(255, 0, 0), 2, 8, 0);
		};
		vector <Point2f> getCorners(Mat src, const vector<Rect> &rects, const vector<RotatedRect>&rorects);
		void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center);
		vector<Point2f> sortCorners(Point2f corners[4]);
		void PerspectiveTransform(const Mat &src, vector<Point2f> corners, Mat &dst);

		Size const getPlateSize(){
			return plate_pic_size;
		}
	private:
		Size plate_pic_size;
		const Mat element = cv::getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
		double SOBEL = 3;
		double WIDTH_SCAN = 0.20;
		double WIDTH_HEIGHT_MIN = 0.08;
		double WIDTH_HEIGHT_MAX = 2.555556;
		double SEC_WIDTH_HEIGHT_MAX = 0.75;
		double MAXSIZE = 0.200;
		double MINSIZE = 0.0017;
		double SEC_SIZE = 0.015;
		int LINE_SHIFT = 3;
		int RECT_ROTATE = 45;
	};
}
#endif 
