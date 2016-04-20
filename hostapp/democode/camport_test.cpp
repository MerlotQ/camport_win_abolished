// camport_test.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include "stdafx.h"
#endif

#include "percipio_opencv_camport.h"
#include <opencv2/opencv.hpp>
#include <time.h>
#include <stdlib.h>
#include "depth_render.h"

static cv::Mat left, right, depth, point_cloud;
static DepthRender render;
static int fps_counter = 0;
static clock_t fps_tm = 0;


void process_frames(percipio::OCVCamPort &port);
void save_frame_to_file();
int get_fps();

void frame_arrived_callback(void *user_data) {
  //1. call port.NextFrame to refresh new frame buffer here
  //2. call port.GetFrame to get frame data here
  // To avoid performance problem ,time consuming task in callback function is not recommended.
}

int main(int argc, char** argv) {
  percipio::OCVCamPort port(percipio::PROTO_01GN04);
  render.range_mode = DepthRender::COLOR_RANGE_DYNAMIC;
  render.color_type = DepthRender::COLORTYPE_BLUERED;
  render.invalid_label = 0;
  render.Init();
  percipio::SetLogLevel(percipio::LOG_LEVEL_INFO);
  port.SetCallbackUserData(NULL);
  port.SetFrameReadyCallback(frame_arrived_callback);
  //port.SetWaitNextFrameTimeout(1000);
  auto ret = port.OpenDevice();
  if (percipio::CAMSTATUS_SUCCESS != ret) {
    printf("open device failed\n");
    return -1;
  }
  //set flag below to get point cloud
  //port.SetPointCloudOutput(true);

//  int reti = port.SetWorkMode(percipio::WORKMODE_IR);
  int reti = port.SetWorkMode(percipio::WORKMODE_DEPTH);
//  int reti = port.SetWorkMode(percipio::WORKMODE_IR_DEPTH);
  if (reti < 0) {
    printf("set mode failed,error code:%d\n", reti);
    return -1;
  }
//  port.SetDepthResolution(percipio::RESO_MODE_160x120);
  //port.SetDepthResolution(percipio::RESO_MODE_320x240);
  port.SetDepthResolution(percipio::RESO_MODE_640x480);
//  port.SetDepthResolution(percipio::RESO_MODE_1280x960);

  port.SetLaser(0);
  //display a empty window for receiving key input
  cv::imshow("left", cv::Mat::zeros(100, 100, CV_8UC1));
  fps_tm = clock();
  fps_counter = 0;
  while (true) {
    if (port.NextFrame() == percipio::CAMSTATUS_SUCCESS) {
      process_frames(port);
    }
    auto k = cv::waitKey(1);
    if (k == 'q') {
      break;
    }
    if (k == 's') {
      save_frame_to_file();
    }
  }//while
  port.CloseDevice();
  left.release();
  right.release();
  depth.release();
  point_cloud.release();
  render.Uninit();
  return 0;
}

void process_frames(percipio::OCVCamPort &port) {
  auto ret = port.GetFrame(percipio::CAMDATA_LEFT, left);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
    cv::imshow("left", left);
  }
  ret = port.GetFrame(percipio::CAMDATA_RIGHT, right);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
    cv::imshow("right", right);
  }
  ret = port.GetFrame(percipio::CAMDATA_DEPTH, depth);
  if (percipio::CAMSTATUS_SUCCESS == ret) {
    cv::Mat t;
    int fps = get_fps();
    render.Compute(depth, t);
    cv::imshow("depth", t);
    if (fps > 0) {
      auto v = depth.ptr<unsigned short>(depth.rows / 2)[depth.cols / 2];
      printf("fps:%d distance: %d\n", (int)fps, v);
    }
  }
  ret = port.GetFrame(percipio::CAMDATA_POINT3D, point_cloud);
}

int get_fps() {
  const int kMaxCounter = 20;
  fps_counter++;
  if (fps_counter < kMaxCounter) {
    return -1;
  }
  auto elapse = (clock() - fps_tm);
  int v = (int)(((float)fps_counter) / elapse * CLOCKS_PER_SEC);
  fps_tm = clock();
  fps_counter = 0;
  return v;
}

void save_frame_to_file() {
  static int idx = 0;
  char buff[100];
  if (!left.empty()) {
    sprintf(buff, "%d-left.png", idx);
    cv::imwrite(buff, left);
  }
  if (!right.empty()) {
    sprintf(buff, "%d-right.png", idx);
    cv::imwrite(buff, right);
  }
  if (!depth.empty()) {
    sprintf(buff, "%d-depth.txt", idx);
    std::ofstream ofs(buff);
    ofs << depth;
    ofs.close();
  }
  if (!point_cloud.empty()) {
    sprintf(buff, "%d-points.txt", idx);
    std::ofstream ofs(buff);
    auto ptr = point_cloud.ptr<percipio::Vect3f>();
    for (int i = 0; i < point_cloud.size().area(); i++) {
      ofs << ptr->x << "," << ptr->y << "," << ptr->z << std::endl;
      ptr++;
    }
    ofs.close();
  }
  idx++;
}

