#include "thread_pthread.h"

#include <unistd.h>

ThreadPthread::ThreadPthread() 
: m_running(false), 
  m_stop(false)
{
}

void* ThreadPthread::threadEntryFunc(void* _ptr)
{
  ThreadPthread* thread = static_cast<ThreadPthread *>(_ptr);
  thread->m_running = true;
  thread->run();
  pthread_detach(thread->m_thread);  // Remove "leak" from valgrind.
  thread->m_running = false;
  return NULL;
}

int ThreadPthread::start()
{
    return pthread_create(&m_thread, NULL, &ThreadPthread::threadEntryFunc, this);
}

void ThreadPthread::stop()
{
  m_stop = true;
  pre_stop();
  while(m_running)
  {
    usleep(100000);
  }
}

bool ThreadPthread::isFinished() const
{
  return !m_running;
}
