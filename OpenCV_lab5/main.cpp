#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/times.h>
#include <unistd.h>;

void process_frame(cv::Mat * frame) {
    cv::cvtColor(*frame, *frame, cv::COLOR_BGR2RGB);
    cv::flip(*frame, *frame, 1);
    cv::blur(*frame, *frame, cv::Size(10, 10));
}
int main() {
    struct timespec start_fps, end_fps, start_in, end_in, start_out, end_out, start_process, end_process,start_wait, end_wait;
    double time_in = 0, time_out = 0, time_process = 0, time_wait = 0;
    cv::VideoCapture camera(0);

    if (!camera.isOpened()) {
        return 0;
    }

    int Frame_Count = 0;
    clock_gettime(CLOCK_MONOTONIC, &start_fps);

    while (true) {
        cv::Mat frame;
        clock_gettime(CLOCK_MONOTONIC, &start_in);
        camera.read(frame);
        clock_gettime(CLOCK_MONOTONIC, &end_in);
        time_in += static_cast<double>(end_in.tv_sec - start_in.tv_sec) + static_cast<double>(end_in.tv_nsec - start_in.tv_nsec) / 1e9;
        if (frame.empty()) {
            break;
        }
        double fps = 0;
        clock_gettime(CLOCK_MONOTONIC, &start_process);
        process_frame(&frame);
        clock_gettime(CLOCK_MONOTONIC, &end_process);
        time_process += static_cast<double>(end_process.tv_sec - start_process.tv_sec) + static_cast<double>(end_process.tv_nsec - start_process.tv_nsec) / 1e9;
        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));
        cv::putText(frame, fps_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        clock_gettime(CLOCK_MONOTONIC, &start_out);
        cv::imshow("Camera", frame);
        clock_gettime(CLOCK_MONOTONIC, &end_out);
        time_out += static_cast<double>(end_out.tv_sec - start_out.tv_sec) + static_cast<double>(end_out.tv_nsec - start_out.tv_nsec) / 1e9;

        Frame_Count++;
        clock_gettime(CLOCK_MONOTONIC, &start_wait);
        int c = cv::waitKey(33);
        clock_gettime(CLOCK_MONOTONIC, &end_wait);
        time_wait += static_cast<double>(end_wait.tv_sec - start_wait.tv_sec) + static_cast<double>(end_wait.tv_nsec - start_wait.tv_nsec) / 1e9;

        if (c == 27) {
            break;
        }

    }

    clock_gettime(CLOCK_MONOTONIC, &end_fps);
    double time_fps = static_cast<double>(end_fps.tv_sec - start_fps.tv_sec) + static_cast<double>(end_fps.tv_nsec - start_fps.tv_nsec) / 1e9;

    std::cout << "FPS: " << Frame_Count / time_fps << std::endl;
    std::cout << "All time: " << time_fps <<"seconds"<< std::endl;
    std::cout << "Time in: " << time_in << " seconds  " <<100*time_in/time_fps<<" %"<< std::endl;
    std::cout << "Time out: " << time_out << " seconds  "<<100*time_out/time_fps<<" %" << std::endl;
    std::cout << "Time process: " << time_process << " seconds  "<<100*time_process/time_fps<<" %" << std::endl;
    std::cout << "Time wait: " << time_wait <<"seconds  " <<100*time_wait/time_fps<<" %" << std::endl;

    camera.release();
    cv::destroyWindow("Camera");
    return 0;
}
