#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <thread>
#include <atomic>

#include "recognition.hpp" //there will be algorithm with recognition

std::atomic<bool> keyPressed(true);
std::atomic<bool> workIsDone(false);

void waitForDone() {
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "Done" || input == "done" || input == "d") {
            keyPressed = true;
        } else if (input == "End" || input == "end" || input == "e"){
            std::cout << "Program ended" << std::endl;
            workIsDone = true;
            keyPressed = true;
        }
    }
}

void printCoordinatesHough(const cv::Mat& grayImage) {
    cv::Mat workingImage = grayImage.clone(); // Create a copy of the image to avoid modifying the original

    cv::bitwise_not(workingImage, workingImage);// For black dots on white background, invert the image

    cv::Mat blurred;// Apply blur to reduce noise
    cv::GaussianBlur(workingImage, blurred, cv::Size(5, 5), 1.5);

    cv::Mat binary;     // Apply adaptive thresholding to handle uneven lighting
    cv::adaptiveThreshold(blurred, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY, 11, 2);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));     // Clean up using morphological operations
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    cv::Mat visualImage = grayImage.clone();
    cv::cvtColor(visualImage, visualImage, cv::COLOR_GRAY2BGR);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(binary, circles, cv::HOUGH_GRADIENT, 1,
                     8,  // min distance between circles (reduced)
                     50,  // param1: higher threshold for Canny edge detector (reduced)
                     10,  // param2: accumulator threshold (reduced significantly)
                     0.9,   // min radius (reduced for small dots)
                     10   // max radius (reduced)
    );

    std::cout << "Found " << circles.size() << " circles on the photo:" << std::endl;

    for (size_t i = 0; i < circles.size(); i++) {
        int cx = static_cast<int>(circles[i][0]);
        int cy = static_cast<int>(circles[i][1]);
        int radius = static_cast<int>(circles[i][2]);

        std::cout << "Circle " << i + 1 << ": (" << cy << ", " << cx << "), Radius: " << radius << std::endl;

        cv::circle(visualImage, cv::Point(cx, cy), 2, cv::Scalar(0, 255, 0), -1);
        cv::circle(visualImage, cv::Point(cx, cy), radius, cv::Scalar(0, 0, 255), 1);
    }

    cv::imshow("Original Grayscale", grayImage);
    cv::imshow("Inverted", workingImage);
    cv::imshow("Binary", binary);
    cv::imshow("Detected Circles", visualImage);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

int main() {
    std::thread inputThread(waitForDone); //background thread for terminal input
    inputThread.detach();

    std::cout << "Start of the work. Initializing the camera" << std::endl;

    while (!workIsDone) {
        if (keyPressed) {
            keyPressed = false;
            cv::VideoCapture cap(4);
            if (!cap.isOpened()) {
                std::cerr << "Error: Could not open camera." << std::endl;
                return -1;
            }

            //wait for camera to initialize
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            //capture frame
            cv::Mat photo;
            bool success = cap.read(photo);
            if (!success) {
                std::cerr << "Error: Could not read from camera." << std::endl;
                return -1;
            }

            bool saved = cv::imwrite("img.jpg", photo);
            if (!saved) {
                std::cerr << "!!Error: Could not save image.!!" << std::endl;
                return -1;
            }

            std::cout << "Image captured and saved successfully." << std::endl;
            cap.release();

            cv::Mat gray;
            cv::cvtColor(photo, gray, cv::COLOR_BGR2GRAY);

            // cv::Mat binary;
            // cv::threshold(gray, binary, 200, 255, cv::THRESH_BINARY_INV);
            //
            // //morphology to clean up small noise (if dots are noisy or broken)
            // cv::Mat cleaned;
            // cv::morphologyEx(binary, cleaned, cv::MORPH_OPEN,
            //                  cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

            //print for further debug
            // printCoordinates(cleaned);
            printCoordinatesHough(gray);
            //apply mask to original image (now is not working as it should)
            // cv::Mat result;
            // frame.copyTo(result, cleaned);
            //from 0 to 10 -- our dots on the tape



           //  cv::imwrite("dots_mask.png", cleaned);
           //  // cv::imwrite("dots_only.png", result);
           //
           // cv::imshow("Binary Inverted", binary);
           // cv::waitKey(0);
           // cv::imshow("Cleaned Mask", cleaned);
           // cv::waitKey(0);
            //    cv::imshow("Final Result", result);
            //    cv::waitKey(0);
            //cv::destroyAllWindows();
            cap.release();
        }
        std::cout << "Change photo" << std::endl;
        std::cout << "Type 'Done' to continue the work" << std::endl;
        std::cout << "Type 'End' to stop the work" << std::endl;
        keyPressed = false;
        while (!keyPressed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return 0;
}
