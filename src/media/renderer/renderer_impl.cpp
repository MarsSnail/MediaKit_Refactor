﻿#include "renderer_impl.h"

#include "boost/bind.hpp"
#include "media/base/playback_clock.h" 
#include "log/log_wrapper.h"
#include "base/message_loop_thread_manager.h"

namespace media {

std::map<int, std::shared_ptr<TimeSource>> RendererImpl::playback_clock_map_;

std::shared_ptr<TimeSource> RendererImpl::GetPlaybackClock(int pipelineId){
  std::map<int, std::shared_ptr<TimeSource>>::iterator iter =
    playback_clock_map_.find(pipelineId);

  if (iter == playback_clock_map_.end()) {
    return NULL;
    }
  return iter->second;
}

RendererImpl::RendererImpl(std::shared_ptr<AudioRenderer> audio_renderer,
                           std::shared_ptr<VideoRenderer> video_renderer)
    : state_(STATE_UNINITIALIZED),
      playback_clock_(new PlaybackClock()),
      audio_renderer_(audio_renderer),
      video_renderer_(video_renderer),
      is_seeking_(false){
  // TODO(lixiaonan): 暂时不添加多个pipeline实例的ID管理模块，默认ID为0，后续需要添加，并修改此处代码
  playback_clock_map_.insert(std::pair<int, std::shared_ptr<TimeSource>>(0, playback_clock_));
  video_renderer_->SetDelegate(this);
}

RendererImpl::~RendererImpl(){
}

void RendererImpl::SetDelegate(RendererDelegate* delegate) {
  delegate_ = delegate;
}

void RendererImpl::Initialize(DemuxerStreamProvider* demuxer_stream_provider,
                              PipelineStatusCB init_cb,
                              PipelineStatusCB status_cb,
                              VideoRenderer::PaintCB paint_cb) {
  demuxer_stream_provider_ = demuxer_stream_provider;
  paint_cb_ = paint_cb;
  init_cb_ = init_cb;
  status_cb_ = status_cb;
  InitializeAudioRenderer();
}

void RendererImpl::StartPlayingFrom(int64_t time_offset) {
  if (state_ != STATE_NORMAL)
    return;
  playback_clock_->SetStartTime(time_offset);
  video_renderer_->StartPlayingFrom(time_offset);
  audio_renderer_->StartPlayingFrom(time_offset);
}
void RendererImpl::SetPlaybackRate(float rate) {
  video_renderer_->SetPlaybackRate(rate);
  audio_renderer_->SetPlaybackRate(rate);
  playback_clock_->SetPlaybackRate(rate);
}

void RendererImpl::SetVolume(float volume) {
}

void RendererImpl::Pause() {
  playback_clock_->Pause();
  audio_renderer_->Pause();
  video_renderer_->Pause();
  LOGGING(LOG_LEVEL_DEBUG) << "CurrentMediaTime:" << GetCurrentTime();
}

void RendererImpl::Resume() {
  LOGGING(LOG_LEVEL_DEBUG) << "CurrentMediaTime:" << GetCurrentTime();
  playback_clock_->Resume();
  audio_renderer_->Resume();
  video_renderer_->Resume();
  return;
}

void RendererImpl::ShowStateInfo() {
  LOGGING(LOG_LEVEL_DEBUG) << "CurrentMediaTime:" << GetCurrentTime();
  audio_renderer_->ShowStateInfo();
  video_renderer_->ShowStateInfo();
}

void RendererImpl::UpdateAlignSeekTimestamp(int64_t timestamp) {
  playback_clock_->Seek(timestamp);
}

void RendererImpl::Seek(int64_t timestamp_ms) {
  playback_clock_->Pause();
  audio_renderer_->ClearAVFrameBuffer();
  video_renderer_->ClearAVFrameBuffer();
  is_seeking_ = true;
  return;
}

void RendererImpl::Stop() {
  TRACEPOINT;
  video_renderer_->SetDelegate(nullptr);
  video_renderer_->Stop();
  TRACEPOINT;
  audio_renderer_->Stop();
  TRACEPOINT;
}

int64_t RendererImpl::GetPlaybackTime() {
  int64_t current_timestamp = GetCurrentTime();
  return current_timestamp;
}
void RendererImpl::InitializeAudioRenderer() {
  audio_renderer_->SetDelegate(this);
  audio_renderer_->Initialize(
      demuxer_stream_provider_->GetDemuxerStream(DemuxerStream::AUDIO),
      boost::bind(&RendererImpl::OnInitializeAudioRendererDone, this, _1),
      status_cb_, boost::bind(&RendererImpl::GetCurrentTime, this));
}
void RendererImpl::OnInitializeAudioRendererDone(PipelineStatus status) {
  // TODO(lixiaonan):
  if (status != PIPELINE_OK) {
    init_cb_(status);
    return;
  }
  std::cout << "Audio Renderer init Done" << std::endl;
  InitializeVideoRenderer();
}

void RendererImpl::InitializeVideoRenderer() {
  video_renderer_->Initialize(
      demuxer_stream_provider_->GetDemuxerStream(DemuxerStream::VIDEO),
      boost::bind(&RendererImpl::OnInitializeVideoRendererDone, this, _1),
      status_cb_, paint_cb_, boost::bind(&RendererImpl::GetCurrentTime, this));
}

void RendererImpl::OnInitializeVideoRendererDone(PipelineStatus status) {
  // TODO(lixiaonan): 增加 init audio renderer的初始化状态处理
  state_ = status == PIPELINE_OK ? STATE_NORMAL : STATE_NORMAL;
  init_cb_(status);
}
int64_t RendererImpl::GetCurrentTime() {
  int64_t current_timestamp = playback_clock_->GetCurrentMediaTime();
  return current_timestamp;
}

void RendererImpl::OnPlayClockUpdate(int64_t timestamp) {
  if (delegate_) {
    delegate_->OnPlayProgressUpdate(timestamp);
  }
}

void RendererImpl::OnGetFirstAudioFrame(int64_t pts) { 
  if (is_seeking_) {
    LOGGING(LOG_LEVEL_DEBUG) << "Seek-Timestamp(ms):" << pts << ";";
    playback_clock_->Seek(pts);
    playback_clock_->Resume();
    is_seeking_ = false;
  } else {
    playback_clock_->StartTicking();
  }
}

}  // namespace media
