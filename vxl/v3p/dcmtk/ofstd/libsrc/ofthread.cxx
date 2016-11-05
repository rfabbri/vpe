/*
 *
 *  Copyright (C) 1997-2002, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  ofstd
 *
 *  Author:  Marco Eichelberg
 *
 *  Purpose: Provides operating system independent abstractions for basic
 *           multi-thread concepts: threads, thread specific data,
 *           semaphores, mutexes and read/write locks. The implementation
 *           of these classes supports the Solaris, POSIX and Win32
 *           multi-thread APIs.
 *
 */

#include "osconfig.h"

/* if WITH_THREADS is undefined, we don't even attempt to implement a thread interface. */
#ifdef WITH_THREADS

#ifdef HAVE_WINDOWS_H
#define WINDOWS_INTERFACE
extern "C" {
#include <windows.h>
#include <process.h>
}

#elif defined(HAVE_THREAD_H) && defined(HAVE_SYNCH_H)
#define SOLARIS_INTERFACE
extern "C" {
#include <thread.h>
#include <synch.h>      /* for read/write locks, semaphores and mutexes */
}

#elif defined(HAVE_PTHREAD_H) && defined(HAVE_SEMAPHORE_H)
#define POSIX_INTERFACE
#ifndef HAVE_PTHREAD_RWLOCK
// not all POSIX pthread implementations provide read/write locks
#define POSIX_INTERFACE_WITHOUT_RWLOCK
#endif
extern "C" {
#include <pthread.h>    /* for threads, read/write locks and mutexes*/
#include <semaphore.h>  /* for semaphores */
}
#endif

#endif /* WITH_THREADS */

#define INCLUDE_CSTRING
#define INCLUDE_CERRNO
#include "ofstdinc.h"
#include "ofthread.h"
#include "ofconsol.h"
#include "ofstring.h"

/* ------------------------------------------------------------------------- */

/* Thread stub function - called by the thread creation system function.
 * Calls the run() method of the OFString class and returns.
 */


#ifdef HAVE_WINDOWS_H
unsigned int __stdcall thread_stub(void *arg)
{
  ((OFThread *)arg)->run();
  return 0; // thread terminates
}
#else
void *thread_stub(void *arg)
{
  ((OFThread *)arg)->run();
  return NULL; // thread terminates
}
#endif

/* ------------------------------------------------------------------------- */

#ifdef WINDOWS_INTERFACE
  const int OFThread::busy = -1;
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const int OFThread::busy = ESRCH;
#else
  const int OFThread::busy = -2;
#endif

OFThread::OFThread()
#ifdef WINDOWS_INTERFACE
: theThreadHandle(0)
, theThread(0)
#else
: theThread(0)
#endif
{
}

OFThread::~OFThread()
{
#ifdef WINDOWS_INTERFACE
  CloseHandle((HANDLE)theThreadHandle);
#endif
}

int OFThread::start()
{
#ifdef WINDOWS_INTERFACE
  unsigned int tid = 0;
  theThreadHandle = _beginthreadex(NULL, 0, thread_stub, (void *)this, 0, &tid);
  if (theThreadHandle == 0) return errno; else
  {
    theThread = tid;
    return 0;
  }
#elif defined(POSIX_INTERFACE)
  pthread_t tid=0;
  int result = pthread_create(&tid, NULL, thread_stub, (void *)this);
  if (0 == result) theThread = (unsigned long) tid; else theThread = 0;
  return result;
#elif defined(SOLARIS_INTERFACE)
  thread_t tid=0;
  int result = thr_create(NULL, 0, thread_stub, (void *)this, 0, &tid);
  if (0 == result) theThread = (unsigned long) tid; else theThread = 0;
  return result;
#else
  return -1;
#endif
}

int OFThread::join()
{
#ifdef WINDOWS_INTERFACE
  if (WaitForSingleObject((HANDLE)theThreadHandle, INFINITE) == WAIT_OBJECT_0) return 0;
  else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  void *retcode=NULL;
  return pthread_join((pthread_t)theThread, &retcode);
#elif defined(SOLARIS_INTERFACE)
  void *retcode=NULL;
  return thr_join((thread_t)theThread, NULL, &retcode);
#else
  return -1;
#endif
}

unsigned long OFThread::threadID()
{
  return theThread;
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
OFBool OFThread::equal(unsigned long tID)
#else
OFBool OFThread::equal(unsigned long /* tID */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  if (theThread == tID) return OFTrue; else return OFFalse;
#elif defined(POSIX_INTERFACE)
  if (pthread_equal((pthread_t)theThread, (pthread_t)tID)) return OFTrue; else return OFFalse;
#elif defined(SOLARIS_INTERFACE)
  if ((thread_t)theThread == (thread_t)tID) return OFTrue; else return OFFalse;
#else
  return 0;
#endif
}

void OFThread::thread_exit()
{
#ifdef WINDOWS_INTERFACE
  _endthreadex(0);
#elif defined(POSIX_INTERFACE)
  pthread_exit(NULL);
#elif defined(SOLARIS_INTERFACE)
  thr_exit(NULL);
#else
#endif
  return; // will never be reached
}

unsigned long OFThread::self()
{
#ifdef WINDOWS_INTERFACE
  return (unsigned long) GetCurrentThreadId();
#elif defined(POSIX_INTERFACE)
  return (unsigned long) pthread_self();
#elif defined(SOLARIS_INTERFACE)
  return (unsigned long) thr_self();
#else
  return 0;
#endif
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
void OFThread::errorstr(OFString& description, int code)
#else
void OFThread::errorstr(OFString& description, int /* code */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  if (code == OFThread::busy) description = "another thread already waiting for join"; else
  {
    LPVOID buf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, (DWORD)code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &buf, 0, NULL);
    if (buf) description = (const char *)buf;
    LocalFree(buf);
  }
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const char *str = strerror(code);
  if (str) description = str; else description.clear();
#else
  description = "error: threads not implemented";
#endif
  return;
}

/* ------------------------------------------------------------------------- */


OFThreadSpecificData::OFThreadSpecificData()
: theKey(NULL)
{
#ifdef WINDOWS_INTERFACE
  DWORD *key = new DWORD;
  if (key)
  {
    *key = TlsAlloc();
    if (*key == (DWORD) -1) delete key;
    else theKey=key;
  }
#elif defined(POSIX_INTERFACE)
  pthread_key_t *key = new pthread_key_t;
  if (key)
  {
    if (pthread_key_create(key, NULL)) delete key;
    else theKey=key;
  }
#elif defined(SOLARIS_INTERFACE)
  thread_key_t *key = new thread_key_t;
  if (key)
  {
    if (thr_keycreate(key, NULL)) delete key;
    else theKey=key;
  }
#else
#endif
}

OFThreadSpecificData::~OFThreadSpecificData()
{
#ifdef WINDOWS_INTERFACE
  if (theKey) TlsFree(*((DWORD *)theKey));
#elif defined(POSIX_INTERFACE)
  delete (pthread_key_t *)theKey;
#elif defined(SOLARIS_INTERFACE)
  delete (thread_key_t *)theKey;
#else
#endif
}

OFBool OFThreadSpecificData::initialized() const
{
#ifdef WITH_THREADS
  if (theKey) return OFTrue; else return OFFalse;
#else
  return OFFalse; // thread specific data is not supported if we are working in a single-thread environment
#endif
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
int OFThreadSpecificData::set(void *value)
#else
int OFThreadSpecificData::set(void * /* value */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  if (theKey)
  {
    if (0 == TlsSetValue(*((DWORD *)theKey), value)) return (int)GetLastError(); else return 0;
  } else return ERROR_INVALID_HANDLE;
#elif defined(POSIX_INTERFACE)
  if (theKey) return pthread_setspecific(*((pthread_key_t *)theKey), value); else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theKey) return thr_setspecific(*((thread_key_t *)theKey), value); else return EINVAL;
#else
  return -1;
#endif
}

int OFThreadSpecificData::get(void *&value)
{
#ifdef WINDOWS_INTERFACE
  if (theKey)
  {
    value = TlsGetValue(*((DWORD *)theKey));
    return 0;
  } else {
    value = NULL;
    return ERROR_INVALID_HANDLE;
  }
#elif defined(POSIX_INTERFACE)
  if (theKey)
  {
    value = pthread_getspecific(*((pthread_key_t *)theKey));
    return 0;
  } else {
    value = NULL;
    return EINVAL;
  }
#elif defined(SOLARIS_INTERFACE)
  value = NULL;
  if (theKey) return thr_getspecific(*((thread_key_t *)theKey), &value); else return EINVAL;
#else
  value = NULL;
  return -1;
#endif
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
void OFThreadSpecificData::errorstr(OFString& description, int code)
#else
void OFThreadSpecificData::errorstr(OFString& description, int /* code */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  LPVOID buf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, (DWORD)code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &buf, 0, NULL);
  if (buf) description = (const char *)buf;
  LocalFree(buf);
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const char *str = strerror(code);
  if (str) description = str; else description.clear();
#else
  description = "error: thread specific data not implemented";
#endif
  return;
}

/* ------------------------------------------------------------------------- */


#ifdef WINDOWS_INTERFACE
  const int OFSemaphore::busy = -1;
#elif defined(POSIX_INTERFACE)
  const int OFSemaphore::busy = EAGAIN;  // Posix returns EAGAIN instead of EBUSY in trywait.
#elif defined(SOLARIS_INTERFACE)
  const int OFSemaphore::busy = EBUSY;
#else
  const int OFSemaphore::busy = -2;
#endif


#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
OFSemaphore::OFSemaphore(unsigned int numResources)
#else
OFSemaphore::OFSemaphore(unsigned int /* numResources */ )
#endif
: theSemaphore(NULL)
{
#ifdef WINDOWS_INTERFACE
  theSemaphore = (void *)(CreateSemaphore(NULL, numResources, numResources, NULL));
#elif defined(POSIX_INTERFACE)
  sem_t *sem = new sem_t;
  if (sem)
  {
    if (sem_init(sem, 0, numResources) == -1) delete sem;
    else theSemaphore = sem;
  }
#elif defined(SOLARIS_INTERFACE)
  sema_t *sem = new sema_t;
  if (sem)
  {
    if (sema_init(sem, numResources, USYNC_THREAD, NULL)) delete sem;
    else theSemaphore = sem;
  }
#else
#endif
}

OFSemaphore::~OFSemaphore()
{
#ifdef WINDOWS_INTERFACE
  CloseHandle((HANDLE)theSemaphore);
#elif defined(POSIX_INTERFACE)
  if (theSemaphore) sem_destroy((sem_t *)theSemaphore);
  delete (sem_t *)theSemaphore;
#elif defined(SOLARIS_INTERFACE)
  if (theSemaphore) sema_destroy((sema_t *)theSemaphore);
  delete (sema_t *)theSemaphore;
#else
#endif
}

OFBool OFSemaphore::initialized() const
{
#ifdef WITH_THREADS
  if (theSemaphore) return OFTrue; else return OFFalse;
#else
  return OFTrue;
#endif
}

int OFSemaphore::wait()
{
#ifdef WINDOWS_INTERFACE
  if (WaitForSingleObject((HANDLE)theSemaphore, INFINITE) == WAIT_OBJECT_0) return 0;
  else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  if (theSemaphore)
  {
    if (sem_wait((sem_t *)theSemaphore)) return errno; else return 0;
  } else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theSemaphore) return sema_wait((sema_t *)theSemaphore); else return EINVAL;
#else
  return -1;
#endif
}

int OFSemaphore::trywait()
{
#ifdef WINDOWS_INTERFACE
  DWORD result = WaitForSingleObject((HANDLE)theSemaphore, 0);
  if (result == WAIT_OBJECT_0) return 0;
  else if (result == WAIT_TIMEOUT) return OFSemaphore::busy;
  else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  if (theSemaphore)
  {
    if (sem_trywait((sem_t *)theSemaphore)) return errno; else return 0; // may return EAGAIN
  } else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theSemaphore) return sema_trywait((sema_t *)theSemaphore); else return EINVAL; // may return EBUSY
#else
  return -1;
#endif
}

int OFSemaphore::post()
{
#ifdef WINDOWS_INTERFACE
  if (ReleaseSemaphore((HANDLE)theSemaphore, 1, NULL)) return 0; else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  if (theSemaphore)
  {
    if (sem_post((sem_t *)theSemaphore)) return errno; else return 0;
  } else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theSemaphore) return sema_post((sema_t *)theSemaphore); else return EINVAL;
#else
  return -1;
#endif
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
void OFSemaphore::errorstr(OFString& description, int code)
#else
void OFSemaphore::errorstr(OFString& description, int /* code */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  if (code == OFSemaphore::busy) description = "semaphore is already locked"; else
  {
    LPVOID buf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, (DWORD)code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &buf, 0, NULL);
    if (buf) description = (const char *)buf;
    LocalFree(buf);
  }
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const char *str = strerror(code);
  if (str) description = str; else description.clear();
#else
  description = "error: semaphore not implemented";
#endif
  return;
}


/* ------------------------------------------------------------------------- */


#ifdef WINDOWS_INTERFACE
  const int OFMutex::busy = -1;
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const int OFMutex::busy = EBUSY;
#else
  const int OFMutex::busy = -2;
#endif

OFMutex::OFMutex()
: theMutex(NULL)
{
#ifdef WINDOWS_INTERFACE
  theMutex = (void *)(CreateMutex(NULL, FALSE, NULL));
#elif defined(POSIX_INTERFACE)
  pthread_mutex_t *mtx = new pthread_mutex_t;
  if (mtx)
  {
    if (pthread_mutex_init(mtx, NULL)) delete mtx;
    else theMutex = mtx;
  }
#elif defined(SOLARIS_INTERFACE)
  mutex_t *mtx = new mutex_t;
  if (mtx)
  {
    if (mutex_init(mtx, USYNC_THREAD, NULL)) delete mtx;
    else theMutex = mtx;
  }
#else
#endif
}


OFMutex::~OFMutex()
{
#ifdef WINDOWS_INTERFACE
  CloseHandle((HANDLE)theMutex);
#elif defined(POSIX_INTERFACE)
  if (theMutex) pthread_mutex_destroy((pthread_mutex_t *)theMutex);
  delete (pthread_mutex_t *)theMutex;
#elif defined(SOLARIS_INTERFACE)
  if (theMutex) mutex_destroy((mutex_t *)theMutex);
  delete (mutex_t *)theMutex;
#else
#endif
}


OFBool OFMutex::initialized() const
{
#ifdef WITH_THREADS
  if (theMutex) return OFTrue; else return OFFalse;
#else
  return OFTrue;
#endif
}


int OFMutex::lock()
{
#ifdef WINDOWS_INTERFACE
  if (WaitForSingleObject((HANDLE)theMutex, INFINITE) == WAIT_OBJECT_0) return 0;
  else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  if (theMutex) return pthread_mutex_lock((pthread_mutex_t *)theMutex); else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theMutex) return mutex_lock((mutex_t *)theMutex); else return EINVAL;
#else
  return -1;
#endif
}

int OFMutex::trylock()
{
#ifdef WINDOWS_INTERFACE
  DWORD result = WaitForSingleObject((HANDLE)theMutex, 0);
  if (result == WAIT_OBJECT_0) return 0;
  else if (result == WAIT_TIMEOUT) return OFMutex::busy;
  else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  if (theMutex) return pthread_mutex_trylock((pthread_mutex_t *)theMutex); else return EINVAL; // may return EBUSY.
#elif defined(SOLARIS_INTERFACE)
  if (theMutex) return mutex_trylock((mutex_t *)theMutex); else return EINVAL; // may return EBUSY.
#else
  return -1;
#endif
}

int OFMutex::unlock()
{
#ifdef WINDOWS_INTERFACE
  if (ReleaseMutex((HANDLE)theMutex)) return 0; else return (int)GetLastError();
#elif defined(POSIX_INTERFACE)
  if (theMutex) return pthread_mutex_unlock((pthread_mutex_t *)theMutex); else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theMutex) return mutex_unlock((mutex_t *)theMutex); else return EINVAL;
#else
  return -1;
#endif
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
void OFMutex::errorstr(OFString& description, int code)
#else
void OFMutex::errorstr(OFString& description, int /* code */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  if (code == OFSemaphore::busy) description = "mutex is already locked"; else
  {
    LPVOID buf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, (DWORD)code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &buf, 0, NULL);
    if (buf) description = (const char *)buf;
    LocalFree(buf);
  }
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const char *str = strerror(code);
  if (str) description = str; else description.clear();
#else
  description = "error: mutex not implemented";
#endif
  return;
}


/* ------------------------------------------------------------------------- */

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)

class OFReadWriteLockHelper
{
 public:
  OFReadWriteLockHelper(): accessMutex(), usageSemaphore(1), numReaders(0) {}
  ~OFReadWriteLockHelper() {}

  OFMutex accessMutex;
  OFSemaphore usageSemaphore;
  int numReaders;
 private:
  /* undefined */ OFReadWriteLockHelper(const OFReadWriteLockHelper& arg);
  /* undefined */ OFReadWriteLockHelper& operator=(const OFReadWriteLockHelper& arg);
};

#endif

/* ------------------------------------------------------------------------- */


#if defined(WINDOWS_INTERFACE)
  const int OFReadWriteLock::busy = -1;
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const int OFReadWriteLock::busy = EBUSY;
#else
  const int OFReadWriteLock::busy = -2;
#endif

OFReadWriteLock::OFReadWriteLock()
: theLock(NULL)
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
   OFReadWriteLockHelper *rwl = new OFReadWriteLockHelper();
   if ((rwl->accessMutex.initialized()) && (rwl->usageSemaphore.initialized())) theLock=rwl;
   else delete rwl;
#elif defined(POSIX_INTERFACE)
  pthread_rwlock_t *rwl = new pthread_rwlock_t;
  if (rwl)
  {
    if (pthread_rwlock_init(rwl, NULL)) delete rwl;
    else theLock = rwl;
  }
#elif defined(SOLARIS_INTERFACE)
  rwlock_t *rwl = new rwlock_t;
  if (rwl)
  {
    if (rwlock_init(rwl, USYNC_THREAD, NULL)) delete rwl;
    else theLock = rwl;
  }
#else
#endif
}

OFReadWriteLock::~OFReadWriteLock()
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
  delete (OFReadWriteLockHelper *)theLock;
#elif defined(POSIX_INTERFACE)
  if (theLock) pthread_rwlock_destroy((pthread_rwlock_t *)theLock);
  delete (pthread_rwlock_t *)theLock;
#elif defined(SOLARIS_INTERFACE)
  if (theLock) rwlock_destroy((rwlock_t *)theLock);
  delete (rwlock_t *)theLock;
#else
#endif
}

OFBool OFReadWriteLock::initialized() const
{
#ifdef WITH_THREADS
  if (theLock) return OFTrue; else return OFFalse;
#else
  return OFTrue;
#endif
}

int OFReadWriteLock::rdlock()
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
  if (theLock)
  {
    OFReadWriteLockHelper *rwl = (OFReadWriteLockHelper *)theLock;
    int result =0;
    while (1)
    {
      if (0 != (result = rwl->accessMutex.lock())) return result; // lock mutex
      if (rwl->numReaders >= 0) // we can grant the read lock
      {
        if (rwl->numReaders == 0)
        {
          if (0 != (result = rwl->usageSemaphore.wait()))
          {
            rwl->accessMutex.unlock();
            return result;
          }
        }
        (rwl->numReaders)++;
        return rwl->accessMutex.unlock();
      }
      // we cannot grant the read lock, block thread.
      if (0 != (result = rwl->accessMutex.unlock())) return result;
      if (0 != (result = rwl->usageSemaphore.wait())) return result;
      if (0 != (result = rwl->usageSemaphore.post())) return result;
    }
  } else return EINVAL;
#elif defined(POSIX_INTERFACE)
  if (theLock) return pthread_rwlock_rdlock((pthread_rwlock_t *)theLock); else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theLock) return rw_rdlock((rwlock_t *)theLock); else return EINVAL;
#else
  return -1;
#endif
}

int OFReadWriteLock::wrlock()
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
  if (theLock)
  {
    OFReadWriteLockHelper *rwl = (OFReadWriteLockHelper *)theLock;
    int result =0;
    while (1)
    {
      if (0 != (result = rwl->accessMutex.lock())) return result; // lock mutex
      if (rwl->numReaders == 0) // we can grant the write lock
      {
        if (0 != (result = rwl->usageSemaphore.wait()))
        {
          rwl->accessMutex.unlock();
          return result;
        }
        rwl->numReaders = -1;
        return rwl->accessMutex.unlock();
      }

      // we cannot grant the write lock, block thread.
      if (0 != (result = rwl->accessMutex.unlock())) return result;
      if (0 != (result = rwl->usageSemaphore.wait())) return result;
      if (0 != (result = rwl->usageSemaphore.post())) return result;
    }
  } else return EINVAL;
#elif defined(POSIX_INTERFACE)
  if (theLock) return pthread_rwlock_wrlock((pthread_rwlock_t *)theLock); else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theLock) return rw_wrlock((rwlock_t *)theLock); else return EINVAL;
#else
  return -1;
#endif
}


int OFReadWriteLock::tryrdlock()
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
  if (theLock)
  {
    OFReadWriteLockHelper *rwl = (OFReadWriteLockHelper *)theLock;
    int result =0;
    if (0 != (result = rwl->accessMutex.lock())) return result; // lock mutex
    if (rwl->numReaders >= 0) // we can grant the read lock
    {
      if (rwl->numReaders == 0)
      {
        if (0 != (result = rwl->usageSemaphore.wait()))
        {
          rwl->accessMutex.unlock();
          return result;
        }
      }
      (rwl->numReaders)++;
      return rwl->accessMutex.unlock();
    }
    result = rwl->accessMutex.unlock();
    if (result) return result; else return OFReadWriteLock::busy;
  } else return EINVAL;
#elif defined(POSIX_INTERFACE)
  if (theLock) return pthread_rwlock_tryrdlock((pthread_rwlock_t *)theLock); else return EINVAL; // may return EBUSY.
#elif defined(SOLARIS_INTERFACE)
  if (theLock) return rw_tryrdlock((rwlock_t *)theLock); else return EINVAL; // may return EBUSY.
#else
  return -1;
#endif
}

int OFReadWriteLock::trywrlock()
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
  if (theLock)
  {
    OFReadWriteLockHelper *rwl = (OFReadWriteLockHelper *)theLock;
    int result =0;
    if (0 != (result = rwl->accessMutex.lock())) return result; // lock mutex
    if (rwl->numReaders == 0) // we can grant the write lock
    {
      if (0 != (result = rwl->usageSemaphore.wait()))
      {
        rwl->accessMutex.unlock();
        return result;
      }
      rwl->numReaders = -1;
      return rwl->accessMutex.unlock();
    }
    result = rwl->accessMutex.unlock();
    if (result) return result; else return OFReadWriteLock::busy;
  } else return EINVAL;
#elif defined(POSIX_INTERFACE)
  if (theLock) return pthread_rwlock_trywrlock((pthread_rwlock_t *)theLock); else return EINVAL; // may return EBUSY.
#elif defined(SOLARIS_INTERFACE)
  if (theLock) return rw_trywrlock((rwlock_t *)theLock); else return EINVAL; // may return EBUSY.
#else
  return -1;
#endif
}


int OFReadWriteLock::unlock()
{
#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE_WITHOUT_RWLOCK)
  if (theLock)
  {
    OFReadWriteLockHelper *rwl = (OFReadWriteLockHelper *)theLock;
    int result =0;
    if (0 != (result = rwl->accessMutex.lock())) return result; // lock mutex
    if (rwl->numReaders == -1) rwl->numReaders = 0; else (rwl->numReaders)--;
    if ((rwl->numReaders == 0) && (0 != (result = rwl->usageSemaphore.post())))
    {
      rwl->accessMutex.unlock();
      return result;
    }
    return rwl->accessMutex.unlock();
  } else return EINVAL;
#elif defined(POSIX_INTERFACE)
  if (theLock) return pthread_rwlock_unlock((pthread_rwlock_t *)theLock); else return EINVAL;
#elif defined(SOLARIS_INTERFACE)
  if (theLock) return rw_unlock((rwlock_t *)theLock); else return EINVAL;
#else
  return -1;
#endif
}

#if defined(WINDOWS_INTERFACE) || defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
void OFReadWriteLock::errorstr(OFString& description, int code)
#else
void OFReadWriteLock::errorstr(OFString& description, int /* code */ )
#endif
{
#ifdef WINDOWS_INTERFACE
  if (code == OFSemaphore::busy) description = "read/write lock is already locked"; else
  {
    LPVOID buf;
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, (DWORD)code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &buf, 0, NULL);
    if (buf) description = (const char *)buf;
    LocalFree(buf);
  }
#elif defined(POSIX_INTERFACE) || defined(SOLARIS_INTERFACE)
  const char *str = strerror(code);
  if (str) description = str; else description.clear();
#else
  description = "error: read/write lock not implemented";
#endif
  return;
}
