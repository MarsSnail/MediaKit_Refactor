#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H
#include <memory>
#include "player/media_player_client.h"
// �����#error:  WinSock.h has already been included
// Windows.hͷ�ļ�����Windows.hͷ�ļ��ְ�����WinSock.h����϶���
// 1���ڰ���Windows.hǰ�����WIN32_LEAN_AND_MEAN����OK�ˣ�WIN32_LEAN_AND_MEAN��ʾ������һЩ����ʹ�ú�ƫ�ŵ����ϣ�
// 2���ڰ���Windows.hǰ����winsock2.h
// 3���ڰ���Windows.hǰ����asio.hpp
#include "ui/progress_window.h"
#include <windows.h>

namespace mediakit {

LRESULT CALLBACK MainWindowProc(HWND, UINT, WPARAM, LPARAM);
class MainWindow : public MediaPlayerClient {
public:
  MainWindow();
  ~MainWindow() = default;
  HWND GetWindowHandle();
  void SetProgressWindowTop();

  // MediaPlayerClient impl
  void OnGetMediaInfo(const media::MediaInfo& media_info) override;
  void OnPlayStateChanged() override {};
  void OnPlayProgressUpdate(const int64_t timestamp) override;
private:
  ATOM RegisterWindowClass(HINSTANCE hInstance);
private:
  HWND hwnd_;
  media::MediaInfo media_info_;
  std::unique_ptr<ProgressWindow> progress_window_;
};
} // namespace mediakit

#endif