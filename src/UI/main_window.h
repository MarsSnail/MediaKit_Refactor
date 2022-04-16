#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H
#include <memory>
#include "player/media_player_client.h"
// �����#error:  WinSock.h has already been included
// Windows.hͷ�ļ�����Windows.hͷ�ļ��ְ�����WinSock.h����϶���
// 1���ڰ���Windows.hǰ�����WIN32_LEAN_AND_MEAN����OK�ˣ�WIN32_LEAN_AND_MEAN��ʾ������һЩ����ʹ�ú�ƫ�ŵ����ϣ�
// 2���ڰ���Windows.hǰ����winsock2.h
// 3���ڰ���Windows.hǰ����asio.hpp
#include <windows.h>
#include "ui/progress_window.h"
#include "player/media_player.h"

namespace mediakit {
class MediaPlayer;
class ProgressWindow;

LRESULT CALLBACK MainWindowProc(HWND, UINT, WPARAM, LPARAM);
class MainWindow : public MediaPlayerClient {
public:
  MainWindow();
  ~MainWindow() = default;
  HWND GetWindowHandle();
  void Seek(int timestamp_ms);
  void SetProgressWindowTop();
  void OnWindowSizeChanged();
  void OnMouseMove();
  void SetMediaPlayer(std::shared_ptr<MediaPlayer> mediaplayer);
  // MediaPlayerClient impl
  void OnGetMediaInfo(const media::MediaInfo& media_info) override;
  void OnPlayStateChanged() override {};
  void OnPlayProgressUpdate(const int64_t timestamp) override;
  void OnPlayPauseButtionClick();
private:
  ATOM RegisterWindowClass(HINSTANCE hInstance);
  void CreatePlayingTimeTextControl(int left, int top, int w, int h);
  std::wstring  FormatDurationInfo(int media_duration);
  void UpdatePlaytingShowText();
  void CreatePlayPauseButtion(int left, int top, int w, int h);
private:
  bool is_pauseing_;
  HWND hwnd_;
  HWND playing_time_hwnd_;
  HWND play_pause_hwnd_;
  std::wstring duration_format_str_;
  std::wstring current_playing_timestamp_str_;
  media::MediaInfo media_info_;
  std::unique_ptr<ProgressWindow> progress_window_;
  int pre_playing_timestamp_by_second_;
  std::shared_ptr<MediaPlayer> mediaplayer_instance_;
};
} // namespace mediakit

#endif