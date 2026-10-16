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

    cv::VideoCapture cap("http://10.83.226.244:8080/video");
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
        
        cv::Mat red,red1,red2,gray,blue;
        // deteksi merah
        cv::inRange(hlsframe, cv::Scalar(0,50,40), cv::Scalar(10,245,255), red1);
        cv::inRange(hlsframe, cv::Scalar(175,50,40), cv::Scalar(179,245,255), red2); 
        red = red1 | red2;
        // deteksi gray
        cv::inRange(hlsframe, cv::Scalar(0, 30, 0), cv::Scalar(179, 150, 60), gray);
        // deteksi biru
        cv::inRange(hlsframe, cv::Scalar(95,70,100), cv::Scalar(125,220,255), blue);

        std::vector<cv::Mat> color = {gray, red, blue};
        std::vector<std::string> label = {"mouse abu", "kartu merah", "penghapus biru"};
        std::vector<double> w_reals = {9.5, 6.0, 17.0};
        // gray, Merah, biru
        std::vector<cv::Scalar> box_colors = {cv::Scalar(0,165,255), cv::Scalar(0,0,255), cv::Scalar(0,255,255)};

        cv::Mat result = resized.clone();
        for (int c = 0; c < 3; c++) {
            cv::Mat morphology = color[c].clone();
            // function morph erode and dilate
            cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),7);
            cv::erode(morphology,morphology,cv::Mat(), cv::Point(-1,-1),4);
            cv::dilate(morphology,morphology,cv::Mat(), cv::Point(-1,-1),2);

            std::vector<std::vector<cv::Point>> contours;
            // function find contour
            cv::findContours(morphology,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);

            for (size_t i = 0; i < contours.size(); i++) {
                if (cv::contourArea(contours[i]) > 2000) {
                    cv::Rect boundingBox = cv::boundingRect(contours[i]);
                    cv::Point tl = boundingBox.tl();
                    cv::rectangle(result,boundingBox,box_colors[c],2);
                    double w_pixel = boundingBox.width;

                    cv::Moments m = cv::moments(contours[i], true);
                    cv::Point p(m.m10 / m.m00, m.m01 / m.m00);

                    double z = fx*w_reals[c]/w_pixel;
                    double x = (p.x-cx)*z/fx;
                    double y = (p.y-cy)*z/fy;

                    std::string label_text = label[c];
                    std::string koordinat = cv::format("[%lf %lf %lf]", x, y, z);

                    std::string output = label_text + " " + koordinat;

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

        cv::imshow("Card", result);
        if (cv::waitKey(25) == 'q') break;
    }
}