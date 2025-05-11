//
// Created by rediskajunior on 5/11/25.
//

#ifndef RECOGNITION_HPP
#define RECOGNITION_HPP

class RecognitionAlgorithm {
public:
    void analyzeCirclePattern(const std::vector<cv::Vec3f>& circles, const cv::Mat& image) {
    if (circles.empty()) {
        std::cout << "No circles detected to analyze." << std::endl;
        return;
    } //There is no need to analyze blank image

    // I: Investigate the approximate row/column structure
    int minY = image.rows;
    int maxY = 0;
    for (const auto& circle : circles) {
        int cy = static_cast<int>(circle[1]);
        minY = std::min(minY, cy);
        maxY = std::max(maxY, cy);
    }
    int tapeHeight = maxY - minY;

    // II: Create histogram of Y-coordinates to find rows (for rows histogram is working good)
    const int HISTOGRAM_BINS = 50;
    std::vector<int> yHistogram(HISTOGRAM_BINS, 0);

    double binSize = static_cast<double>(tapeHeight) / HISTOGRAM_BINS;

    for (const auto& circle : circles) {
        int cy = static_cast<int>(circle[1]);
        int binIdx = static_cast<int>((cy - minY) / binSize);
        if (binIdx >= 0 && binIdx < HISTOGRAM_BINS) {
            yHistogram[binIdx]++;
        }
    }

#ifdef DEBUG
    std::cout << "Y-coordinate histogram:" << std::endl;
    for (int i = 0; i < HISTOGRAM_BINS; i++) {
        std::cout << "Bin " << i << " (y=" << (minY + i*binSize) << "-"
                 << (minY + (i+1)*binSize) << "): " << yHistogram[i] << std::endl;
    }
#endif

    std::vector<int> rowCenters;
    const int PEAK_THRESHOLD = 2; //Minimum to consider it a peak (hardcoded for lab environment)

    for (int i = 1; i < HISTOGRAM_BINS - 1; i++) {
        if (yHistogram[i] > PEAK_THRESHOLD &&
            yHistogram[i] >= yHistogram[i-1] &&
            yHistogram[i] >= yHistogram[i+1]) {
            int rowCenter = static_cast<int>(minY + (i + 0.5) * binSize);
            rowCenters.push_back(rowCenter);
        }
    }

    //Author considers that you have tape with 9 rows (after all, you can change EXPECTED_ROWS value for your needs)

    if (rowCenters.size() < 5 || rowCenters.size() > 10) {
        const int EXPECTED_ROWS = 9;

        std::vector<int> allYCoords;
        for (const auto& circle : circles) {
            allYCoords.push_back(static_cast<int>(circle[1]));
        }

        std::sort(allYCoords.begin(), allYCoords.end());

        //divide the range into equal segments
        rowCenters.clear();
        for (int i = 0; i < EXPECTED_ROWS; i++) {
            int rowCenter = minY + (i + 0.5) * (tapeHeight / EXPECTED_ROWS);
            rowCenters.push_back(rowCenter);
        }

        bool centersChanged = true;
        const int MAX_ITERATIONS = 10;
        int iterations = 0;

        while (centersChanged && iterations++ < MAX_ITERATIONS) {
            std::vector<int> newCenters(EXPECTED_ROWS, 0);
            std::vector<int> counts(EXPECTED_ROWS, 0);
            centersChanged = false;

            for (int y : allYCoords) {
                int closestRow = 0;
                int minDist = std::abs(y - rowCenters[0]);

                for (int i = 1; i < EXPECTED_ROWS; i++) {
                    int dist = std::abs(y - rowCenters[i]);
                    if (dist < minDist) {
                        minDist = dist;
                        closestRow = i;
                    }
                }
                newCenters[closestRow] += y;
                counts[closestRow]++;
            }
            for (int i = 0; i < EXPECTED_ROWS; i++) {
                if (counts[i] > 0) {
                    int newCenter = newCenters[i] / counts[i];
                    if (std::abs(newCenter - rowCenters[i]) > 2) { // 2 pixel threshold (if you have another dots)
                        centersChanged = true;
                    }
                    rowCenters[i] = newCenter;
                }
            }
        }
    }

    std::sort(rowCenters.begin(), rowCenters.end());

#ifdef DEBUG
    std::cout << "Detected " << rowCenters.size() << " rows:" << std::endl;
    for (size_t i = 0; i < rowCenters.size(); i++) {
        std::cout << "Row " << i << " center at y=" << rowCenters[i] << std::endl;
    }
#endif

    //Author considers that you have tape with 25 rows (after all, you can change EXPECTED_COLUMNS value for your needs)

    // III: Find circles by X-coordinate to find columns
    int minX = image.cols;
    int maxX = 0;
    for (const auto& circle : circles) {
        int cx = static_cast<int>(circle[0]);
        minX = std::min(minX, cx);
        maxX = std::max(maxX, cx);
    }

    std::vector<int> columnCenters;
    const int X_PEAK_THRESHOLD = 2; // Same as for rows

    const int EXPECTED_COLUMNS = 25; // Set based on your image (now hardcoded)
    std::vector<int> allXCoords;
    for (const auto& circle : circles) {
        allXCoords.push_back(static_cast<int>(circle[0]));
    }

    columnCenters.clear();

    for (int i = 0; i < EXPECTED_COLUMNS; i++) {
        int colCenter = minX + (i + 0.5) * (maxX - minX) / EXPECTED_COLUMNS;
        columnCenters.push_back(colCenter);
    }

    const int MAX_ITERATIONS = 10;
    bool centersChanged = true;
    int iterations = 0;

    while (centersChanged && iterations++ < MAX_ITERATIONS) {
        std::vector<int> newCenters(EXPECTED_COLUMNS, 0);
        std::vector<int> counts(EXPECTED_COLUMNS, 0);
        centersChanged = false;

        for (int x : allXCoords) {
            int closestIdx = 0;
            int minDist = std::abs(x - columnCenters[0]);

            for (int i = 1; i < EXPECTED_COLUMNS; i++) {
                int dist = std::abs(x - columnCenters[i]);
                if (dist < minDist) {
                    minDist = dist;
                    closestIdx = i;
                }
            }
            newCenters[closestIdx] += x;
            counts[closestIdx]++;
        }

        for (int i = 0; i < EXPECTED_COLUMNS; i++) {
            if (counts[i] > 0) {
                int newCenter = newCenters[i] / counts[i];
                if (std::abs(newCenter - columnCenters[i]) > 2) { // pixel threshold
                    centersChanged = true;
                }
                columnCenters[i] = newCenter;
            }
        }
    }

    std::sort(columnCenters.begin(), columnCenters.end());

#ifdef DEBUG
    std::cout << "Detected " << columnCenters.size() << " columns (via K-means):" << std::endl;
    for (size_t i = 0; i < columnCenters.size(); i++) {
        std::cout << "Column " << i << " center at x=" << columnCenters[i] << std::endl;
    }
#endif

    // IV: "Move" each circle to its closest row and column
    std::vector<std::vector<bool>> bitMatrix(rowCenters.size(), std::vector<bool>(columnCenters.size(), false));

    for (const auto& circle : circles) {
        int cx = static_cast<int>(circle[0]);
        int cy = static_cast<int>(circle[1]);

        int closestRowIdx = 0;
        int minRowDist = std::abs(cy - rowCenters[0]);

        for (size_t i = 1; i < rowCenters.size(); i++) {
            int dist = std::abs(cy - rowCenters[i]);
            if (dist < minRowDist) {
                minRowDist = dist;
                closestRowIdx = i;
            }
        }

        int closestColIdx = 0;
        int minColDist = std::abs(cx - columnCenters[0]);

        for (size_t i = 1; i < columnCenters.size(); i++) {
            int dist = std::abs(cx - columnCenters[i]);
            if (dist < minColDist) {
                minColDist = dist;
                closestColIdx = i;
            }
        }

        if (closestRowIdx >= 0 && closestRowIdx < bitMatrix.size() &&
            closestColIdx >= 0 && closestColIdx < bitMatrix[0].size()) {
            bitMatrix[closestRowIdx][closestColIdx] = true;
        }
    }

    // V: Calculate if it is 0 or 1 (bit pressence)
    std::cout << "\nBit pattern (1 = dot present, 0 = no dot):\n" << std::endl;

    for (size_t col = 0; col < columnCenters.size(); col++) {
        std::cout << "Column " << col + 1 << ": ";
        for (size_t row = 0; row < rowCenters.size(); row++) {
            std::cout << (bitMatrix[row][col] ? "1 " : "0 ");
        }
        std::cout << std::endl;
    }

    // VI: Visualize the grid (only for comfortable debugging)
    cv::Mat gridImage = image.clone();
    cv::cvtColor(gridImage, gridImage, cv::COLOR_GRAY2BGR);

    for (int y : rowCenters) {
        cv::line(gridImage, cv::Point(0, y), cv::Point(image.cols - 1, y), cv::Scalar(0, 255, 0), 1);
    }

    for (int x : columnCenters) {
        cv::line(gridImage, cv::Point(x, 0), cv::Point(x, image.rows - 1), cv::Scalar(0, 255, 0), 1);
    }

    for (const auto& circle : circles) {
        int cx = static_cast<int>(circle[0]);
        int cy = static_cast<int>(circle[1]);
        int radius = static_cast<int>(circle[2]);

        int closestRowIdx = 0;
        int minRowDist = std::abs(cy - rowCenters[0]);
        for (size_t i = 1; i < rowCenters.size(); i++) {
            int dist = std::abs(cy - rowCenters[i]);
            if (dist < minRowDist) {
                minRowDist = dist;
                closestRowIdx = i;
            }
        }

        int closestColIdx = 0;
        int minColDist = std::abs(cx - columnCenters[0]);
        for (size_t i = 1; i < columnCenters.size(); i++) {
            int dist = std::abs(cx - columnCenters[i]);
            if (dist < minColDist) {
                minColDist = dist;
                closestColIdx = i;
            }
        }

        cv::circle(gridImage, cv::Point(cx, cy), radius, cv::Scalar(0, 255, 0), 2);

#ifdef DEBUG
        // Draw row and column indices for reference (optional)
        // cv::putText(gridImage, "r" + std::to_string(closestRowIdx) + ",c" + std::to_string(closestColIdx),
        //            cv::Point(cx - 20, cy - 10), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 255), 1);
#endif
    }
    cv::imwrite("tape_grid_analysis.jpg", gridImage);

    cv::imshow("Tape Grid Analysis", gridImage);
    cv::waitKey(0);
}

//mostly of this function has been written for debugging
void printCoordinatesHough(const cv::Mat& grayImage){
    cv::Mat processedImage = grayImage.clone();// Create a copy to avoid modifying the original

    cv::bitwise_not(processedImage, processedImage);// First invert the image since dots are black on white

    cv::Mat blurred;// Apply blur to reduce noise
    cv::GaussianBlur(processedImage, blurred, cv::Size(5, 5), 1.5);

    cv::Mat binary;// Apply adaptive thresholding to handle uneven lighting
    cv::adaptiveThreshold(blurred, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                         cv::THRESH_BINARY, 11, 2);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));// Clean up using morphological operations
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

#ifdef DEBUG
    cv::Mat visualImage = grayImage.clone();
    cv::cvtColor(visualImage, visualImage, cv::COLOR_GRAY2BGR);
#endif

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(binary, circles, cv::HOUGH_GRADIENT, 1,
                     10,  // min distance between circles
                     50,  // param1: higher threshold for Canny edge detector
                     10,  // param2: accumulator threshold
                     1,   // min radius
                     10   // max radius
    );

    #ifdef DEBUG
    std::cout << "Found " << circles.size() << " circles on the photo:" << std::endl;

    for (size_t i = 0; i < circles.size(); i++) {
        int cx = static_cast<int>(circles[i][0]);
        int cy = static_cast<int>(circles[i][1]);
        int radius = static_cast<int>(circles[i][2]);
        std::cout << "Circle " << i + 1 << ": (" << cy << ", " << cx << "), Radius: " << radius << std::endl;

        cv::circle(visualImage, cv::Point(cx, cy), 2, cv::Scalar(0, 255, 0), -1);
        cv::circle(visualImage, cv::Point(cx, cy), radius, cv::Scalar(0, 0, 255), 1);
    }

    cv::imshow("Original", grayImage);
    cv::imshow("Binary", binary);
    cv::imshow("Detected Circles", visualImage);
    #endif

    analyzeCirclePattern(circles, grayImage);

    cv::waitKey(0);
    cv::destroyAllWindows();
}
};

#endif //RECOGNITION_HPP