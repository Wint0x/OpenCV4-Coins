// Header-only class

#ifndef COINDETECT_HPP
#define COINDETECT_HPP

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <filesystem>

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>

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

#endif