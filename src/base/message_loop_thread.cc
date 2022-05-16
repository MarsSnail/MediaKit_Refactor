#include "base/message_loop_thread.h"
#include "base_type.h"

namespace media {
MessageLoop::MessageLoop(MKThreadId mk_thread_id):pending_task_count_(0) {
  mk_thread_id_ = mk_thread_id;
  task_runner_.reset(new boost::asio::io_service());
  // 1. ���߳���Ϣ���У��������̣߳����߳���ϵͳ��������ʱĬ�ϴ�����
  //    ��˲���Ҫ����������߳�
  // 2. ��Ϣѭ��ʵ�֣���Winƽ̨�ϻ���Windows������Ϣѭ������ʵ�֣�
  //    ��˲���Ҫboost::asio::io_service::work��ȷ�����̲߳��˳�
  if(mk_thread_id == TID_MAIN) {
    return;
  }

  io_service_work_.reset(
      new boost::asio::io_service::work(*task_runner_.get()));
  boost::function<void(void)> thread_main_func;
  if(mk_thread_id == TID_DEMUXER) {
    thread_main_func = boost::bind(&MessageLoop::DemuxerThreadMain, this);
  } else if(mk_thread_id == TID_DECODE) {
    thread_main_func = boost::bind(&MessageLoop::DecodeThreadMain, this);
  } else if(mk_thread_id == TID_WORK) {
    thread_main_func = boost::bind(&MessageLoop::DecodeThreadMain, this);
  } else {
    BOOST_ASSERT_MSG(0, "unsupported thread id");
  }
  message_loop_thread_.reset(new boost::thread(thread_main_func));
}
unsigned int MessageLoop::GetTID() {
  return thread_id_;
}

void MessageLoop::RunNextTask() {
  task_runner_->run();
}

MKThreadId MessageLoop::GetThreadType() {
  return mk_thread_id_;
}

void MessageLoop::PostTask(AsyncTask async_task,
  bool out_task_run_statistics_info, const std::string& post_from) {
  CallBackFunc callback = boost::bind(&MessageLoop::OnTaskRunCompleted, this);
  AsyncTask pending_post_task =
      FunctionWrap(async_task, post_from, out_task_run_statistics_info, callback);
  pending_task_count_++;
  task_runner_->post(pending_post_task);
}

void MessageLoop::Stop() {
  task_runner_->stop();
}

void MessageLoop::DemuxerThreadMain() {
  task_runner_->run();
}

void MessageLoop::DecodeThreadMain() {
  task_runner_->run();
}

void MessageLoop::WorkThreadMain() {
  task_runner_->run();
}

void MessageLoop::OnTaskRunCompleted() {
  pending_task_count_--;
}

} //  namespace media