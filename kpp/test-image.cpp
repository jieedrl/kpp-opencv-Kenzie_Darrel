#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::Mat image = cv::imread("../image_n_video/photos/card.jpeg");
    cv::resize(image, image, cv::Size(640,480));

    // variabel hsvImage
    cv::Mat hsvImage;
    // function convert color
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);
    
    // variabel thresholdedImage
    cv::Mat thresholdedImage,red1,red2;
    // function thresholding
    cv::inRange(hsvImage, cv::Scalar(0,130,150), cv::Scalar(4,255,255), red1);
        cv::inRange(hsvImage, cv::Scalar(175,130,150), cv::Scalar(179,255,255), red2);
        cv::bitwise_or(red1,red2,thresholdedImage); 
    // variabel morphologi
    cv::Mat morphology = thresholdedImage.clone();
    // function morph erode and dilate
    cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);
    cv::erode(morphology,morphology,cv::Mat(), cv::Point(-1,-1),1);
    cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);

    // variabel vector contour
    std::vector<std::vector<cv::Point>> contours;
    // function find contour
    cv::findContours(morphology,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    // variabel hasil
    cv::Mat result = image.clone();

    for (size_t i = 0; i < contours.size(); i++) {
        if (cv::contourArea(contours[i]) > 3000) {
            cv::Rect boundingBox = cv::boundingRect(contours[i]);
            cv::rectangle(result,boundingBox,cv::Scalar(0,255,0),2);
        }
    }

    // perbedaan
    cv::imshow("hls", hsvImage);
    cv::imshow("thresholded", thresholdedImage);
    cv::imshow("morph", morphology);
    cv::imshow("Bounding Box Result", result);
    cv::waitKey(0);
    return 0;
}