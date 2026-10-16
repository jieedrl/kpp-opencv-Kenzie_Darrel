#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::Mat image = cv::imread("../image_n_video/photos/3-object.jpeg");
    cv::resize(image, image, cv::Size(640,480));

    // variabel hlsImage
    cv::Mat hlsImage;
    // function convert color
    cv::cvtColor(image, hlsImage, cv::COLOR_BGR2HLS);
    
    // variabel thresholdedImage
    cv::Mat thresholdedImage,red1,red2;
    
    // kartu merah
    cv::inRange(hlsImage, cv::Scalar(0,90,50), cv::Scalar(8,190,255), red1);
    cv::inRange(hlsImage, cv::Scalar(165,90,50), cv::Scalar(179,190,255), red2);
    cv::bitwise_or(red1,red2,thresholdedImage); 
    // variabel morphologi

    cv::Mat morphology = thresholdedImage.clone();
    // function morph erode and dilate
    cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),4);
    cv::erode(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);
    // cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2)

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

    cv::Moments m = cv::moments(thresholdedImage, true);
    cv::Point p(m.m10 / m.m00, m.m01 / m.m00);
    std::string point_text = cv::format("[%d %d]", p.x, p.y);

    cv::putText(
        result,
        point_text,
        cv::Point(p.x, p.y),
        cv::FONT_HERSHEY_SIMPLEX, 0.4,
        cv::Scalar(0,255,0),
        2
    );

    // perbedaan
    cv::imshow("hls", hlsImage);
    cv::imshow("thresholded", thresholdedImage);
    cv::imshow("morph", morphology);
    cv::imshow("Bounding Box Result", result);
    cv::waitKey(0);
    return 0;
}