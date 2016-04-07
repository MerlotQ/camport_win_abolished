//
// Percipio All Rights Reserved 2016
//

#ifndef PERCIPIO_CAMPORT_H_
#define PERCIPIO_CAMPORT_H_
#include <stdint.h>
#include <fstream>

#ifdef _WIN32

#ifdef PERCIPIO_API_EXPORTS
#define PERCIPIO_API __declspec(dllexport)
#else
#define PERCIPIO_API __declspec(dllimport)
#endif

#else

#define PERCIPIO_API __attribute__ ((visibility ("default"))) 

#endif

namespace percipio {

enum HardwareModel {
  ARGUS01 = 1,
  PROTO_03GN03 = 2,
  PROTO_01GN04 = 3,
};


enum CameraFrameDataTypes {
  CAMDATA_LEFT  = 0x00,
  CAMDATA_RIGHT = 0x01,
  CAMDATA_COLOR = 0x02,
  CAMDATA_DEPTH = 0x03,
  CAMDATA_POINT3D = 0x80
};

enum CameraSourceStatus {
  CAMSTATUS_SUCCESS = 0,
  CAMSTATUS_ERROR = -1,
  CAMSTATUS_PARAM_INVALID = -2,
  CAMSTATUS_NODATA = -3,
  CAMSTATUS_NOTSUPPORT = -4,
  CAMSTATUS_NOTCONNECTED = -5,
};

struct Vect3f {
  float x, y, z;
};

struct ImageBuffer {
  enum PixelTypes {
    PIX_8C1  = 0,
    PIX_8C3  = 1,
    PIX_16C1 = 2,
    PIX_32C1 = 3,
    PIX_32FC3 = 4,
  };

  int width;
  int height;
  PixelTypes type;
  unsigned char *data;

  ImageBuffer() {
    width = height = 0;
    data = 0;
  }

  template <class T>
  T* ptr(int row) {
    unsigned char *t = data + get_pixel_size() * width * row;
    return (T*)t;
  }

  int get_pixel_size() const {
    switch (type) {
    case percipio::ImageBuffer::PIX_8C1:
      return 1;
      break;
    case percipio::ImageBuffer::PIX_8C3:
      return 3;
      break;
    case percipio::ImageBuffer::PIX_16C1:
      return 2;
      break;
    case percipio::ImageBuffer::PIX_32C1:
      return 4;
      break;
    case percipio::ImageBuffer::PIX_32FC3:
      return 4 * 3;
      break;
    default:
      return -1;
      break;
    }
  }
};

enum DeviceProperties {
  PROP_DEVICE_INFO = 0x00,
  PROP_DEVICE_ADDR,
  PROP_CMOS_REG,
  PROP_LASER_POW,
  PROP_WORKMODE,
  PROP_DEPTH_RESOLUTION,
  PROP_CALIB_DEPTH_INTRISTIC,
  PROP_POINT_CLOUD_OUTPUT,
  PROP_FRAME_READY_CALLBACK,
  PROP_CALLBACK_USER_DATA,
  PROP_WAIT_NEXTFRAME_TIMEOUT,
};

typedef enum {
  RESO_MODE_160x120 = 0x00,
  RESO_MODE_320x240 = 0x01,
  RESO_MODE_640x480 = 0x02,
  RESO_MODE_1280x960 = 0xff,
} ResolutionModes;

typedef enum {
  WORKMODE_IDLE     = 0,
  WORKMODE_IR       = 1,
  WORKMODE_DEPTH    = 2,
  WORKMODE_IR_DEPTH = 3,
  WORKMODE_RGB      = 4,
  WORKMODE_RGBD     = 6,
  WORKMODE_TEST     = 8,
} DeviceWorkModes;

typedef enum {
  LOG_LEVEL_VERBOSE = 10,
  LOG_LEVEL_INFO = 20,
  LOG_LEVEL_WARN = 30,
  LOG_LEVEL_ERROR = 40,
  LOG_LEVEL_NONE  = 0xffff
} LogLevels;

struct CmosCtrlParam {
  int32_t reg_addr;
  int32_t reg_value;
  int8_t cam_id;
};

struct DeviceInfo {
  int32_t hardware_model;
  uint16_t major;
  uint16_t minor;
  uint8_t  sn[32];
  int8_t  str_name[64];
};

struct CamIntristic {
  uint32_t width;
  uint32_t height;
  float data[9];
};

typedef void(*EventCallbackFunc)(void *user_data);

//device abi interface
class ICameraVideoSource {
 public:
  virtual ~ICameraVideoSource() {}
  virtual CameraSourceStatus OpenDevice() = 0;
  virtual void CloseDevice() = 0;
  virtual CameraSourceStatus Config(const char *data) = 0;
  virtual CameraSourceStatus GetFrame(int cam_data_type, ImageBuffer *buff1) = 0;
  virtual CameraSourceStatus NextFrame() = 0;
  //return negative value for error status , reutrn positive value for the actual data size
  virtual int GetProperty(int prop_id, char * data_buff, int size) = 0;
  //return negative value for error status , reutrn positive value for the actual data size
  virtual int SetProperty(int prop_id, const char * data, int size) = 0;
};

//create camera data source
PERCIPIO_API ICameraVideoSource* CreateSource(HardwareModel model);
PERCIPIO_API void ReleaseSource(ICameraVideoSource* ptr);

/**
* \brief log display level */
PERCIPIO_API  void SetLogLevel(int level);
PERCIPIO_API  int LibVersion();


//depth cam device
class DepthCameraDevice {
 public:
  DepthCameraDevice(HardwareModel model = ARGUS01) {
    _source = CreateSource(model);
  }
  ~DepthCameraDevice() {
    ReleaseSource(_source);
  }

  ICameraVideoSource * get_source() const {
    return _source;
  }

  CameraSourceStatus OpenDevice() {
    return get_source()->OpenDevice();
  }
  void CloseDevice() {
    get_source()->CloseDevice();
  }

  void LoadConfig(const char *data) {
    get_source()->Config(data);
  }

  void LoadConfigFile(const char *filename) {
    std::ifstream ifs(filename);
    if (ifs) {
      std::string str((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
      get_source()->Config(str.c_str());
    }
  }

  CameraSourceStatus NextFrame() {
    return get_source()->NextFrame();
  }

  CameraSourceStatus GetFrame(int cam_id, ImageBuffer *buff) {
    return get_source()->GetFrame(cam_id, buff);
  }

  int SetCmosRegister(int camid, int regid, int value) {
    CmosCtrlParam param;
    param.cam_id = camid;
    param.reg_addr = regid;
    param.reg_value = value;
    return get_source()->SetProperty(PROP_CMOS_REG, (char*)&param, sizeof(param));
  }

  int SetLaser(unsigned char duty) {
    return get_source()->SetProperty(PROP_LASER_POW, (char*)&duty, sizeof(duty));
  }

  int SetDepthResolution(ResolutionModes reso) {
    return get_source()->SetProperty(PROP_DEPTH_RESOLUTION, (char*)&reso, sizeof(reso));
  }

  //change work status
  int SetWorkMode(DeviceWorkModes mode) {
    return get_source()->SetProperty(percipio::PROP_WORKMODE, (char*)&mode, sizeof(mode));
  }

  //compute Point cloud for each frame, this require WORKMODE_DEPTH or WORKMODE_IR_DEPTH.
  int SetPointCloudOutput(bool enable) {
    return get_source()->SetProperty(percipio::PROP_POINT_CLOUD_OUTPUT, (char*)&enable, sizeof(enable));
  }

  int SetFrameReadyCallback(EventCallbackFunc callback) {
    return get_source()->SetProperty(percipio::PROP_FRAME_READY_CALLBACK, (char*)callback, sizeof(callback));
  }

  int SetCallbackUserData(void* data) {
    return get_source()->SetProperty(percipio::PROP_CALLBACK_USER_DATA, (char*)data, sizeof(data));
  }

  //set max blocking time when call NextFrame.
  //for non-blocking NextFrame call, set value to zero or negative num.
  int SetWaitNextFrameTimeout(int timeout_ms){
    return get_source()->SetProperty(percipio::PROP_WAIT_NEXTFRAME_TIMEOUT, (char*)timeout_ms, sizeof(timeout_ms));
  }

 private:
  ICameraVideoSource * _source;
};


}//namespace percipio

#endif
