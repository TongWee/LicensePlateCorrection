#include "stdafx.h"
#include "BR.h"
using namespace br;
/*
name:BR
function:构造函数
remark:
param1 原始图像的大小Size对象
*/
BR::BR(Size size)
{
	plate_pic_size = size;
};
/*
name:int preprocess
function:预处理（高斯模糊、灰度化）
remark:
param1 源图像
param2 目标图像
*/
int BR::preprocess(const Mat &src, Mat &gray)
{
	//高斯滤波
	GaussianBlur(src, src, Size(5, 5), 0, 0, BORDER_DEFAULT);
	Mat mat_gray;
	//如果为彩色图像，进行灰度化
	if (src.channels() == 3)
		cvtColor(src, mat_gray, COLOR_RGB2GRAY);
	else
		mat_gray = src;
	gray = mat_gray.clone();
	return 1;
};
/*name:void verifyContour
function:去除噪声轮廓
remark:
param1 源图像
param2 轮廓序列
param3 轮廓层次
*/
void BR::verifyContour(Mat &src, vector<vector<Point>> &contours, vector<Vec4i> &hierarchy){
	Rect rect;
	RotatedRect rorect;
	double contourSize;
	vector<vector<Point>> dst_contours;
	//为了填充噪声区域，先转换到RGB空间
	cvtColor(src, src, COLOR_GRAY2RGB);
	//遍历每个轮廓
	for (int i = 0; i < contours.size(); i++)
	{
		rect = boundingRect(contours[i]);
		rorect = minAreaRect(contours[i]);
		contourSize = contourArea(contours[i]);
		//如果连通域的BR或MBR不满足要求，填充为背景色
		if (!(verifySizes(rect) || verifySizes(rorect)))
		{
			drawContours(src, contours, i, COLOR_BLACK, FILLED, 8);
			continue;
		}
		//如果连通域在水平方向上，与边框发生粘连，填充为背景色
		if (rect.tl().x < src.size().width*0.005 || rect.br().x>src.size().width*0.995)
		{
			drawContours(src, contours, i, COLOR_BLACK, FILLED, 8);
			continue;
		}
		//如果前景区域在其外接矩形中，所占比例过小，填充为背景色
		double radio = contourSize / rorect.size.area();
		if (radio<0.2){
			drawContours(src, contours, i, COLOR_BLACK, FILLED, 8);
			continue;
		}
	}
	//转换为灰度图像
	cvtColor(src, src, COLOR_RGB2GRAY);
	//重新寻找轮廓
	contours.clear();
	hierarchy.clear();
	Mat temp = src.clone();
	findContours(src, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	src = temp.clone();
}

/*
name:int BR::verifySizes(Rect rect)
function:去除不合理外接矩形
remark:
param1 矩形
*/
int BR::verifySizes(const Rect &rect){
	double width = rect.width;
	double height = rect.height;
	//高度为0，不满足条件
	if (height == 0)
		return 0;
	double radio = width / height;
	double areaSize = width*height;
	double maxSize = plate_pic_size.area()*MAXSIZE;
	double minSize = plate_pic_size.area()*MINSIZE;
	//长宽比不满足条件
	if (radio < WIDTH_HEIGHT_MIN || radio>WIDTH_HEIGHT_MAX)
		return 0;
	//矩形面积不满足条件
	if (areaSize < minSize || areaSize>maxSize)
		return 0;
	return 1;
}
/*
name:int verifySizes(RotatedRect rect)
function:去除不合理最小外接矩形
param1 最小外接矩形
*/
int BR::verifySizes(const RotatedRect &rect){
	float width = rect.size.width;
	float height = rect.size.height;
	float areaSize = width*height;
	float maxSize = plate_pic_size.area()*MAXSIZE;
	float minSize = plate_pic_size.area()*MINSIZE;
	//矩形面积不满足条件
	if (areaSize < minSize || areaSize>maxSize)
		return 0; float temp_rotate = 0;
	//对旋转角度进行归一化处理
	if (width < height)
		temp_rotate = 90 - rect.angle;
	else
		temp_rotate = -rect.angle;
	//旋转角度不满足条件
	if (temp_rotate < RECT_ROTATE || temp_rotate>180 - RECT_ROTATE)
		return 0;
	//长宽比不满足条件
	float r = 0;
	if (width > height)
		r = width / height;
	else
		r = height / width;
	if (r > 9.2)
		return 0;
	return 1;
}
/*
name:int BR::SecverifySizes(Rect rect)
function:二次去除不合理外接矩形
remark:
param1 矩形
*/
int BR::SecverifySizes(const Rect &rect){
	float width = rect.width;
	float height = rect.height;
	float radio = width / height;
	//面积不满足条件
	if (rect.area() < plate_pic_size.area()*SEC_SIZE)
		return 0;
	//长宽比不满足条件
	if (radio > SEC_WIDTH_HEIGHT_MAX)
		return 0;
	return 1;
};
/*
name:int BR::SecverifySizes(RotatedRect rect)
function:二次去除不合理最小外接矩形
remark:
param1 最小外接矩形
*/
int BR::SecverifySizes(const RotatedRect &rect){
	float width = rect.size.width;
	float height = rect.size.height;
	double max = width > height ? width : height;
	double cha = abs(width - height);
	float areaSize = width*height;
	//长宽比不满足条件
	float radio = cha / max;
	if (radio < 0.38)
	{
		return 0;
	}
	//面积不满足条件
	if (areaSize < plate_pic_size.area()*SEC_SIZE)
		return 0;
	else
		return 1;
};
/*
name:hanzi
function:汉字区域合并 和 最小外接矩形序列
remark:
param1 源图像
param2 轮廓序列
param3 轮廓层次
param4 最小外接矩形序列
*/
vector<vector<Point>> BR::hanzi(Mat &src, vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, vector<RotatedRect>&rorects){
	//扫描线
	float scan_width = src.cols*WIDTH_SCAN;
	//绘制扫描线
	//line(src, Point(scan_width, 0), Point(scan_width, src.rows), Scalar(0, 255, 0), 2, 8, 0);
	vector<Point> hanzi;
	vector<vector<Point>> br_contours;
	Rect rect;
	RotatedRect rorect, rorect_next;
	//标志位，记录内部是否属于内侧轮廓
	//1 内侧轮廓；0 不是内侧轮廓
	int rorect_flags[20] = { 0 };
	for (int i = 0; i < contours.size(); i++){
		rect = boundingRect(contours[i]);
		rorect = minAreaRect(contours[i]);
		int rect_width = rect.tl().x + rect.width;
		//扫描线左侧为汉字区域，存入hanzi
		if (rect_width < scan_width){
			for each (Point p in contours[i])
			{
				hanzi.push_back(p);
			}
		}
		//扫描线右侧为非汉字区域
		else{
			br_contours.push_back(contours[i]);
			//如果为内侧轮廓，剔除
			if (rorect_flags[i] == 1)
				continue;
			//如果当前轮廓，有包含的内侧轮廓
			if (hierarchy[i][2] != -1)
			{
				rorect_next = minAreaRect(contours[hierarchy[i][2]]);
				//内侧轮廓标志位置 1
				rorect_flags[hierarchy[i][2]] = 1;
				//旋转角度归一化
				double angle = rorect_next.angle - rorect.angle;
				if (abs(angle)>45)
					rorect_next.angle = rorect_next.angle - 90;
				//依据内侧MBR的旋转角度，改变最外侧MBR的旋转角度
				//rorect.angle = (rorect_next.angle + rorect.angle) / 2;
				//二次判断MBR，符合条件的存入序列
				if (SecverifySizes(rorect))
					rorects.push_back(rorect);
			}
			else
			{
				//对字符 1 的最小外接矩形，宽度进行扩展
				digitOne(rorect, contours[i], 0);
				//二次判断MBR，符合条件的存入序列
				if (SecverifySizes(rorect))
					rorects.push_back(rorect);
			}
		}

	}
	//将汉字区域合并，存入MBR序列
	rorect = minAreaRect(hanzi);
	rorects.push_back(rorect);
	br_contours.push_back(hanzi);
	return br_contours;
}
/*
name:Sobel_Thresh
function:Sobel二值化
remark:
param1 源图像
param2 目标图像
*/
int BR::Sobel_Thresh(const Mat &src, Mat &sobel)
{
	Mat dst_x, dst_y, Sobel;
	//x 方向 Sobel
	cv::Sobel(src, dst_x, src.depth(), 2, 0, SOBEL);
	//y 方向 Sobel
	cv::Sobel(src, dst_y, src.depth(), 0, 2, SOBEL);
	//归一化
	convertScaleAbs(dst_x, dst_x);
	convertScaleAbs(dst_y, dst_y);
	//融合
	addWeighted(dst_x, 0.5, dst_y, 0.5, 0, Sobel);
	//滤波去噪
	cv::blur(Sobel, Sobel, Size(2, 2), Point(-1, -1), 4);
	//imshow("Soble", Sobel);
	//二值化
	threshold(Sobel, Sobel, 0, 255, THRESH_BINARY | THRESH_OTSU);
	sobel = src.clone();
	//形态学去噪
	erode(sobel, sobel, element, Point(-1, -1), 1);
	dilate(sobel, sobel, element, Point(-1, -1), 1);
	return 1;
}
/*
name:getRects
function:获得外接矩形
remark:
param1 源图像
param2 轮廓序列
param3 轮廓层次序列
param4 矩形序列
*/
void BR::getRects(Mat &src, const vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, vector<Rect> &rects){
	Rect rect, rect_next;
	rects.clear();
	//标记位 1 为内侧矩形，0 为外侧矩形
	int rect_flags[20] = { 0 };
	for (int i = 0; i < contours.size(); i++)
	{
		//避免越界错误
		if (i + 1 == contours.size())
			break;
		//为内侧矩形的，置标记位为 1
		rect = boundingRect(contours[i]);
		rect_next = boundingRect(contours[i + 1]);
		if (rect.contains(rect_next.tl())){
			rect_flags[i + 2] = 1;
		}
	}
	//对外侧矩形进行二次判断，符合条件的存入序列
	for (int i = 0; i < contours.size(); i++)
	{
		if (rect_flags[i + 1] == 1){
			continue;
		}
		rect = boundingRect(contours[i]);
		if (SecverifySizes(rect)){
			rects.push_back(rect);
		}
	}
	rects.push_back(rect);
}
/*name:int digitOne
function:判断是否是数字一
remark:
param1 MBR
param2 轮廓
param3 内部前景区域面积（避免孔洞）
return 1则为字符1；0则其他字符
*/
int BR::digitOne(RotatedRect &rorect, vector<Point> contour, double innerArea){
	double areaSize, rectSize;
	rorect = minAreaRect(contour);
	rectSize = rorect.size.area();
	if (innerArea == 0)
	{
		areaSize = contourArea(contour);
	}
	else
	{
		areaSize = rectSize - innerArea;
	}
	double radio = areaSize / rectSize;
	//前景区域占外接矩形面积足够大，则判定为字符 1
	if (radio > 0.82)
	{
		//对字符1的外接矩形宽度进行扩展
		if (rorect.size.width < rorect.size.height)
			rorect.size.width = rorect.size.width * 3.5;
		else
			rorect.size.height = rorect.size.height*3.5;
		return 1;
	}
	//不是字符 1
	else
	{
		return 0;
	}
}
/*name: Point2f computeIntersect
function:计算两直线交点
remark:
param1 直线1
param2 直线2
*/
Point2f BR::computeIntersect(const Vec4f &a, const Vec4f &b){
	float x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	float x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
	{
		Point2f pt;
		pt.x = ((x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4)) / d;
		pt.y = ((x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4)) / d;
		return pt;
	}
	else
		return Point2f(-1, -1);
}
/*name:void getLine
function: 转换直线存储方式
remark:
param1 vec4f直线
param2 points直线
*/
void BR::getLine(Vec4f &line, const vector<Point> &points){
	fitLine(points, line, DIST_L12, 0, 0.01, 0.01);
	int x0 = line[2];
	int y0 = line[3];
	int x1 = x0 - 500 * line[0];
	int y1 = y0 - 500 * line[1];
	line[0] = x1;
	line[1] = y1;
};

/*
name:vetor<Point2f> getCorners
function: 获取车牌字符区域角点
remark:
param1 源图像
param2 外接矩形序列
param3 最小外接矩形序列
*/
vector <Point2f> BR::getCorners(Mat src, const vector<Rect> &rects, const vector<RotatedRect>&rorects){
	vector<Point2f> four_corners;
	vector<Point> lqPoints_top;
	vector<Point> lqPoints_bottom;
	Vec4f topline, bottomline;
	//获取字符外接矩形上下边中点
	for each (Rect rect in rects)
	{
		int p_x = rect.tl().x + rect.width / 2;
		int p_y = rect.tl().y - LINE_SHIFT;
		lqPoints_top.push_back(Point(p_x, p_y));
		p_y = rect.tl().y + rect.height + LINE_SHIFT;
		lqPoints_bottom.push_back(Point(p_x, p_y));
	}
	//拟合topLine&bottomLine	
	getLine(topline, lqPoints_top);
	getLine(bottomline, lqPoints_bottom);
	//由最小外接矩形获得leftLine&rightLine
	RotatedRect left_rect, right_rect;
	left_rect = rorects.front();
	right_rect = rorects.front();
	//检测最左侧与最右侧MBR
	for each (RotatedRect rorect in rorects)
	{
		if (left_rect.center.x > rorect.center.x)
			left_rect = rorect;
		if (right_rect.center.x < rorect.center.x)
			right_rect = rorect;
	}
	//获得leftline
	Point2f corners[4];
	left_rect.points(corners);
	vector<Point2f> corner = sortCorners(corners);
	Vec4f leftline(corner[0].x - LINE_SHIFT, corner[0].y, corner[3].x - LINE_SHIFT, corner[3].y);

	//获得rightline
	right_rect.points(corners);
	corner.clear();
	corner = sortCorners(corners);
	Vec4f rightline(corner[1].x + LINE_SHIFT, corner[1].y, corner[2].x + LINE_SHIFT, corner[2].y);

	//绘制
	//drawLine(src, leftline);
	//drawLine(src, rightline);

	//计算直线交点，获得车牌字符区域四个角点
	Point2f point;
	//tl
	point = computeIntersect(topline, leftline);
	four_corners.push_back(point);
	//cout << "TL:"<<point << endl;
	//tr
	point = computeIntersect(topline, rightline);
	four_corners.push_back(point);
	//cout << "TR:" << point << endl;
	//br
	point = computeIntersect(bottomline, rightline);
	four_corners.push_back(point);
	//cout << "BR:" << point << endl;
	//bl
	point = computeIntersect(bottomline, leftline);
	four_corners.push_back(point);
	//cout << "BL:" << point << endl;	
	return four_corners;
}

/*
name:vetor<Point2f> sortCorners
function: 对点进行排序
remark:
param1 点序列
*/
vector<Point2f> BR::sortCorners(Point2f corners[4])
{
	vector<Point2f> corner;
	cv::Point2f center(0, 0);
	for (int i = 0; i < 4; i++)
		center += corners[i];
	center *= (1. / 4);

	std::vector<cv::Point2f> top, bot;

	for (int i = 0; i < 4; i++)
	{
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}

	cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
	cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
	cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
	cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

	//顺序为topleft,topright,bottomright,bottomleft
	corner.push_back(tl);
	corner.push_back(tr);
	corner.push_back(br);
	corner.push_back(bl);
	return corner;
}
/*
name: void PerspectiveTransform
function:透视变换
remark:
param1 源图像
param2 角点序列
param3 目标图像
*/
void BR::PerspectiveTransform(const Mat &src, vector<Point2f> corners, Mat &dst){
	//得到透视变换矩阵
	vector<cv::Point2f> quad_pts;
	quad_pts.push_back(cv::Point2f(0, 0));
	quad_pts.push_back(cv::Point2f((float)dst.cols, 0));
	quad_pts.push_back(cv::Point2f((float)dst.cols, (float)dst.rows));
	quad_pts.push_back(cv::Point2f(0, (float)dst.rows));
	cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
	//透视变换
	cv::warpPerspective(src, dst, transmtx, dst.size());
}

/*
name:void BR_Correction
function:车牌图像畸变矫正
remark:
param1 源图像
param2 目标图像
param3 详细过程模式
*/
void BR::BR_Correction(Mat src, Mat &dst, bool flag){
	Mat preprocess_img;
	plate_pic_size = Size(src.cols, src.rows);
	//预处理
	preprocess(src, preprocess_img);
	//二值化
	Mat threshold_img;
	threshold(preprocess_img, threshold_img, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//寻找轮廓
	Mat contour_img = threshold_img.clone();
	vector<vector<Point>> contours_src;
	vector<vector<Point>> contours_dst;
	vector<Vec4i> hierarchy;
	findContours(threshold_img, contours_src, RETR_LIST, CHAIN_APPROX_NONE);
	//去除噪声轮廓
	Mat contours_img = src.clone();
	verifyContour(contour_img, contours_src, hierarchy);
	//详细过程模式_显示去噪后的二值化图像
	if (flag){
		imshow("Contour Image", contour_img);
		moveWindow("Contour Image", 1000, 200);
		waitKey(10);
	}
	//合并汉字区域
	vector<RotatedRect> rorects;
	Mat Hanzi_img = src.clone();
	contours_dst = hanzi(Hanzi_img, contours_src, hierarchy, rorects);
	//获取BR & MBR
	Mat MBR_BR_img = src.clone();
	vector<Rect> rects;
	getRects(MBR_BR_img, contours_dst, hierarchy, rects);
	//详细过程模式_绘制BR&MBR
	if (flag){
		///////////////绘制BR////////////////
		Mat BR_img = src.clone();
		for each (Rect rect in rects)
		{
			rectangle(BR_img, rect, COLOR_GREEN, 2, 8, 0);
		}
		imshow("Bounding Rectangle", BR_img);
		moveWindow("Bounding Rectangle", 1000, 600);
		waitKey(10);
		///////////////绘制MBR////////////////
		Mat MBR_img = src.clone();
		for each (RotatedRect rorect in rorects)
		{
			Point2f corners[4];
			rorect.points(corners);
			for (int i = 0; i < 4; i++){
				cv::line(MBR_img, corners[i], corners[(i + 1) % 4], Scalar(255, 0, 255), 2, 8, 0);
			}
		}
		cv::imshow("Minimum Bounding Rectangle", MBR_img);
		moveWindow("Minimum Bounding Rectangle", 1400, 200);
		waitKey(10);
	}
	//拟合直线，求四个顶点
	vector<Point2f> four_vertex;
	Mat edge_img = src.clone();
	four_vertex = getCorners(edge_img, rects, rorects);
	//详细过程模式_绘制车牌字符区域
	if (flag){
		for (int i = 0; i < 4; i++){
			line(edge_img, four_vertex[i], four_vertex[(i + 1) % 4], COLOR_ORANGLE, 4, 8, 0);
		}
		cv::imshow("Edge of License Plate", edge_img);
		moveWindow("Edge of License Plate", 1400, 600);
		cv::waitKey(10);
	}
	//透视畸变矫正
	cv::Mat correct_img = cv::Mat::zeros(90, 409, CV_8UC3);
	Mat distortion_img = src.clone();
	PerspectiveTransform(distortion_img, four_vertex, correct_img);
	dst = correct_img;
}