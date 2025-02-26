#ifndef PLAYER_MEDIA_PLAYER_CLIENT_H
#define PLAYER_MEDIA_PLAYER_CLIENT_H
#include "base/base_type.h"
#include "media/base/video_frame.h"

namespace mediakit{
class MediaPlayerClient{
public:
  virtual void OnGetMediaInfo(const media::MediaInfo& media_info) = 0;
  virtual void OnPlayProgressUpdate(const int64_t timestamp) =0;
  virtual void OnPlayStateChanged() = 0;
  virtual void OnOpenMediaFileFailed(const std::string file_name,
    int error_code,
    const std::string& error_description) = 0;
  virtual void OnGetKeyVideoFrame(
      int timestamp_ms,
      std::shared_ptr<media::VideoFrame> video_frame) = 0;
};
} // namespace mediakit
#endif