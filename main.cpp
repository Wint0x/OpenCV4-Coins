#include "coindetect.hpp"

const bool equal_mats(const cv::Mat&, const cv::Mat&);

// Path of the data set
const char* DATA_SET_PATH = "/home/mintdev/Desktop/OpenCV/OPENCVTEST/build/DATA_SETS/coins/data/train";

// Just some Colors.
/*
cv::Scalar BLACK = cv::Scalar(0,0,0);
cv::Scalar BLUE = cv::Scalar(255, 178, 50);
cv::Scalar YELLOW = cv::Scalar(0, 255, 255);
cv::Scalar RED = cv::Scalar(0,0,255);
*/

const char* window_name = "Edge Map";

int main(int argc, char** argv)
{
    std::string input_image = "";
    if (argc > 1)
    {
        std::vector<std::string> args_list(argv + 1, argv + argc);

        for (int i = 0; i < args_list.size(); i++)
        {
            if (i != args_list.size() -1)
                input_image.append(args_list[i] + " ");
            else
                input_image.append(args_list[i]);
        }

        if (!fs::is_regular_file(input_image.c_str()))
        {
            std::cerr << "Not a valid file!\n";
            return EXIT_FAILURE;
        }
    }

    else
    {
        std::cerr << "Please supply a file path...\n";
        return EXIT_FAILURE;
    }

    // Mat and Path map
    std::unordered_map<cv::String, cv::Mat> cMap;

    // Thread for loading data_set images
    std::thread reader_t(preprocess::read_dataset, DATA_SET_PATH);
    reader_t.join();

    // Debug message
    // std::cout << "Done\n" << "Loaded: " << preprocess::fn.size() << " files!" << std::endl; 

    // Convert to matrixes
    int num_threads = std::thread::hardware_concurrency();

    std::vector<std::thread> threads;
    std::mutex imageMatsMutex;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([i, num_threads, &imageMatsMutex, &cMap]()
        {
            for (int j = i; j < preprocess::fn.size(); j += num_threads) 
            {
                cv::Mat img_mat = preprocess::convert_image(preprocess::fn[j]);

                if (!img_mat.empty()) 
                {
                    // Lock the vector to avoid data races
                    std::lock_guard<std::mutex> lock(imageMatsMutex);

                    // Add to Map
                    std::pair<cv::String, cv::Mat> pair;
                    pair = {preprocess::fn[j], img_mat};
                    cMap.insert(pair);

                    preprocess::matrixes.push_back(img_mat);
                }
            }
        });
    }

    for (std::thread& t : threads)
    {
        t.join();
    }

    // std::cout << "Loaded: " << preprocess::matrixes.size() << " Mats!" << std::endl;

    // Compare mats test
    
    // Open target image (Grayscale)
    cv::Mat load_path = preprocess::convert_image(cv::String(input_image));

    // Reopen a colored version of the image
    cv::Mat display = cv::imread(cv::String(input_image), cv::IMREAD_COLOR);

    if (display.empty() || load_path.empty())
    {
        std::cerr << "Could not read " << input_image << '!' << std::endl;
        return EXIT_FAILURE;
    }

    // Check if image used is a sample from the dataset itself
    bool isFound = false;
    for (const auto& [name, matrix] : cMap)
    {
        if (equal_mats(load_path, matrix))
        {
            std::cout << std::flush;
            std::cout << "Found match: " << name << '\n';
            isFound = true;
            return EXIT_SUCCESS;
        }
    }

    // ========================================================= //
    // ========================================================= //
    // ========================================================= //

    /* Coin detection system (Counts coins) */
    std::vector<cv::Vec3f> coins;

    // Define Matrixes
    // Gray will hold the image's data converted to grayscale (COLOR_BGR2GRAY)
    cv::Mat src = display.clone();
    cv::Mat gray, thresh;
    // dst.create(src.size(), src.type());

    // Resize original image
    cv::resize(src, src, cv::Size(1080, 1080), cv::INTER_LINEAR);

    cv::Mat drawing = src.clone();

    // Convert to grayscale
    cv::cvtColor( src, gray, cv::COLOR_BGR2GRAY );

    // Blur original image
    cv::GaussianBlur( gray, gray, cv::Size(11,11), 0);

    // Canny Thresholding operation
    // 3 -> cv::THRESH_BINARY type
    cv::Canny(gray, thresh, 30, 60, 3);

    // Dilate image
    cv::dilate(thresh, thresh, cv::Mat(), cv::Point(1, 1), 2, 1, 1);

    cv::namedWindow( window_name, cv::WINDOW_NORMAL );

    // Canny the blurred grayscale image
    /*
        cv::Canny(input_img, output_img, low_threshold, max_threshold, kernel_size)
    */
    // cv::Canny(gray, thresh, 100, 200);

    // Find Contours
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours( thresh, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );

    // Draw contours in our original image
    // cv::Mat drawing = cv::Mat::zeros( thresh.size(), CV_8UC3 );
    auto color = cv::Scalar(0,0,255); // RED

    cv::drawContours(drawing, contours, -1, color, 2);

    /* GET THE AREA OF EACH CONTOUR */
    std::unordered_map<uint32_t, double> areas;
    for ( uint32_t i = 0;  i < contours.size();  i++ )
    {
        std::pair<decltype(i), double> pair = {i, cv::contourArea(contours[i])};
        areas.insert(pair);
    }

    std::cout << areas.size() << std::endl;

    // std::cout << "Detected: " << contours.size() << " coins!" << std::endl; 

    /* 
    Comment / Uncomment this if you need OpenCV to display a window with detected coins
    for( size_t i = 0; i < coins.size(); i++ )
    {
         cv::Point center(cvRound(coins[i][0]), cvRound(coins[i][1]));
         int radius = cvRound(coins[i][2]);
         // draw the circle center
         cv::circle( src, center, 3, cv::Scalar(0,255,0), -1, 8, 0 );
         // draw the circle outline
         cv::circle( src, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );
    }
    */

    cv::imshow( window_name, drawing );

    cv::waitKey();

    return EXIT_SUCCESS;
}

const bool equal_mats(const cv::Mat& mat1, const cv::Mat& mat2)
{
            // treat two empty mat as identical as well
        if (mat1.empty() && mat2.empty())
            return true;
        
        // if dimensionality of two mat is not identical, these two mat is not identical
        if (mat1.cols != mat2.cols || mat1.rows != mat2.rows || mat1.dims != mat2.dims)
            return false;

        cv::Mat diff;
        cv::compare(mat1, mat2, diff, cv::CMP_NE);
        int nz = cv::countNonZero(diff);
        return nz==0;
}


