//
// Created by Alina on 07.04.2025.
//

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include "recognition.hpp"

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

    //Inverted threshold: dark areas become white (foreground)
    cv::Mat binary;
    cv::threshold(img, binary, 100, 255, cv::THRESH_BINARY_INV); // adjust 100 if needed

    //Morphology to clean up small noise (if dots are noisy or broken)
    cv::Mat cleaned;
    cv::morphologyEx(binary, cleaned, cv::MORPH_OPEN,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

    //Apply mask to original image
    cv::Mat result;
    img.copyTo(result, cleaned);

    cv::imwrite("dots_mask.png", cleaned);
    cv::imwrite("dots_only.png", result);

    cv::imshow("Binary Inverted", binary);
    cv::imshow("Cleaned Mask", cleaned);
    cv::imshow("Final Result", result);
    cv::waitKey(0);

//    cap.release();

    return 0;
}
