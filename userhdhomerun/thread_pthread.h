#ifndef _thread_pthread_h_
#define _thread_pthread_h_

#include <pthread.h>

class ThreadPthread
{
public:
  ThreadPthread();

  int start();
  void stop();
  bool running();
  bool isFinished();

protected:
  virtual void run() = 0;

protected:
  bool m_stop;

private:
  static void* threadEntryFunc(void* _ptr);

private:
  bool m_running;
  pthread_t m_thread; 
};

#endif // _thread_pthread_h_
