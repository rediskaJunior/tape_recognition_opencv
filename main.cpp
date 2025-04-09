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


void printCoordinates(const cv::Mat& binaryMask) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    //find contours of the white regions (dots on the tape)
    cv::findContours(binaryMask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    //show num of dots
    std::cout << "Found " << contours.size() << " dots on the photo:" << std::endl;

    for (size_t i = 0; i < contours.size(); ++i) {
        //compute the center of the dot
        cv::Moments m = cv::moments(contours[i]);
        if (m.m00 > 0) {
            //if center (num of white pixels in it) is > 0
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);
            std::cout << "Dot " << i + 1 << ": (" << cy << ", " << cx << ")" << std::endl;
        }
    }
}

int main() {
    std::thread inputThread(waitForDone); //background thread for terminal input
    inputThread.detach();

    std::cout << "Start of the work. Initializing the camera" << std::endl;

    while (!workIsDone) {
        if (keyPressed) {
            keyPressed = false;
            cv::VideoCapture cap(1);
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

            cv::Mat binary;
            cv::threshold(gray, binary, 200, 255, cv::THRESH_BINARY_INV);

            //morphology to clean up small noise (if dots are noisy or broken)
            cv::Mat cleaned;
            cv::morphologyEx(binary, cleaned, cv::MORPH_OPEN,
                             cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

            //print for further debug
            printCoordinates(cleaned);
            //apply mask to original image (now is not working as it should)
            // cv::Mat result;
            // frame.copyTo(result, cleaned);
            //from 0 to 10 -- our dots on the tape


            cv::imwrite("dots_mask.png", cleaned);
            // cv::imwrite("dots_only.png", result);

           cv::imshow("Binary Inverted", binary);
           cv::waitKey(0);
           cv::imshow("Cleaned Mask", cleaned);
           cv::waitKey(0);
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
