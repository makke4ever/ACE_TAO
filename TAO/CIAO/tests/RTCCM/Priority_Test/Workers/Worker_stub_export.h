
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl WORKER_STUB
// ------------------------------
#ifndef WORKER_STUB_EXPORT_H
#define WORKER_STUB_EXPORT_H

#include "ace/config-all.h"

#if !defined (WORKER_STUB_HAS_DLL)
#  define WORKER_STUB_HAS_DLL 1
#endif /* ! WORKER_STUB_HAS_DLL */

#if defined (WORKER_STUB_HAS_DLL) && (WORKER_STUB_HAS_DLL == 1)
#  if defined (WORKER_STUB_BUILD_DLL)
#    define WORKER_STUB_Export ACE_Proper_Export_Flag
#    define WORKER_STUB_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define WORKER_STUB_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* WORKER_STUB_BUILD_DLL */
#    define WORKER_STUB_Export ACE_Proper_Import_Flag
#    define WORKER_STUB_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define WORKER_STUB_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* WORKER_STUB_BUILD_DLL */
#else /* WORKER_STUB_HAS_DLL == 1 */
#  define WORKER_STUB_Export
#  define WORKER_STUB_SINGLETON_DECLARATION(T)
#  define WORKER_STUB_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* WORKER_STUB_HAS_DLL == 1 */

// Set WORKER_STUB_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (WORKER_STUB_NTRACE)
#  if (ACE_NTRACE == 1)
#    define WORKER_STUB_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define WORKER_STUB_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !WORKER_STUB_NTRACE */

#if (WORKER_STUB_NTRACE == 1)
#  define WORKER_STUB_TRACE(X)
#else /* (WORKER_STUB_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define WORKER_STUB_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (WORKER_STUB_NTRACE == 1) */

#endif /* WORKER_STUB_EXPORT_H */

// End of auto generated file.
