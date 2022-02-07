#ifndef MEDIA_DEMUXER_DEMUXER_DELEGATE_H_
#define MEDIA_DEMUXER_DEMUXER_DELEGATE_H_

#include <stdint.h>
#include "demuxer_stream_provider.h"
#include "media/base/pipeline_status.h"

namespace media {
  class DemuxerDelegate {
  public:
    // Seek ���ڹؼ�֡�����Ƶ���Ź��̵�������ת
    // ��������seekʱ����ʵ��seekʱ�����ܴ�
    // ���Ի��ڴκ���֪ͨ��ʱ���������Ϊ���յ�seek��ʱ���
    virtual void OnUpdateAlignedSeekTimestamp(int64_t timestamp) = 0;
  };
}  // namespace media

#endif