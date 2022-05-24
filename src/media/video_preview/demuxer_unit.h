#ifndef MEDIA_VIDEO_RREVIEW_DEMUXER_UNIT_H
#define MEDIA_VIDEO_RREVIEW_DEMUXER_UNIT_H

#include <memory>
#include <string>

#include "base/base_type.h"
#include "net/io_channel.h"
#include "media/base/video_decoder_config.h"
#include "media/base/audio_decoder_config.h"

//  Note by lixiaonan
// ��ǰʹ�õ�FFmpegDemuxer�࣬��Ϲ���Ĳ���״̬��ҵ���߼����Լ��������̵߳���������
// ���µ�Ҫʵ����Ƶ֡Ԥ������ʱ���޷�����FFmpegDemuxer���ṩ������
// �������DemuxerUnit�࣬Ŀ�����ṩdemuxer��صĻ������ܽӿڣ��ڲ����������š������߳�
// ��ص�����
namespace media {
class DemuxerUnit {
public:
  DemuxerUnit(const std::string& source_url);
  bool Initialize();
  std::shared_ptr<EncodedAVFrame> ReadAVFrame();
  std::shared_ptr<EncodedAVFrame> ReadVideoAVFrame();
  std::shared_ptr<EncodedAVFrame> ReadAudioAVFrame();
  void Seek(int64_t timestamp_ms);
  VideoDecoderConfig GetVideoConfig();
  void ClearBuffer();
private:
  std::shared_ptr<EncodedAVFrame> ReadAVFrameByType(AVMediaType type);
  bool CreateAVIOContext();
  bool CreateVideoCodecConfig();
  bool CreateAudioCodecConfig();
  AVStream* GetAVStreamByType(AVMediaType type);
 // ffmpeg callback function
 static int FFmpegReadPacketCB(void* opaque,
                               unsigned char* buffer,
                               int buffer_size);
 static int64_t FFmpegSeekCB(void* opaque, int64_t offset, int whence);
 int FileRead(unsigned char* buffer, int buffer_size);
 long long FileSeek(long long offset, int whence);
private:
  std::string source_url_;
  // key: stream index
  // value: stream type(video/audio/subtext)
  std::map<int, int> stream_index_info;
  std::unique_ptr<net::IOChannel> data_source_;
  unsigned char* av_io_buffer_;
  AVIOContext* av_io_context_;
  AVFormatContext* av_format_context_;
  VideoDecoderConfig video_decoder_config_;
  AudioDecoderConfig audio_decoder_config_;
};
} // namespace media
#endif