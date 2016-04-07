
#ifndef VERTRY_OPENCV_CAM_PORT_H_
#define VERTRY_OPENCV_CAM_PORT_H_

#include "percipio_camport.h"
#include <opencv2/core/core.hpp>

namespace percipio {

class OCVCamPort: public DepthCameraDevice {
 public:
  OCVCamPort(HardwareModel model = percipio::ARGUS01) : DepthCameraDevice(model) {
  }
  ~OCVCamPort(void) {
  }

  //this function won't block thread and will immdiately return
  //return :CAMSTATUS_SUCCESS if successfully got a frame
  //        CAMSTATUS_NODATA if current has no valid image(may try later)
  CameraSourceStatus GetFrame(int cam_id, cv::Mat &image) {
    CameraSourceStatus ret = DepthCameraDevice::GetFrame(cam_id, &pimage);
    if (ret == CAMSTATUS_SUCCESS) {
      CopyBuffer(&pimage, image);
    }
    return ret;
  }

 private:
  OCVCamPort(const OCVCamPort&) { }

  void CopyBuffer(ImageBuffer *pbuf, cv::Mat &img) {
    switch (pbuf->type) {
    case ImageBuffer::PIX_8C1:
      img.create(pbuf->height, pbuf->width, CV_8UC1);
      break;
    case ImageBuffer::PIX_16C1:
      img.create(pbuf->height, pbuf->width, CV_16UC1);
      break;
    case ImageBuffer::PIX_8C3:
      img.create(pbuf->height, pbuf->width, CV_8UC3);
      break;
    case ImageBuffer::PIX_32FC3:
      img.create(pbuf->height, pbuf->width, CV_32FC3);
      break;
    default:
      img.release();
      return;
      break;
    }
    memcpy(img.data, pbuf->data, pbuf->width * pbuf->height * pbuf->get_pixel_size());
  }

  ImageBuffer pimage;
};

}//namespace percipio

#endif



