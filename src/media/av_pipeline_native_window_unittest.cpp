#if 1
#include "boost/smart_ptr/scoped_ptr.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/function.hpp"
#include "boost/thread.hpp"
#include <stdio.h>

extern "C" {
#include "GL/glew.h"
}
#include "GL/glut.h"

#include "media/av_pipeline.h"
#include "media/demuxer/ffmpeg_demuxer.h"
#include "media/renderer/renderer.h"
#include "media/renderer/audio_renderer.h"
#include "media/renderer/video_renderer.h"
#include "media/decoder/audio_decoder.h"
#include "media/decoder/video_decoder.h"
#include "media/renderer/video_renderer_impl.h"
#include "media/renderer/audio_renderer_impl.h"
#include "media/decoder/ffmpeg_video_decoder.h"
#include "media/decoder/ffmpeg_audio_decoder.h"
#include "net/url.h"
#include "net/io_channel.h"
#include "log/log_wrapper.h"
#include "log/watch_movie_state_recoder.h"
#include "base/message_loop_thread_manager.h"
#include "av_pipeline_factory.h"
#include "media/renderer/yuv_render.h"
#include "base/gl_context_win.h"

std::unique_ptr<media::AVPipeline> av_pipeline = nullptr;
std::queue<std::shared_ptr<media::VideoFrame> > queue_video_frame;
boost::mutex queue_mutex;
media::WatchMovieStateRecoder file_view_record;
std::string movie_name;
media::YuvRender yuv_render;
int winW = 80, winH = 60;
int videoW, videoH;

void display();

void GlobalPaintCallBack(std::shared_ptr<media::VideoFrame> video_frame) {
  boost::mutex::scoped_lock lock(queue_mutex);
  queue_video_frame.push(video_frame);
}

std::shared_ptr<media::VideoFrame> GlobalReadVideoFrame() {
  boost::mutex::scoped_lock lock(queue_mutex);
  std::shared_ptr<media::VideoFrame> video_frame;
  if (queue_video_frame.empty())
    return video_frame;
  video_frame = queue_video_frame.front();
  queue_video_frame.pop();
  return video_frame;
}

void ExitApp(){
  LOGGING(media::LOG_LEVEL_INFO) << "Playback Time:" << av_pipeline->GetPlaybackTime();
  file_view_record.RecordFilmViewProgress(movie_name, av_pipeline->GetPlaybackTime());
  file_view_record.WriteToLocalFile();
  exit(0);
}

void CreatePipeline(const char* video_url) {
  file_view_record.Initialize();
  int64_t x1_timestamp = file_view_record.GetFilmViewProgress("x1");
  file_view_record.RecordFilmViewProgress("x2", 10304104);
  file_view_record.WriteToLocalFile();

  movie_name = media::GetMovieNameUtf8(video_url);
  av_pipeline = MakeAVPipeLine(video_url, boost::bind(GlobalPaintCallBack, _1));
  av_pipeline->Start();
}

void display(void) {
  std::shared_ptr<media::VideoFrame> videoFrame = GlobalReadVideoFrame();
  if (!videoFrame.get()) {
    return;
  }
  yuv_render.updateTexture(videoFrame);
  yuv_render.render(winW, winH, videoFrame->_w, videoFrame->_h);
}

#define MAX_LOADSTRING 100

// ȫ�ֱ���:
HINSTANCE hInst;                      // ��ǰʵ��
WCHAR szTitle[MAX_LOADSTRING] = L"mediakit";        // �������ı�
WCHAR szWindowClass[MAX_LOADSTRING] = L"mediakit";  // ����������
media::GLContextWin* gl_context = nullptr;

// �˴���ģ���а����ĺ�����ǰ������:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // TODO: �ڴ˴����ô��롣

  // ��ʼ��ȫ���ַ���
  // LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  // LoadStringW(hInstance, IDC_STUDYOPENGL, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // ִ��Ӧ�ó����ʼ��:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }
  CreatePipeline(media::UTF16toANSI(lpCmdLine).c_str());
  HACCEL hAccelTable =
      LoadAccelerators(hInstance, MAKEINTRESOURCE(109));

  MSG msg;

  // ����Ϣѭ��:
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}

//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(109));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(109);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(108));

  return RegisterClassExW(&wcex);
}

//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  hInst = hInstance;  // ��ʵ������洢��ȫ�ֱ�����
  HWND hWnd =
      CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                    0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd) {
    return FALSE;
  }

  SetWindowLong(
      hWnd, GWL_EXSTYLE,
      GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  gl_context = new media::GLContextWin(hWnd, GetWindowDC(hWnd));
  yuv_render.Init();
  yuv_render.SetViewport(winW, winH);
  return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND  - ����Ӧ�ó���˵�
//  WM_PAINT    - ����������
//  WM_DESTROY  - �����˳���Ϣ������
//
//
float g = 0.0f;
bool flag = true;
LRESULT CALLBACK WndProc(HWND hWnd,
                         UINT message,
                         WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
    case WM_COMMAND: {
      int wmId = LOWORD(wParam);
      // �����˵�ѡ��:
    } break;
    case WM_PAINT: {
      display();
    } break;
    case WM_SIZE: {
      RECT rects;
      ::GetWindowRect(hWnd, &rects);
      winW = rects.right - rects.left;
      winH = rects.bottom - rects.top;
      yuv_render.SetViewport(winW, winH);
    } break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
    case WM_INITDIALOG:
      return (INT_PTR)TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
      }
      break;
  }
  return (INT_PTR)FALSE;
}
#endif