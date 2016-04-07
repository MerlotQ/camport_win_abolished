// camport_test.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "percipio_opencv_camport.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <time.h>
#include "depth_render.h"



int _tmain(int argc, _TCHAR* argv[]) {
  percipio::OCVCamPort port(percipio::PROTO_01GN04);
  DepthRender render;
  render.range_mode = DepthRender::COLOR_RANGE_DYNAMIC;
  render.color_type = DepthRender::COLORTYPE_GRAY;
  render.invalid_label = 0;
  render.Init();
  percipio::SetLogLevel(percipio::LOG_LEVEL_INFO);
  auto ret = port.OpenDevice();
  if (percipio::CAMSTATUS_SUCCESS != ret) {
    printf("open device failed\n");
    system("pause");
    return -1;
  }

  //int reti = port.SetWorkMode(percipio::WORKMODE_IR);
  int reti = port.SetWorkMode(percipio::WORKMODE_DEPTH);
  //int reti = port.SetWorkMode(percipio::WORKMODE_IR_DEP);
  if (reti < 0) {
    printf("set mode failed,error code:%d\n", reti);
    return -1;
  }
  //port.SetDepthResolution(percipio::RESO_MODE_160x120);
  //port.SetDepthResolution(percipio::RESO_MODE_320x240);
  //port.SetDepthResolution(percipio::RESO_MODE_640x480);
  //port.SetDepthResolution(percipio::RESO_MODE_1280x960);
  port.SetLaser(0);
  cv::Mat m(100, 100, CV_8UC1);
  m.setTo(0xff);
  cv::imshow("a", m);

  cv::Mat a, b, d;
  bool flg = false;
  int idx = 0;
  cv::imshow("a", cv::Mat(3, 3, CV_8UC1));
  unsigned char t = 0;
  auto tm = clock();
  auto counter = 0;
  while (true) {
    while (port.NextFrame() != percipio::CAMSTATUS_SUCCESS) {
      auto q = cv::waitKey(1); //wait until data arrived
      if (q != -1) {
        t = q;
      }
      if (t == 'q')goto exit_lb;
    }
    ret = port.GetFrame(percipio::CAMID_LEFT, a);
    if (percipio::CAMSTATUS_SUCCESS == ret) {
      cv::imshow("a", a);
      if (a.data[a.size().area() - 1] != 0xa5){
        printf(".");
      }
    }
    ret = port.GetFrame(percipio::CAMID_RIGHT, b);
    if (percipio::CAMSTATUS_SUCCESS == ret) {
      cv::imshow("b", b);
      if (b.data[a.size().area() - 1] != 0xa5){
        printf(".");
      }
    }
    
    ret = port.GetFrame(percipio::CAMID_DEPTH, d);
    if (percipio::CAMSTATUS_SUCCESS == ret) {
      auto v = d.ptr<unsigned short>(d.rows / 2)[d.cols / 2];
      cv::Mat t;
      render.Compute(d, t);
      cv::imshow("d", t);
      counter++;
      if (counter == 20){
        auto elapse = clock() - tm;
        float t = 20;
        t = t / elapse;
        t = t * CLOCKS_PER_SEC;
        printf("fps:%d distance: %d\n", (int)t, v);
        tm = clock();
        counter = 0;
      }
    }
    if (t == 's' && !a.empty()) {
      char buff[100];
      sprintf(buff, "%d-left.png", idx);
      cv::imwrite(buff, a);
    }
    if (t == 's' && !a.empty()) {
      char buff[100];
      sprintf(buff, "%d-right.png", idx);
      cv::imwrite(buff, b);
    }
    if (t == 's' && !d.empty()) {
      std::ofstream ofs("img.txt");
      ofs << d;
      ofs.close();
    }
    idx++;
    t = 0;
  }
exit_lb:
  port.CloseDevice();

  return 0;
}

