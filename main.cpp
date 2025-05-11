//Includes with all necessary libraries for this project (DO NOT DELETE)
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <thread>
#include <atomic>

#include "recognition.hpp" //there is an algorithm with recognition (DO NOT DELETE)

std::atomic<bool> keyPressed(true);
std::atomic<bool> workIsDone(false);

void waitForDone() {
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "Done" || input == "done" || input == "d" || input == "D") {
            keyPressed = true;
        } else if (input == "End" || input == "end" || input == "e" || input == "E") {
            std::cout << "Program ended" << std::endl;
            workIsDone = true;
            keyPressed = true;
        } else {
            std::cout << "Try again, please. 'd' or 'e'" << std::endl;
        }
    }
}

int main() {

    #ifdef DEBUG
    std::cout << "Debug mode is activated." << std::endl;
    #endif

    std::thread inputThread(waitForDone); //background thread for communication with user
    inputThread.detach();

    std::cout << "----------Start of the work. Initializing the camera.----------" << std::endl;

    while (!workIsDone) {
        if (keyPressed) {
            keyPressed = false;
            cv::VideoCapture cap(4); //index for the camera can be found with "ls /dev/video*"
            if (!cap.isOpened()) {
                std::cerr << "!!Error: Could not open camera!!" << std::endl;
                return -1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            cv::Mat photo;
            bool success = cap.read(photo);
            if (!success) {
                std::cerr << "!!Error: Could not read from camera!!" << std::endl;
                return -1;
            }

        #ifdef DEBUG
            bool saved = cv::imwrite("img.jpg", photo);
            if (!saved) {
                std::cerr << "!!Error: Could not save image.!!" << std::endl;
                return -1;
            }
        #endif

            std::cout << "----------Image captured and saved successfully.----------" << std::endl;
            cap.release();

            cv::Mat gray;
            cv::cvtColor(photo, gray, cv::COLOR_BGR2GRAY);

            RecognitionAlgorithm Alg;

            Alg.printCoordinatesHough(gray); //Call function with algorithm

            cap.release();
        }
        std::cout << "----------Change photo of your tape range.----------" << std::endl;
        std::cout << "Type 'Done' or 'd' to continue the work." << std::endl;
        std::cout << "Type 'End' or 'e' to stop the work completely." << std::endl;
        keyPressed = false;
        while (!keyPressed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return 0;
}
