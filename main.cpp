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

const char* window_name = "Coin Map";

int main(int argc, char** argv)
{
    std::string input_image = "";
    int mode{-1};

    if (argc > 1)
    {
        std::vector<std::string> args_list(argv + 1, argv + argc);

        // First parameter is detection mode
        mode = atoi(argv[1]);
    
        if (mode < 1 || mode > 2)
        {
            std::cerr << "Invalid mode parameter.\n::[Modes]::\n1) Hugh Circles Algorithm\n2) Canny Edge Detection." << std::endl;
            return EXIT_FAILURE;
        }
        // Fetches the rest of user arguments (image file path);
        for (int i = 1; i < args_list.size(); i++)
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
    // std::vector<cv::Vec3f> coins;

    // Define Matrixes
    // Gray will hold the image's data converted to grayscale (COLOR_BGR2GRAY)
    cv::Mat src = display.clone();
    cv::Mat gray, thresh;

    auto drawing = src.clone();

    // Convert to grayscale
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    switch (mode)
    {
        case 1:
            // Apply gaussian blur
            cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);
            detection::detect_coins_hughcircles(drawing, gray);
            break;

        case 2:
            // Apply gaussian blur
            cv::GaussianBlur(gray, gray, cv::Size(7, 7), 0);
            detection::detect_coins_contours(gray, thresh, drawing);
            break;

        default:    
            std::cerr << "Something went wrong!" << std::endl;
            return EXIT_FAILURE;
    }

    //cv::namedWindow(window_name, cv::WINDOW_KEEPRATIO);
    //cv::imshow( window_name, drawing);

    cv::waitKey();
    //cv::destroyAllWindows();

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
