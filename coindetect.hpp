// Header-only class

#ifndef COINDETECT_HPP
#define COINDETECT_HPP

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <filesystem>
#include <random>

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>

namespace fs = std::filesystem;

namespace preprocess
{
    // Data Set Loaders
    inline static std::vector<cv::String> fn;
    inline static std::vector<cv::Mat> matrixes;

    // Mutex
    static std::mutex mtx;

    // Convert image to Matrix utility function
    inline cv::Mat convert_image(const cv::String& file)
    {
        cv::Mat img = cv::imread(file, cv::IMREAD_GRAYSCALE);
        return img;
    }

    // Loads the DataSet
    inline static void read_dataset(const char* PATH)
    {
        fs::recursive_directory_iterator rdi(PATH, fs::directory_options::skip_permission_denied);
        mtx.lock();
        for (const fs::directory_entry& de : rdi)
        {
            if (de.path().extension() == ".jpg")
                fn.push_back(cv::String(de.path()));
        }
        mtx.unlock();
    }

};

namespace detection
{
    static void detect_coins_hughcircles(cv::Mat& output, cv::Mat& gray)
    {
        // Use HoughCircles to detect circles in the image
        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, gray.rows / 8, 200, 100, 0, 0);

        // Check if any circles are detected
        if (circles.empty()) 
        {
            std::cerr << "No coins detected." << std::endl;
            return;
        }

        // Draw the circles on the original image
        for (const auto& circle : circles) 
        {
            cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
            int radius = cvRound(circle[2]);
            // Draw the circle center
            cv::circle(output, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
            // Draw the circle outline
            cv::circle(output, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
        }

        std::cout << "Around: " << circles.size() << " coin/s detected!" << std::endl;
    }

    static void detect_coins_contours(cv::Mat& gray, cv::Mat& thresh, cv::Mat& drawing)
    {
        cv::Mat temp;
        // otsu threshold 
        double otsu_thresh_val = cv::threshold(
        gray, temp, 0, 255, 
        cv::THRESH_BINARY | cv::THRESH_OTSU);

        // Canny edge detection (in, out, min threshold, max threshold, kernel size)
        cv::Canny(gray, thresh, otsu_thresh_val * 0.5, otsu_thresh_val, 3);

        // Apply dilation
        cv::dilate(thresh, thresh, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(5, 5)));

        // Find contours
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;

        cv::findContours(thresh.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        //std::cout << contours.size() << std::endl;

        // Draw contours
        cv::drawContours(drawing, contours, -1, cv::Scalar(0, 0, 255), 2);

        std::cout << "Around: " << contours.size() << " coin/s detected!" << std::endl;
    }
};

#endif
