
#ifndef VERTRY_OPENCV_CAM_PORT_H_
#define VERTRY_OPENCV_CAM_PORT_H_

#include "percipio_camport.h"

namespace percipio {

class OCVCamPort: public DepthCameraDevice {
 public:
  OCVCamPort(HardwareModel model = percipio::PROTO_01GN04) : DepthCameraDevice(model) {
  }
  ~OCVCamPort(void) {
  }

 private:
  OCVCamPort(const OCVCamPort&);
  ImageBuffer pimage;
};

}//namespace percipio

#endif



