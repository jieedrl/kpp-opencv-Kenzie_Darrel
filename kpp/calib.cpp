#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap("http://10.92.213.219:8080/video");

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);

    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open camera!" << std::endl;
        return -1;
    }

    std::cout << "Press C to capture | Q to quit" << std::endl;

    cv::Mat frame, gray;
    
    cv::Size chessboard(8, 6);
    float chess_size = 0.023f;

    std::vector<cv::Point3f> objp;
    std::vector<std::vector<cv::Point3f>> obj_points;
    std::vector<std::vector<cv::Point2f>> img_points;
    
    for (int i = 0; i < chessboard.height; i++) {
        for (int j = 0; j < chessboard.width; j++) {
            objp.push_back(cv::Point3f(j*chess_size, i*chess_size,0.0f));
        }
    }
    
    cv::Size img_size;

    int64 t_prev = cv::getTickCount();
    double fps = 0.0;

    while (true) {
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "Frame kamera kosong\n";
            break;
        }

        cv::resize(frame, frame, cv::Size(960,540));

        int64 t_now = cv::getTickCount();
        double elapsed_sec = (t_now-t_prev) / cv::getTickFrequency();
        t_prev = t_now;
        if (elapsed_sec > 0) fps = 1.0 / elapsed_sec;

        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));

        cv::putText(
            frame, 
            fps_text,
            cv::Point(800, 20),
            cv::FONT_HERSHEY_SIMPLEX, 0.7,
            cv::Scalar(0,255,0),
            2
        );

        img_size = frame.size();
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corners;
        bool found = findChessboardCorners(gray, chessboard, corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK);

        if (found) {
            cv::cornerSubPix(
                gray,
                corners,
                cv::Size(11, 11),
                cv::Size(-1, -1),
                cv::TermCriteria(
                    cv::TermCriteria::EPS |
                    cv::TermCriteria::COUNT,
                    30,
                    0.001
                )
            );
            
            cv::drawChessboardCorners(frame, chessboard, corners, found);

            cv::putText(
                frame,
                "Chessboard detected - Press C",
                cv::Point(20, 30),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                cv::Scalar(0, 255, 0),
                2
            );
        } else {
            cv::putText(
                frame,
                "Chessboard not detected",
                cv::Point(20, 30),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                cv::Scalar(0, 0, 255),
                2
            );
        }

        cv::putText(
            frame,
            "Samples: " + std::to_string(obj_points.size()),
            cv::Point(20, 60),
            cv::FONT_HERSHEY_SIMPLEX,
            0.7,
            cv::Scalar(255, 255, 0),
            2
        );

        cv::imshow("Camera Calibration", frame);

        char key = cv::waitKey(10);

        if (key == 'c') {
            if (found) {
                obj_points.push_back(objp);
                img_points.push_back(corners);

                std::cout << "Sampel berhasil diambil: " << obj_points.size() << '\n';
            } else std::cout << "Chessboard belum terdeteksi!\n";
        }

        if (key == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    
    if (obj_points.empty()) {
        std::cerr << "Error: Tidak ada chessboard yang berhasil dideteksi.\n";
        return 1;
    }
    
    cv::Mat K;
    cv::Mat D;

    std::vector<cv::Mat> rvecs;
    std::vector<cv::Mat> tvecs;

    double ret = cv::calibrateCamera(
        obj_points,
        img_points,
        img_size,
        K,
        D,
        rvecs,
        tvecs
    );

    std::cout << "\nCamera matrix K:\n" << K << '\n';
    std::cout << "\nDistortion D:\n" << D.t() << '\n';
    std::cout << "\nReprojection error: " << ret << '\n';

    cv::FileStorage fs(
        "../yaml/calibration.yml",
        cv::FileStorage::WRITE
    );

    if (!fs.isOpened()) {
        std::cerr << "Error: Gagal menyimpan calibration.yml\n";
        return 1;
    }

    fs << "K" << K;
    fs << "D" << D;

    fs.release();

    std::cout << "\nKalibrasi berhasil disimpan ke calibration.yml\n";

    return 0;
}