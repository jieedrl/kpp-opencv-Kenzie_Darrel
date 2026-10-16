#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // variabel
    cv::VideoCapture cap("http://10.89.12.23:8080/video");
    cv::Mat frame;

    int64 t_prev = cv::getTickCount();
    double fps = 0.0;

    // perulangan 
    while (cap.isOpened()) {
        cap >> frame;

        cv::Mat resized;
        cv::resize(frame, resized, cv::Size(1280,960));

        int64 t_now = cv::getTickCount();
        double elapsed_sec = (t_now-t_prev) / cv::getTickFrequency();
        t_prev = t_now;
        if (elapsed_sec > 0) fps = 1.0 / elapsed_sec;

        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));

        cv::Mat hlsframe;
        // function convert color
        cv::cvtColor(resized, hlsframe, cv::COLOR_BGR2HLS);
        
        // variabel thresholdedframe
        cv::Mat thresholdedframe, red1, red2;

        // untuk red card
        cv::inRange(hlsframe, cv::Scalar(0,90,50), cv::Scalar(8,190,255), red1);
        cv::inRange(hlsframe, cv::Scalar(165,90,50), cv::Scalar(179,190,255), red2);
        thresholdedframe = red1 | red2;

        // untuk green paper
        cv::inRange(hlsframe, cv::Scalar(45,80,40), cv::Scalar(75,190,200), thresholdedframe);

        // variabel morphologi
        cv::Mat morphology = thresholdedframe.clone();
        // function morph erode and dilate
        cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),4);
        cv::erode(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);
        // cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);

        // variabel vector contour
        std::vector<std::vector<cv::Point>> contours;
        // function find contour
        cv::findContours(morphology,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
        // variabel hasil
        cv::Mat result = resized.clone();

        for (size_t i = 0; i < contours.size(); i++) {
            if (cv::contourArea(contours[i]) > 2000) {
                cv::Rect boundingBox = cv::boundingRect(contours[i]);
                cv::Point tl = boundingBox.tl();
                cv::rectangle(result,boundingBox,cv::Scalar(0,255,0),2);
                double w_pixel = boundingBox.width;

                cv::Moments m = cv::moments(thresholdedframe, true);
                cv::Point p(m.m10 / m.m00, m.m01 / m.m00);

                cv::FileStorage fs("../yaml/calibration.yml", cv::FileStorage::READ);
                cv::Mat K;
                fs["K"] >> K;
                fs.release();

                double fx = K.at<double>(0,0);
                double fy = K.at<double>(1,1);
                double cx = K.at<double>(0,2);
                double cy = K.at<double>(1,2);

                double z = fx*600.0/w_pixel;
                double x = (p.x-cx)*z/fx;
                double y = (p.y-cy)*z/fy;

                std::string output = cv::format("[%lf %lf %lf %lf]", x, y, z, w_pixel);

                cv::putText(
                    result,
                    output, 
                    tl,
                    cv::FONT_HERSHEY_SIMPLEX,0.7,
                    cv::Scalar(0,255,0),
                    2
                );
            }
        }

        if (result.empty()) break;   

        cv::putText(
            result, 
            fps_text,
            cv::Point(800, 20),
            cv::FONT_HERSHEY_SIMPLEX, 0.7,
            cv::Scalar(0,255,0),
            2
        );

        // show
        cv::imshow("Card", result);
        if (cv::waitKey(25) == 'q') break;
    }
}