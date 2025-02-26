#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H
#include <memory>
#include "player/media_player_client.h"
// 解决：#error:  WinSock.h has already been included
// Windows.h头文件，而Windows.h头文件又包含了WinSock.h这个老东西
// 1、在包含Windows.h前定义宏WIN32_LEAN_AND_MEAN，就OK了（WIN32_LEAN_AND_MEAN表示不包含一些极少使用和偏门的资料）
// 2、在包含Windows.h前包含winsock2.h
// 3、在包含Windows.h前包含asio.hpp
#include <windows.h>
#include "ui/progress_window.h"
#include "player/media_player.h"
#include "ui/video_preview_window.h"

#define IDM_FILE_OPEN 40001

namespace mediakit {
class MediaPlayer;
class ProgressWindow;

LRESULT CALLBACK MainWindowProc(HWND, UINT, WPARAM, LPARAM);
class MainWindow : public MediaPlayerClient {
public:
  MainWindow();
  ~MainWindow();
  //~MainWindow() = default;
  HWND GetWindowHandle();
  void Seek(int timestamp_ms);
  void SetProgressWindowTop();
  void OnWindowSizeChanged();
  void OnMouseMove();
  // @timestamp_ms:视频帧预览时间戳，时间单位：毫秒
  // @width: 预览视频帧窗口的宽度
  // @height: 预览视频帧窗口的高度
  void NotifyShowVideoPreview(int x, int y, int timestamp_ms);
  void SetMediaPlayer(std::shared_ptr<MediaPlayer> mediaplayer);

  // MediaPlayerClient impl
  void OnGetMediaInfo(const media::MediaInfo& media_info) override;
  void OnPlayStateChanged() override {};
  void OnPlayProgressUpdate(const int64_t timestamp) override;
  void OnOpenMediaFileFailed(const std::string file_name,
                             int error_code,
                             const std::string& error_description) override;
  void OnGetKeyVideoFrame(
      int timestamp_ms,
      std::shared_ptr<media::VideoFrame> video_frame) override;

  void OnPlayPauseButtionClick();
  void OnOpenNewFile();
  void OnWindowClose();
  void DestroyPreMediaPlayer(std::shared_ptr<MediaPlayer> instance);
  void HidePreviewWindow();
private:
  void ShowPreviewWindow(int x, int y);
  void StartPlayNewVideo(std::wstring file_path);
  void CreateMainMenu(HWND hwnd);
  ATOM RegisterWindowClass(HINSTANCE hInstance);
  void CreatePlayingTimeTextControl(int left, int top, int w, int h);
  std::wstring  FormatDurationInfo(int media_duration);
  void UpdatePlaytingShowText();
  void CreatePlayPauseButtion(int left, int top, int w, int h);
private:
  bool is_pauseing_;
  int pre_video_preview_timestamp_;
  HMENU h_main_menu;
  HWND hwnd_;
  HWND playing_time_hwnd_;
  HWND play_pause_hwnd_;
  std::wstring duration_format_str_;
  std::wstring current_playing_timestamp_str_;
  media::MediaInfo media_info_;
  std::unique_ptr<ProgressWindow> progress_window_;
  std::unique_ptr<VideoPreviewWindow> video_preview_window_;
  int pre_playing_timestamp_by_second_;
  std::shared_ptr<MediaPlayer> mediaplayer_instance_;
  std::string file_path_;
};
} // namespace mediakit

#endif