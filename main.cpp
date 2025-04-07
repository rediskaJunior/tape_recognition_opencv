#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

#include "recognition.hpp" //there will be algorithm with recognition

void printCoordinates(const cv::Mat& binaryMask) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    //find contours of the white regions
    cv::findContours(binaryMask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    //show num of dots
    std::cout << "Found " << contours.size() << " dots on the photo:" << std::endl;

    for (size_t i = 0; i < contours.size(); ++i) {
        //compute the center
        cv::Moments m = cv::moments(contours[i]);
        if (m.m00 > 0) {
            //if center is > 0 (num of white pixels in it)
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);
            std::cout << "Dot " << i + 1 << ": (" << cy << ", " << cx << ")" << std::endl;
        }
    }
}

int main(){

    //save for later
//    cv::VideoCapture cap(0); //for now its web of the laptop
//    cv::Mat frame;
//    while(cap.read(frame)){

//trying to understand how to work with it
    cv::Mat img = cv::imread("test.jpg", cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "Image not found" << std::endl;
        return -1;
    }

    //inverted threshold: dark areas become white (foreground)
    cv::Mat binary;
    cv::threshold(img, binary, 100, 255, cv::THRESH_BINARY_INV); // adjust 100 if needed

    //morphology to clean up small noise (if dots are noisy or broken)
    cv::Mat cleaned;
    cv::morphologyEx(binary, cleaned, cv::MORPH_OPEN,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

    printCoordinates(cleaned);
    //apply mask to original image
    cv::Mat result;
    img.copyTo(result, cleaned);
    //from 0 to 10 -- our dots on the tape


    cv::imwrite("dots_mask.png", cleaned);
    cv::imwrite("dots_only.png", result);

    cv::imshow("Binary Inverted", binary);
    cv::waitKey(0);
    cv::imshow("Cleaned Mask", cleaned);
    cv::waitKey(0);
    cv::imshow("Final Result", result);
    cv::waitKey(0);

//    cap.release();

    return 0;
}
