#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::FileStorage fs("../yaml/calibration.yml", cv::FileStorage::READ);
    cv::Mat K;
    fs["K"] >> K;
    fs.release();

    double fx = K.at<double>(0,0);
    double fy = K.at<double>(1,1);
    double cx = K.at<double>(0,2);
    double cy = K.at<double>(1,2);

    cv::VideoCapture cap("http://10.95.94.134:8080/video");
    cv::Mat frame;

    int64 t_prev = cv::getTickCount();
    double fps = 0.0;

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
        cv::cvtColor(resized, hlsframe, cv::COLOR_BGR2HLS);
        
        cv::Mat red, red1, red2, blue, green;
        // untuk red 
        cv::inRange(hlsframe, cv::Scalar(0,90,50), cv::Scalar(8,190,255), red1);
        cv::inRange(hlsframe, cv::Scalar(165,90,50), cv::Scalar(179,190,255), red2);
        red = red1 | red2;

        // untuk green paper
        cv::inRange(hlsframe, cv::Scalar(72,130,80), cv::Scalar(90,195,180), green);

        // untuk blue eraser
        cv::inRange(hlsframe, cv::Scalar(100,80,100),cv::Scalar(118,190,255), blue);

        std::vector<cv::Mat> color = {red, green, blue};
        std::vector<std::string> label = {"red card", "green paper", "blue eraser"};
        std::vector<double> w_reals = {21.0, 20.8, 11.5};
        
        cv::Mat result = resized.clone();
        for (int c = 0; c < 3; c++) {
            cv::Mat morphology = color[c].clone();
            cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);
            cv::erode(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(morphology,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);

            for (size_t i = 0; i < contours.size(); i++) {
                if (cv::contourArea(contours[i]) > 2000) {
                    cv::Rect boundingBox = cv::boundingRect(contours[i]);
                    cv::Point tl = boundingBox.tl();
                    cv::rectangle(result,boundingBox,cv::Scalar(255,0,0),2);
                    double w_pixel = boundingBox.width;

                    cv::Moments m = cv::moments(contours[i], true);
                    cv::Point p(m.m10 / m.m00, m.m01 / m.m00);

                    double z = fx*w_reals[c]/w_pixel;
                    double x = (p.x-cx)*z/fx;
                    double y = (p.y-cy)*z/fy;

                    std::string label_text = label[c];
                    std::string koordinat = cv::format("(%.2lf, %.2lf, %.2lf)", x, y, z);
                    std::string jarak_z = cv::format("Jarak Z = %.2lf cm", z);

                    std::string output = label_text + " " + koordinat;

                    cv::putText(
                        result,
                        output, 
                        tl,
                        cv::FONT_HERSHEY_SIMPLEX,0.7,
                        cv::Scalar(0,255,255),
                        2
                    );

                    cv::putText(
                        result,
                        jarak_z,
                        cv::Point(tl.x, tl.y+28),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        cv::Scalar(0,255,255),
                        2
                    );
                }
            }
        }

        if (result.empty()) break;   

        cv::putText(
            result, 
            fps_text,
            cv::Point(1180, 20),
            cv::FONT_HERSHEY_SIMPLEX, 0.75,
            cv::Scalar(0,255,0),
            2
        );

        cv::imshow("Card", result);
        if (cv::waitKey(25) == 'q') break;
    }
}