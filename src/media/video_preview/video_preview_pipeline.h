#ifndef MEDIA_PREVIEW_PIPELINE_H
#define MEDIA_PREVIEW_PIPELINE_H

#include "media/video_preview/demuxer_unit.h"
#include "media/video_preview/video_decoder_unit.h"

// �ṩ����ʱ�������ȡ��Ӧ��Ƶ�ؼ�֡���ݵĹ���
namespace media {

class VideoPreviewPipelineDelegate {
public:
  virtual void OnGetKeyVideoFrame(
    int timestamp_ms, std::shared_ptr<VideoFrame> video_frame) = 0;
};

class VideoPreviewPipeline {
public:
  VideoPreviewPipeline(const std::string& video_url);
  void GetKeyFrame(int64_t timestamp_ms);
  bool initialize();
  void SetDelegate(VideoPreviewPipelineDelegate* delegate);

private:
  void GetKeyFrameInternal(int64_t timestamp_ms);

private:
bool initialized_;
VideoPreviewPipelineDelegate* delegate_;
std::string video_url_;
std::unique_ptr<DemuxerUnit> demuxer_unit_;
std::unique_ptr<VideoDeocderUnit> video_decoder_unit_;
bool working_;
};
} // namespace media

#endif