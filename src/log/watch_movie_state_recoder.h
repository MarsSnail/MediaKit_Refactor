#ifndef LOG_FILE_VIEW_RECODER_H
#define LOG_FILE_VIEW_RECODER_H
//  �־û��洢��Ӱ��¼��ÿ�δ�ͬһӰƬ���Զ������ϴβ���ʱ�䣬��������
//  1. �ַ������ʽΪ��utf8
//  2. ������֯��ʽΪ��json
//  3. json���ݸ�ʽ���£�
//  {
//       "ƽ���" : 10, 
//       "�����ζ�Ƭ" : 10,
//   }

#include <string>
#include "json/json.h"

namespace media {
class WatchMovieStateRecoder{
public:
  WatchMovieStateRecoder();
  void Initialize();
  int64_t GetFilmViewProgress(const std::string& film_name_utf8_format);
  void RecordFilmViewProgress(const std::string& file_name_utf8_format,
    int64_t timestamp);
  void WriteToLocalFile();
private:
  bool initialized;
  std::string record_info_file_path_;
  Json::Value record_info_root_;
};
} // namespace media
#endif //#ifndef LOG_FILE_VIEW_RECODER_H
