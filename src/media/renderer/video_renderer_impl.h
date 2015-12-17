#ifndef MEDIA_RENDERER_VIDEO_RENDERER_IMPL_H
#define MEDIA_RENDERER_VIDEO_RENDERER_IMPL_H

#include "boost/thread/thread.hpp"
#include "boost/thread/condition.hpp"
#include "boost/scoped_ptr.hpp"

#include "media/decoder/video_frame_stream.h"
#include "media/renderer/renderer_impl.h"
#include "media/renderer/video_renderer.h"

namespace media {
class VideoRendererImpl : public VideoRenderer {
 public:
  VideoRendererImpl(
      TaskRunner* task_runner_,
      const VideoFrameStream::VecVideoDecoders& vec_video_decoders);

  // VideoRenderer impl
  virtual void Initialize(DemuxerStream* demuxer_stream,
                          PipelineStatusCB init_cb, PipelineStatusCB status_cb,
                          PaintCB paint_cb, GetTimeCB get_time_cb) override;
  virtual void StartPlayingFrom(int64_t offset) override;
  virtual void SetPlaybackRate(float rate) override;

 private:
  enum State {
    STATE_UNINITIALIZED,
    STATE_INITIALIZING,
    STATE_INITIALIZED,
    STATE_PLAYING,
  };

  enum FrameOperation {
    OPERATION_WAIT_FOR_PAINT,
    OPERATION_DROP_FRAME,
    OPERATION_PAINT_IMMEDIATELY,
  };

  void ThreadMain();
  void OnInitializeVideoFrameStreamDone(bool result);
  void InitializeAction();
  void OnReadFrameDone(VideoFrameStream::Status status,
                       std::shared_ptr<VideoFrame> video_frame);
  void ReadFrame();
  void PaintReadyVideoFrame(std::shared_ptr<VideoFrame> video_frame);
  void ReadFrameIfNeeded();
  FrameOperation DetermineNextFrameOperation(int64_t current_time,
                                             int64_t next_frame_pts);

  bool pending_paint_;
  State state_;
  int64_t start_playing_time_;
  PaintCB paint_cb_;
  GetTimeCB get_time_cb_;
  PipelineStatusCB init_cb_;
  PipelineStatusCB status_cb_;
  DemuxerStream* demuxer_stream_;
  TaskRunner* task_runner_;
  boost::mutex ready_frames_lock_;
  boost::condition frame_available_;
  std::queue<std::shared_ptr<VideoFrame> > pending_paint_frames_;
  boost::scoped_ptr<VideoFrameStream> video_frame_stream_;
  boost::scoped_ptr<boost::thread> video_frame_paint_thread_;
  int64_t droped_frame_count_;
};
}  // namespace media
#endif