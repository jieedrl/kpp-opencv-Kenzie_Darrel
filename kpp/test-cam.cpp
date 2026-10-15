#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // variabel
    cv::VideoCapture cap("http://10.144.179.93:8080/video");
    cv::Mat frame;

    // perulangan 
    while (cap.isOpened()) {
        cap >> frame;
        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(1280,960));

        cv::Mat hlsframe;
        // function convert color
        cv::cvtColor(resized, hlsframe, cv::COLOR_BGR2HLS);
        
        // variabel thresholdedframe
        cv::Mat thresholdedframe;
        // function thresholding
        cv::inRange(hlsframe, cv::Scalar(0,166,179), cv::Scalar(4,211,255), thresholdedframe);

        // variabel morphologi
        cv::Mat morphology = thresholdedframe.clone();
        // function morph erode and dilate
        cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);
        cv::erode(morphology,morphology,cv::Mat(), cv::Point(-1,-1),1);
        cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);

        // variabel vector contour
        std::vector<std::vector<cv::Point>> contours;
        // function find contour
        cv::findContours(morphology,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
        // variabel hasil
        cv::Mat result = resized.clone();

        for (size_t i = 0; i < contours.size(); i++) {
            if (cv::contourArea(contours[i]) > 3000) {
                cv::Rect boundingBox = cv::boundingRect(contours[i]);
                cv::rectangle(result,boundingBox,cv::Scalar(0,255,0),2);
            }
        }

        if (result.empty()) break;   

        // show
        cv::imshow("Card", thresholdedframe);
        if (cv::waitKey(25) == 'q') break;
    }
}