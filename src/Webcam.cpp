#include "Webcam.h"

#include <string>

Webcam::Webcam(int device) : running_(false), new_frame_(false), device_(device) {
}

Webcam::~Webcam() {
    stop();
}

bool Webcam::read(cv::OutputArray& out) {
    std::lock_guard guard(frame_mutex_);
    if (new_frame_) {
        new_frame_ = false;
        frame_.copyTo(out);
        return true;
    }

    return false;
}

Error Webcam::startIf() {
    if (running_.load()) {
        return {};
    }

    webcam_.open(device_);
    if (!webcam_.isOpened()) {
        return "Unable to open capture device " + std::to_string(device_);
    }

    running_ = true;
    thread_ = std::thread([this]{
        while (running_.load()) {
            cv::Mat frame;
            if (webcam_.read(frame)) {
                frame_mutex_.lock();
                size_ = frame.size();
                frame_ = frame;
                new_frame_ = true;
                frame_mutex_.unlock();
            }
        }
    });

    return {};
}

int Webcam::getWidth() {
    return size_.width;
}

int Webcam::getHeight() {
    return size_.height;
}

void Webcam::stop() {
    running_ = false;
    thread_.join();
}
