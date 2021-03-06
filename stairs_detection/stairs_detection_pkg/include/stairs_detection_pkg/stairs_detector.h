#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stairs_detection_pkg/neighbour_finder.h>
#define PI 3.141592654

class Line {
public:
	// In OpenCV x is col, y is row
	//
	// @param[in]  x1    The x1
	// @param[in]  y1    The y1
	// @param[in]  x2    The x2
	// @param[in]  y2    The y2
	//
	Line(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0) {
		this->k = (y1 - y2) / (x1 - x2);
		this->b = y1 - k * x1;
		this->t = atan((y1 - y2) / (x1 - x2));
		this->r = x1 * sin(t) + y1 * cos(t);
		this->p1 = *new cv::Point(x1, y1);
		this->p2 = *new cv::Point(x2, y2);
		this->center = p1 - p2;
		this->length = cv::norm(this->p1 - this->p2);
	}

	Line(cv::Point p1, cv::Point p2) : Line(p1.x, p1.y, p2.x, p2.y) {}

	Line(cv::Vec4i line) : Line(line[0], line[1], line[2], line[3]) {}

	Line() {}

	/**
	 * @brief      Calculate Pixels of the line on the image
	 *
	 * @param[in]  src   The source image
	 */
	void calPixels(cv::Mat src) {
		this->pixels.clear();
		cv::LineIterator it(src, p1, p2, 8, true);
		pixels_num = it.count;
		for (int i = 0; i < pixels_num; i++, ++it) {
			pixels.push_back(it.pos());
		}
	}

	friend std::ostream &operator<<(std::ostream &os, const Line &line) {
		os << "Point1: " << line.p1 << " Point2: " << line.p2 << " k:" << line.k
		   << " b:" << line.b << " t: " << line.t << " r: " << line.r;
		return os;
	}

	double k, b = 0;
	double r, t = 0;
	double length = 0;
	cv::Point p1;
	cv::Point p2;
	cv::Point center;
	int pixels_num = 0;
	// cv::LineIterator it;
	std::vector<cv::Point> pixels;
};
typedef std::vector<Line> Lines;

struct Stairs_Detector_Params
{
	// high level bools
	bool show_result = false;
	bool debug = false;
	bool ignore_invalid = false;
	bool fill_invalid = false;
	bool use_laplacian = false;

	// canny
	double canny_low_threshold = 40;
	double canny_ratio = 10;
	int    canny_kernel_size = 1;

	// hough transfrom
	double hough_min_line_length = 25; // pixel distance
	double hough_max_line_gap = 15;   // pixel distance
	double hough_threshold = 20;      //
	int    hough_rho = 8;            //
	double hough_theta = 1;           // angle

	// filter by slope histogram
	double filter_slope_hist_bin_width = 20;  // degree

	// filter by hard slope
	double filter_slope_bandwidth = 10; // angle
    
	// merge parameters
	double merge_max_dist_diff = 20; // pixel
	double merge_max_angle_diff = 30; // degree
	int    merge_close_count = 10; // number

	// bounding box
	int    minimum_line_num = 3;
};


class Stairs_Detector {
public:
	Stairs_Detector();

	/**
	 * @brief      Main function to get stairs
	 *
	 * @param[in]  input_image   The input image
	 * @param      bounding_box  The bounding box
	 *
	 * @return     The stairs.
	 */
	bool getStairs(const cv::Mat& input_image, std::vector<cv::Point>& bounding_box);

	/**
	 * @brief      Sets the parameter.
	 *
	 * @param[in]  param  The parameter
	 */
	void setParam(const Stairs_Detector_Params& param);

	/**
	* @brief      Draws xy lines.
	*
	* @param      image  The image
	* @param[in]  lines  The lines
	*
	*/

	void drawLines(cv::Mat &image, const Lines &lines, const cv::Scalar &color);
	/**
	 * @brief      Draws line's slope.
	 *
	 * @param      image  The image
	 * @param[in]  lines  The lines
	 * @param[in]  color  The color
	 */
	void drawLinesSlope(cv::Mat &image, const Lines &lines, const cv::Scalar &color);

	/**
	 * @brief      Draws line's radius.
	 *
	 * @param      image  The image
	 * @param[in]  lines  The lines
	 * @param[in]  color  The color
	 */
	void drawLinesRadius(cv::Mat &image, const Lines &lines, const cv::Scalar &color);

	/**
	 * @brief      Draws a box.
	 *
	 * @param[in]  image         The image
	 * @param[in]  bounding_box  The bounding box
	 */
	void drawBox(cv::Mat& image, const std::vector<cv::Point>& bounding_box);


	/**
	 * @brief      Visualize the histogram
	 *
	 * @param[in]  hist         The hist
	 * @param[in]  height       The height
	 * @param[in]  window_name  The window name
	 */
	void visualizeHist(const std::vector<int> hist, int height, std::string window_name);


	/**
	 * @brief      calculate the hough lines
	 *
	 * @param[in]  edge_image  The edge image
	 * @param      lines       The lines
	 */
	void houghLine(const cv::Mat &edge_image, Lines &lines);

	/**
	 * @brief      filter lines by a specific threshold and buff size
	 *
	 * @param[in]  input_lines     The input lines
	 * @param      filtered_lines  The filtered lines
	 */
	void filterLinesBySlopeThreshold(const Lines& input_lines, Lines& filtered_lines );

	/**
	 * @brief      filter lines by slope histogram
	 *
	 * @param[in]  input_lines     The input lines
	 * @param      filtered_lines  The filtered lines
	 */
	void filterLinesBySlopeHist(const Lines& input_lines, Lines& filtered_lines );

	/**
	 * @brief      Gets the bounding box.
	 *
	 * @param[in]  input_lines   The input lines
	 * @param      bounding_box  The bounding box
	 *
	 * @return     The bounding box.
	 */
	bool getBoundingBox(const cv::Mat input_rgb_image, const Lines &input_lines, std::vector<cv::Point>& bounding_box);

private:
	Stairs_Detector_Params param_;
	/**
	 * @brief      Gets the neighboor pixel identifier.
	 *
	 * @param[in]  row   The row
	 * @param[in]  col   The col
	 * @param      ids   The identifiers
	 *
	 * @return     if it's success or not
	 */
	bool getNeighbourId(int row, int col, std::vector<cv::Vec2i> &ids, const cv::Mat& input_image);


	/**
	 * @brief      Determines if lines are close.
	 *
	 * @param[in]  line1  The line 1
	 * @param[in]  line2  The line 2
	 *
	 * @return     True if close, False otherwise.
	 */
	bool isLineClose(const Line &line1, const Line &line2);

	/**
	 * @brief      Determines if line parallel.
	 *
	 * @param[in]  line1  The line 1
	 * @param[in]  line2  The line 2
	 *
	 * @return     True if line parallel, False otherwise.
	 */
	bool isLineParallel(const Line &line1, const Line &line2);

	typedef std::vector<std::vector<cv::Point> > MergePointsList;
	/**
	 * @brief      Get the merge list form the input lines, every line that can be
	 *             merged will be put in merge point list, any line that can't be
	 *             merged will be put in pre_merge_lines
	 *
	 * @param[in]  input_lines        The input lines
	 * @param      merge_points_list  The merge points list, type
	 * std::vector<std::vector<cv::Point> >
	 * @param      pre_merge_lines    The pre merge lines
	 *
	 * @return     void
	 */
	void getMergePointList(const Lines &input_lines, MergePointsList &merge_points_list, Lines &pre_merge_lines);

	/**
	 * @brief      merge all lines
	 *
	 * @param[in]  input_lines   The input XY lines
	 * @param      merged_lines  The merged lines
	 */
	void mergeLines(const cv::Mat& input_image, const Lines &input_lines, Lines &merged_lines);

	void ignoreInvalid(const cv::Mat& input_image, cv::Mat& filter_image);

	/**
	 * @function CannyThreshold
	 * @brief      Trackbar callback - Canny thresholds input with a ratio_ 1:3
	 *
	 * @param[in]  src   The source image should be GraySclae
	 * @param      edge  The out put edge image is Gray Scale
	 */
	void cannyEdgeDetection(const cv::Mat& input_image, cv::Mat &edge);
	void sobelEdgeDetection(const cv::Mat& input_image, cv::Mat &edge);
	void laplacianEdgeDetection(const cv::Mat& input_image, cv::Mat &edge);

	void fillByNearestNeighbour(const cv::Mat &image, cv::Mat& filled_image);

	bool isInbound(int x, int y, const cv::Mat& image);

};
