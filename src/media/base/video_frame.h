#ifndef MEDIA_BASE_VIDEO_FRAME_H
#define MEDIA_BASE_VIDEO_FRAME_H

#include <stdint.h>
#include <iostream>
#include "log/log_wrapper.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"
}

namespace media {
class VideoFrame {
 public:
     class TimeRecoder{
     public:
        void LogTimeRecorder() {
          LOGGING(LOG_LEVEL_INFO) << "FFmpegVideoDecoder::Decode; FrameNo:"
            << _frameNo << " decodeExpendTime:" << _decodeExpendTime
            << " addQueueTime:" << _addQueueTime << " pts:" << _pst;
        }
     public:
         int _frameNo;
         int _decodeExpendTime;
         int _addQueueTime;
         int64_t _pst;
         int64_t _popupTime;
         std::wstring _renderResult;
     };

  enum Format {
    UNKNOWN,
    YUV,
    RGB,
    RGBA,
    FORMAT_MAX,
  };

  VideoFrame(int w, int h, Format format);
  ~VideoFrame();

  unsigned char* begin() const { return _data; }

  static int frame_count_base_;
private:
  int GetNextFrameNo();

public:
  int frame_no_;
  unsigned int _w;
  unsigned int _h;
  // RGBA���ݸ�ʽ�£�ÿһ�е��ַ��������㷽ʽ��W*ÿ�����ص���ɫλ�������� RGBA��4��RGB��3��
  int stride_;
  Format _format;
  unsigned char* _data;
  unsigned char* _yuvData[3];
  unsigned char* _yuv;
  int _yuvStride[3];
  int _yuvLineCnt[3];
  int64_t _pts;
  int64_t timestamp_;
  TimeRecoder _timeRecoder;
};
}  // namespace media

#endif