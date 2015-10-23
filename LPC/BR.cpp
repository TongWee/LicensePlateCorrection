#include "stdafx.h"
#include "BR.h"
using namespace br;
/*
name:BR
function:���캯��
remark:
param1 ԭʼͼ��Ĵ�СSize����
*/
BR::BR(Size size)
{
	plate_pic_size = size;
};
/*
name:int preprocess
function:Ԥ������˹ģ�����ҶȻ���
remark:
param1 Դͼ��
param2 Ŀ��ͼ��
*/
int BR::preprocess(const Mat &src, Mat &gray)
{
	//��˹�˲�
	GaussianBlur(src, src, Size(5, 5), 0, 0, BORDER_DEFAULT);
	Mat mat_gray;
	//���Ϊ��ɫͼ�񣬽��лҶȻ�
	if (src.channels() == 3)
		cvtColor(src, mat_gray, COLOR_RGB2GRAY);
	else
		mat_gray = src;
	gray = mat_gray.clone();
	return 1;
};
/*name:void verifyContour
function:ȥ����������
remark:
param1 Դͼ��
param2 ��������
param3 �������
*/
void BR::verifyContour(Mat &src, vector<vector<Point>> &contours, vector<Vec4i> &hierarchy){
	Rect rect;
	RotatedRect rorect;
	double contourSize;
	vector<vector<Point>> dst_contours;
	//Ϊ���������������ת����RGB�ռ�
	cvtColor(src, src, COLOR_GRAY2RGB);
	//����ÿ������
	for (int i = 0; i < contours.size(); i++)
	{
		rect = boundingRect(contours[i]);
		rorect = minAreaRect(contours[i]);
		contourSize = contourArea(contours[i]);
		//�����ͨ���BR��MBR������Ҫ�����Ϊ����ɫ
		if (!(verifySizes(rect) || verifySizes(rorect)))
		{
			drawContours(src, contours, i, COLOR_BLACK, FILLED, 8);
			continue;
		}
		//�����ͨ����ˮƽ�����ϣ���߿���ճ�������Ϊ����ɫ
		if (rect.tl().x < src.size().width*0.005 || rect.br().x>src.size().width*0.995)
		{
			drawContours(src, contours, i, COLOR_BLACK, FILLED, 8);
			continue;
		}
		//���ǰ������������Ӿ����У���ռ������С�����Ϊ����ɫ
		double radio = contourSize / rorect.size.area();
		if (radio<0.2){
			drawContours(src, contours, i, COLOR_BLACK, FILLED, 8);
			continue;
		}
	}
	//ת��Ϊ�Ҷ�ͼ��
	cvtColor(src, src, COLOR_RGB2GRAY);
	//����Ѱ������
	contours.clear();
	hierarchy.clear();
	Mat temp = src.clone();
	findContours(src, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	src = temp.clone();
}

/*
name:int BR::verifySizes(Rect rect)
function:ȥ����������Ӿ���
remark:
param1 ����
*/
int BR::verifySizes(const Rect &rect){
	double width = rect.width;
	double height = rect.height;
	//�߶�Ϊ0������������
	if (height == 0)
		return 0;
	double radio = width / height;
	double areaSize = width*height;
	double maxSize = plate_pic_size.area()*MAXSIZE;
	double minSize = plate_pic_size.area()*MINSIZE;
	//����Ȳ���������
	if (radio < WIDTH_HEIGHT_MIN || radio>WIDTH_HEIGHT_MAX)
		return 0;
	//�����������������
	if (areaSize < minSize || areaSize>maxSize)
		return 0;
	return 1;
}
/*
name:int verifySizes(RotatedRect rect)
function:ȥ����������С��Ӿ���
param1 ��С��Ӿ���
*/
int BR::verifySizes(const RotatedRect &rect){
	float width = rect.size.width;
	float height = rect.size.height;
	float areaSize = width*height;
	float maxSize = plate_pic_size.area()*MAXSIZE;
	float minSize = plate_pic_size.area()*MINSIZE;
	//�����������������
	if (areaSize < minSize || areaSize>maxSize)
		return 0; float temp_rotate = 0;
	//����ת�ǶȽ��й�һ������
	if (width < height)
		temp_rotate = 90 - rect.angle;
	else
		temp_rotate = -rect.angle;
	//��ת�ǶȲ���������
	if (temp_rotate < RECT_ROTATE || temp_rotate>180 - RECT_ROTATE)
		return 0;
	//����Ȳ���������
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
function:����ȥ����������Ӿ���
remark:
param1 ����
*/
int BR::SecverifySizes(const Rect &rect){
	float width = rect.width;
	float height = rect.height;
	float radio = width / height;
	//�������������
	if (rect.area() < plate_pic_size.area()*SEC_SIZE)
		return 0;
	//����Ȳ���������
	if (radio > SEC_WIDTH_HEIGHT_MAX)
		return 0;
	return 1;
};
/*
name:int BR::SecverifySizes(RotatedRect rect)
function:����ȥ����������С��Ӿ���
remark:
param1 ��С��Ӿ���
*/
int BR::SecverifySizes(const RotatedRect &rect){
	float width = rect.size.width;
	float height = rect.size.height;
	double max = width > height ? width : height;
	double cha = abs(width - height);
	float areaSize = width*height;
	//����Ȳ���������
	float radio = cha / max;
	if (radio < 0.38)
	{
		return 0;
	}
	//�������������
	if (areaSize < plate_pic_size.area()*SEC_SIZE)
		return 0;
	else
		return 1;
};
/*
name:hanzi
function:��������ϲ� �� ��С��Ӿ�������
remark:
param1 Դͼ��
param2 ��������
param3 �������
param4 ��С��Ӿ�������
*/
vector<vector<Point>> BR::hanzi(Mat &src, vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, vector<RotatedRect>&rorects){
	//ɨ����
	float scan_width = src.cols*WIDTH_SCAN;
	//����ɨ����
	//line(src, Point(scan_width, 0), Point(scan_width, src.rows), Scalar(0, 255, 0), 2, 8, 0);
	vector<Point> hanzi;
	vector<vector<Point>> br_contours;
	Rect rect;
	RotatedRect rorect, rorect_next;
	//��־λ����¼�ڲ��Ƿ������ڲ�����
	//1 �ڲ�������0 �����ڲ�����
	int rorect_flags[20] = { 0 };
	for (int i = 0; i < contours.size(); i++){
		rect = boundingRect(contours[i]);
		rorect = minAreaRect(contours[i]);
		int rect_width = rect.tl().x + rect.width;
		//ɨ�������Ϊ�������򣬴���hanzi
		if (rect_width < scan_width){
			for each (Point p in contours[i])
			{
				hanzi.push_back(p);
			}
		}
		//ɨ�����Ҳ�Ϊ�Ǻ�������
		else{
			br_contours.push_back(contours[i]);
			//���Ϊ�ڲ��������޳�
			if (rorect_flags[i] == 1)
				continue;
			//�����ǰ�������а������ڲ�����
			if (hierarchy[i][2] != -1)
			{
				rorect_next = minAreaRect(contours[hierarchy[i][2]]);
				//�ڲ�������־λ�� 1
				rorect_flags[hierarchy[i][2]] = 1;
				//��ת�Ƕȹ�һ��
				double angle = rorect_next.angle - rorect.angle;
				if (abs(angle)>45)
					rorect_next.angle = rorect_next.angle - 90;
				//�����ڲ�MBR����ת�Ƕȣ��ı������MBR����ת�Ƕ�
				//rorect.angle = (rorect_next.angle + rorect.angle) / 2;
				//�����ж�MBR�����������Ĵ�������
				if (SecverifySizes(rorect))
					rorects.push_back(rorect);
			}
			else
			{
				//���ַ� 1 ����С��Ӿ��Σ���Ƚ�����չ
				digitOne(rorect, contours[i], 0);
				//�����ж�MBR�����������Ĵ�������
				if (SecverifySizes(rorect))
					rorects.push_back(rorect);
			}
		}

	}
	//����������ϲ�������MBR����
	rorect = minAreaRect(hanzi);
	rorects.push_back(rorect);
	br_contours.push_back(hanzi);
	return br_contours;
}
/*
name:Sobel_Thresh
function:Sobel��ֵ��
remark:
param1 Դͼ��
param2 Ŀ��ͼ��
*/
int BR::Sobel_Thresh(const Mat &src, Mat &sobel)
{
	Mat dst_x, dst_y, Sobel;
	//x ���� Sobel
	cv::Sobel(src, dst_x, src.depth(), 2, 0, SOBEL);
	//y ���� Sobel
	cv::Sobel(src, dst_y, src.depth(), 0, 2, SOBEL);
	//��һ��
	convertScaleAbs(dst_x, dst_x);
	convertScaleAbs(dst_y, dst_y);
	//�ں�
	addWeighted(dst_x, 0.5, dst_y, 0.5, 0, Sobel);
	//�˲�ȥ��
	cv::blur(Sobel, Sobel, Size(2, 2), Point(-1, -1), 4);
	//imshow("Soble", Sobel);
	//��ֵ��
	threshold(Sobel, Sobel, 0, 255, THRESH_BINARY | THRESH_OTSU);
	sobel = src.clone();
	//��̬ѧȥ��
	erode(sobel, sobel, element, Point(-1, -1), 1);
	dilate(sobel, sobel, element, Point(-1, -1), 1);
	return 1;
}
/*
name:getRects
function:�����Ӿ���
remark:
param1 Դͼ��
param2 ��������
param3 �����������
param4 ��������
*/
void BR::getRects(Mat &src, const vector<vector<Point>> &contours, const vector<Vec4i> &hierarchy, vector<Rect> &rects){
	Rect rect, rect_next;
	rects.clear();
	//���λ 1 Ϊ�ڲ���Σ�0 Ϊ������
	int rect_flags[20] = { 0 };
	for (int i = 0; i < contours.size(); i++)
	{
		//����Խ�����
		if (i + 1 == contours.size())
			break;
		//Ϊ�ڲ���εģ��ñ��λΪ 1
		rect = boundingRect(contours[i]);
		rect_next = boundingRect(contours[i + 1]);
		if (rect.contains(rect_next.tl())){
			rect_flags[i + 2] = 1;
		}
	}
	//�������ν��ж����жϣ����������Ĵ�������
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
function:�ж��Ƿ�������һ
remark:
param1 MBR
param2 ����
param3 �ڲ�ǰ���������������׶���
return 1��Ϊ�ַ�1��0�������ַ�
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
	//ǰ������ռ��Ӿ�������㹻�����ж�Ϊ�ַ� 1
	if (radio > 0.82)
	{
		//���ַ�1����Ӿ��ο�Ƚ�����չ
		if (rorect.size.width < rorect.size.height)
			rorect.size.width = rorect.size.width * 3.5;
		else
			rorect.size.height = rorect.size.height*3.5;
		return 1;
	}
	//�����ַ� 1
	else
	{
		return 0;
	}
}
/*name: Point2f computeIntersect
function:������ֱ�߽���
remark:
param1 ֱ��1
param2 ֱ��2
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
function: ת��ֱ�ߴ洢��ʽ
remark:
param1 vec4fֱ��
param2 pointsֱ��
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
function: ��ȡ�����ַ�����ǵ�
remark:
param1 Դͼ��
param2 ��Ӿ�������
param3 ��С��Ӿ�������
*/
vector <Point2f> BR::getCorners(Mat src, const vector<Rect> &rects, const vector<RotatedRect>&rorects){
	vector<Point2f> four_corners;
	vector<Point> lqPoints_top;
	vector<Point> lqPoints_bottom;
	Vec4f topline, bottomline;
	//��ȡ�ַ���Ӿ������±��е�
	for each (Rect rect in rects)
	{
		int p_x = rect.tl().x + rect.width / 2;
		int p_y = rect.tl().y - LINE_SHIFT;
		lqPoints_top.push_back(Point(p_x, p_y));
		p_y = rect.tl().y + rect.height + LINE_SHIFT;
		lqPoints_bottom.push_back(Point(p_x, p_y));
	}
	//���topLine&bottomLine	
	getLine(topline, lqPoints_top);
	getLine(bottomline, lqPoints_bottom);
	//����С��Ӿ��λ��leftLine&rightLine
	RotatedRect left_rect, right_rect;
	left_rect = rorects.front();
	right_rect = rorects.front();
	//�������������Ҳ�MBR
	for each (RotatedRect rorect in rorects)
	{
		if (left_rect.center.x > rorect.center.x)
			left_rect = rorect;
		if (right_rect.center.x < rorect.center.x)
			right_rect = rorect;
	}
	//���leftline
	Point2f corners[4];
	left_rect.points(corners);
	vector<Point2f> corner = sortCorners(corners);
	Vec4f leftline(corner[0].x - LINE_SHIFT, corner[0].y, corner[3].x - LINE_SHIFT, corner[3].y);

	//���rightline
	right_rect.points(corners);
	corner.clear();
	corner = sortCorners(corners);
	Vec4f rightline(corner[1].x + LINE_SHIFT, corner[1].y, corner[2].x + LINE_SHIFT, corner[2].y);

	//����
	//drawLine(src, leftline);
	//drawLine(src, rightline);

	//����ֱ�߽��㣬��ó����ַ������ĸ��ǵ�
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
function: �Ե��������
remark:
param1 ������
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

	//˳��Ϊtopleft,topright,bottomright,bottomleft
	corner.push_back(tl);
	corner.push_back(tr);
	corner.push_back(br);
	corner.push_back(bl);
	return corner;
}
/*
name: void PerspectiveTransform
function:͸�ӱ任
remark:
param1 Դͼ��
param2 �ǵ�����
param3 Ŀ��ͼ��
*/
void BR::PerspectiveTransform(const Mat &src, vector<Point2f> corners, Mat &dst){
	//�õ�͸�ӱ任����
	vector<cv::Point2f> quad_pts;
	quad_pts.push_back(cv::Point2f(0, 0));
	quad_pts.push_back(cv::Point2f((float)dst.cols, 0));
	quad_pts.push_back(cv::Point2f((float)dst.cols, (float)dst.rows));
	quad_pts.push_back(cv::Point2f(0, (float)dst.rows));
	cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
	//͸�ӱ任
	cv::warpPerspective(src, dst, transmtx, dst.size());
}

/*
name:void BR_Correction
function:����ͼ��������
remark:
param1 Դͼ��
param2 Ŀ��ͼ��
param3 ��ϸ����ģʽ
*/
void BR::BR_Correction(Mat src, Mat &dst, bool flag){
	Mat preprocess_img;
	plate_pic_size = Size(src.cols, src.rows);
	//Ԥ����
	preprocess(src, preprocess_img);
	//��ֵ��
	Mat threshold_img;
	threshold(preprocess_img, threshold_img, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//Ѱ������
	Mat contour_img = threshold_img.clone();
	vector<vector<Point>> contours_src;
	vector<vector<Point>> contours_dst;
	vector<Vec4i> hierarchy;
	findContours(threshold_img, contours_src, RETR_LIST, CHAIN_APPROX_NONE);
	//ȥ����������
	Mat contours_img = src.clone();
	verifyContour(contour_img, contours_src, hierarchy);
	//��ϸ����ģʽ_��ʾȥ���Ķ�ֵ��ͼ��
	if (flag){
		imshow("Contour Image", contour_img);
		moveWindow("Contour Image", 1000, 200);
		waitKey(10);
	}
	//�ϲ���������
	vector<RotatedRect> rorects;
	Mat Hanzi_img = src.clone();
	contours_dst = hanzi(Hanzi_img, contours_src, hierarchy, rorects);
	//��ȡBR & MBR
	Mat MBR_BR_img = src.clone();
	vector<Rect> rects;
	getRects(MBR_BR_img, contours_dst, hierarchy, rects);
	//��ϸ����ģʽ_����BR&MBR
	if (flag){
		///////////////����BR////////////////
		Mat BR_img = src.clone();
		for each (Rect rect in rects)
		{
			rectangle(BR_img, rect, COLOR_GREEN, 2, 8, 0);
		}
		imshow("Bounding Rectangle", BR_img);
		moveWindow("Bounding Rectangle", 1000, 600);
		waitKey(10);
		///////////////����MBR////////////////
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
	//���ֱ�ߣ����ĸ�����
	vector<Point2f> four_vertex;
	Mat edge_img = src.clone();
	four_vertex = getCorners(edge_img, rects, rorects);
	//��ϸ����ģʽ_���Ƴ����ַ�����
	if (flag){
		for (int i = 0; i < 4; i++){
			line(edge_img, four_vertex[i], four_vertex[(i + 1) % 4], COLOR_ORANGLE, 4, 8, 0);
		}
		cv::imshow("Edge of License Plate", edge_img);
		moveWindow("Edge of License Plate", 1400, 600);
		cv::waitKey(10);
	}
	//͸�ӻ������
	cv::Mat correct_img = cv::Mat::zeros(90, 409, CV_8UC3);
	Mat distortion_img = src.clone();
	PerspectiveTransform(distortion_img, four_vertex, correct_img);
	dst = correct_img;
}