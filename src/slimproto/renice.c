
#include "slimproto/slimproto.h"

#ifdef SLIMPROTO_RENICE
bool slimproto_renice_thread( int priority )
{
  bool failed;
#ifndef __WIN32__
  int err;
#endif
  failed = false;

#ifdef __WIN32__
  int winpriority;
  int dwError;

  switch (priority)
  {
    case 19:
    case 18:
    case 17:
    case 16:
    case 15:
      winpriority = THREAD_PRIORITY_LOWEST;
      break;

    case 14:
    case 13:
    case 12:
    case 11:
    case 10:
    case  9:
    case  8:
    case  7:
          case  6:
          case  5:
      winpriority = THREAD_PRIORITY_BELOW_NORMAL;
      break;

    case  4:
    case  3:
    case  2:
    case  1:
    case  0:
    case -1:
    case -2:
    case -3:
    case -4:
      winpriority = THREAD_PRIORITY_NORMAL;
      break;

    case -5:
    case -6:
    case -7:
    case -8:
    case -9:
    case -10:
    case -11:
    case -12:
    case -13:
    case -14:
    case -15:
      winpriority = THREAD_PRIORITY_ABOVE_NORMAL;
      break;

    case -16:
    case -17:
    case -18:
    case -19:
    case -20:
      winpriority = THREAD_PRIORITY_HIGHEST;
      break;

    default:
      winpriority = THREAD_PRIORITY_NORMAL;
      break;
  }

  if (!SetThreadPriority(GetCurrentThread(), winpriority))
  {
    dwError = GetLastError();
    fprintf(stderr, "Failed to set thread priority (%d), GetLastError (%d).\n", priority, dwError );
    failed = true;
  }
#else 
  errno = 0;
  err = nice ( priority );

  if ( errno )
  {
    fprintf(stderr, "Failed to set thread priority (%d), errno (%d).\n", priority, errno );
    failed = true;
  }

#endif /* __WIN32__ */

  return (failed);
}

#else

// Dummy implementation when nice levels not available
bool slimproto_renice_thread( int priority )
{
  return false;
}

#endif /* SLIMPROTO_RENICE */
