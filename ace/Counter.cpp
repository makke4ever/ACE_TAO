/* $Id$ */
#include "Counter.h"

#if ! defined (__ACE_INLINE__)
#include "Counter.i"
#endif /* __ACE_INLINE__ */

ACE_RCSID(Event, Object_Counter, "Object_Counter,v 1.0 2004/02/19 10:24:13 storri Exp")

ACE_Object_Counter::ACE_Object_Counter ()
  : m_counter (0)
{
}

ACE_Queue_Counter::ACE_Queue_Counter ()
  : m_counter (0)
{
}
