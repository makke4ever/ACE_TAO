/* -*- C++ -*- */
// $Id$

// ====================================================================
//
// = LIBRARY
//    TAO
//
// = FILENAME
//    DynArray_i.cpp
//
// = AUTHOR
//    Jeff Parsons <parsons@cs.wustl.edu>
//
// ====================================================================

#include "tao/DynAny_i.h"

#if !defined (TAO_HAS_MINIMUM_CORBA)

#include "tao/DynArray_i.h"
#include "tao/InconsistentTypeCodeC.h"

// Constructors and destructor

TAO_DynArray_i::TAO_DynArray_i (const CORBA_Any& any)
  : type_ (any.type ()),
    current_index_ (0),
    da_members_ (0)
{
  ACE_DECLARE_NEW_CORBA_ENV;
  ACE_TRY
    {
      CORBA::TypeCode_ptr tc = this->type_.in ();

      // The type will be correct if this constructor called from a
      // factory function, but it could also be called by the user,
      // so.....
      CORBA::TCKind kind = TAO_DynAny_i::unalias (tc,
                                                  ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (kind == CORBA::tk_array)
        {
          CORBA::ULong numfields = this->get_arg_length (any.type (),
                                                         ACE_TRY_ENV);
          ACE_TRY_CHECK;
          // Resize the array.
          this->da_members_.size (numfields);

          // Get the CDR stream of the argument.
          ACE_Message_Block* mb = any._tao_get_cdr ();

          TAO_InputCDR cdr (mb,
                            any._tao_byte_order ());

          CORBA::TypeCode_var field_tc =
            this->get_element_type (ACE_TRY_ENV);
          ACE_TRY_CHECK;

          for (CORBA::ULong i = 0; i < numfields; i++)
            {
              // This Any constructor is a TAO extension.
              CORBA_Any field_any (field_tc.in (),
                                   0,
                                   cdr.byte_order (),
                                   cdr.start ());

              // This recursive step will call the correct constructor
              // based on the type of field_any.
              this->da_members_[i] =
                TAO_DynAny_i::create_dyn_any (field_any,
                                              ACE_TRY_ENV);
              ACE_TRY_CHECK;

              // Move to the next field in the CDR stream.
              cdr.skip (field_tc.in ());
            }
        }
      else
        {
          ACE_THROW (CORBA_ORB_InconsistentTypeCode ());
        }
    }
  ACE_CATCHANY
    {
      // do nothing...
    }
  ACE_ENDTRY;
  ACE_CHECK;
}

TAO_DynArray_i::TAO_DynArray_i (CORBA_TypeCode_ptr tc)
  : type_ (CORBA::TypeCode::_duplicate (tc)),
    current_index_ (0),
    da_members_ (0)
{
  ACE_DECLARE_NEW_CORBA_ENV;
  ACE_TRY
    {
      // Need to check if called by user.
      CORBA::TCKind kind = TAO_DynAny_i::unalias (tc,
                                                  ACE_TRY_ENV);
      ACE_TRY_CHECK;

      if (kind == CORBA::tk_array)
        {
          CORBA::ULong numfields = this->get_arg_length (tc,
                                                         ACE_TRY_ENV);
          ACE_TRY_CHECK;
          // Resize the array.
          this->da_members_.size (numfields);

          for (CORBA::ULong i = 0; i < numfields; i++)
            // With a typecode arg, we just create the top level.
            this->da_members_[i] = 0;
        }
      else
        ACE_THROW (CORBA_ORB_InconsistentTypeCode ());
    }
  ACE_CATCHANY
    {
      // do nothing...
    }
  ACE_ENDTRY;
  ACE_CHECK;
}

TAO_DynArray_i::~TAO_DynArray_i (void)
{
}

// Functions specific to DynArray

CORBA_AnySeq_ptr
TAO_DynArray_i::get_elements (CORBA::Environment& ACE_TRY_ENV)
{
  CORBA::ULong length = this->da_members_.size ();

  if (length == 0)
    return 0;

  // Arg only sets maximum, so...
  CORBA_AnySeq_ptr elements;

  ACE_NEW_THROW_EX (elements,
                    CORBA_AnySeq (length),
                    CORBA::NO_MEMORY ());
  ACE_CHECK_RETURN (0);

  // ...we must do this explicitly.
  elements->length (length);

  // Initialize each Any.
  for (CORBA::ULong i = 0; i < length; i++)
    {
      CORBA::Any_var temp = this->da_members_[i]->to_any (ACE_TRY_ENV);
      ACE_CHECK_RETURN (0);

      (*elements)[i] = temp.in ();
    }

  return elements;
}

void
TAO_DynArray_i::set_elements (const CORBA_AnySeq& value,
                              CORBA::Environment& ACE_TRY_ENV)
{
  CORBA::ULong length = value.length ();
  CORBA::ULong size = this->da_members_.size ();

  if (size != length)
    {
      ACE_THROW (CORBA_DynAny::InvalidSeq ());
    }

  CORBA::TypeCode_var element_type =
    this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  for (CORBA::ULong i = 0; i < length; i++)
    {
      // Check each arg element for type match.
      CORBA::Boolean equal = value[i].type ()->equal (element_type.in (),
                                                      ACE_TRY_ENV);
      ACE_CHECK;

      if (equal)
        {
          if (!CORBA::is_nil (this->da_members_[i].in ()))
            {
              this->da_members_[i]->destroy (ACE_TRY_ENV);
              ACE_CHECK;
            }

          this->da_members_[i] =
            TAO_DynAny_i::create_dyn_any (value[i],
                                          ACE_TRY_ENV);
          ACE_CHECK;
        }
      else
        {
          ACE_THROW (CORBA_DynAny::InvalidSeq ());
        }
    }
}

// Common functions

void
TAO_DynArray_i::assign (CORBA_DynAny_ptr dyn_any,
                        CORBA::Environment &ACE_TRY_ENV)
{
  // *dyn_any->to_any raises Invalid if arg is bad.
  CORBA_TypeCode_ptr tc = dyn_any->type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::Boolean equal = this->type_.in ()->equal (tc,
                                                   ACE_TRY_ENV);
  ACE_CHECK;

  if (equal)
    {
      CORBA_Any any = *dyn_any->to_any (ACE_TRY_ENV);
      ACE_CHECK;

      this->from_any (any,
                      ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::Invalid ());
    }
}

CORBA_DynAny_ptr
TAO_DynArray_i::copy (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_Any_ptr any = this->to_any (ACE_TRY_ENV);
  ACE_CHECK_RETURN (CORBA_DynAny::_nil ());

  CORBA_DynAny_ptr retval = TAO_DynAny_i::create_dyn_any (*any,
                                                          ACE_TRY_ENV);
  ACE_CHECK_RETURN (CORBA_DynAny::_nil ());

  return retval;
}

void
TAO_DynArray_i::destroy (CORBA::Environment &ACE_TRY_ENV)
{
  // Do a deep destroy
  for (CORBA::ULong i = 0;
       i < this->da_members_.size ();
       i++)
    if (!CORBA::is_nil (this->da_members_[i].in ()))
      {
        this->da_members_[i]->destroy (ACE_TRY_ENV);
        ACE_CHECK;
      }

  // Free the top level
  delete this;
}

void
TAO_DynArray_i::from_any (const CORBA_Any& any,
                          CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Boolean equal = this->type_.in ()->equal (any.type (),
                                                   ACE_TRY_ENV);
  ACE_CHECK;

  if (equal)
    {
      // Get the CDR stream of the argument.
      ACE_Message_Block* mb = any._tao_get_cdr ();
      TAO_InputCDR cdr (mb,
                        any._tao_byte_order ());

      CORBA::ULong length = this->da_members_.size ();
      CORBA::ULong arg_length = this->get_arg_length (any.type (),
                                                      ACE_TRY_ENV);
      ACE_CHECK;

      if (length != arg_length)
        {
          ACE_THROW (CORBA_DynAny::Invalid ());
        }

      CORBA::TypeCode_var field_tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK;

      for (CORBA::ULong i = 0; i < arg_length; i++)
        {
          // This Any constructor is a TAO extension.
          CORBA_Any field_any (field_tc.in (),
                               0,
                               cdr.byte_order (),
                               cdr.start ());

          if (!CORBA::is_nil (this->da_members_[i].in ()))
            {
              this->da_members_[i]->destroy (ACE_TRY_ENV);
              ACE_CHECK;
            }

          this->da_members_[i] =
            TAO_DynAny_i::create_dyn_any (field_any,
                                          ACE_TRY_ENV);
          ACE_CHECK;

          // Move to the next field in the CDR stream.
          cdr.skip (field_tc.in ());
        }
    }
  else
    {
      ACE_THROW (CORBA_DynAny::Invalid ());
    }
}

CORBA::Any_ptr
TAO_DynArray_i::to_any (CORBA::Environment& ACE_TRY_ENV)
{
  TAO_OutputCDR out_cdr;

  CORBA_TypeCode_var field_tc = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK_RETURN (0);

  for (CORBA::ULong i = 0;
       i < this->da_members_.size ();
       i++)
    {
      // Each component must have been initialized.
      if (!this->da_members_[i].in ())
        {
          ACE_THROW_RETURN (CORBA_DynAny::Invalid (), 0);
        }

      // Recursive step
      CORBA_Any_var field_any =
        this->da_members_[i]->to_any (ACE_TRY_ENV);
      ACE_CHECK_RETURN (0);

      ACE_Message_Block* field_mb = field_any->_tao_get_cdr ();

      TAO_InputCDR field_cdr (field_mb,
                              field_any->_tao_byte_order ());

      out_cdr.append (field_tc.in (),
                      &field_cdr,
                      ACE_TRY_ENV);
      ACE_CHECK_RETURN (0);
    }

  TAO_InputCDR in_cdr (out_cdr);

  CORBA_Any *retval;

  CORBA_TypeCode_ptr my_tc = this->type (ACE_TRY_ENV);
  ACE_CHECK_RETURN (0);

  ACE_NEW_THROW_EX (retval,
                    CORBA_Any (my_tc,
                               0,
                               in_cdr.byte_order (),
                               in_cdr.start ()),
                    CORBA::NO_MEMORY ());
  ACE_CHECK_RETURN (0);

  return retval;
}

CORBA::TypeCode_ptr
TAO_DynArray_i::type (CORBA::Environment &)
{
  return CORBA::TypeCode::_duplicate (this->type_.in ());
}

// If the DynAny has been initialized but this component has not, the
// first call to current_component will create the pointer and return
// it.

CORBA_DynAny_ptr
TAO_DynArray_i::current_component (CORBA::Environment &ACE_TRY_ENV)
{
  if (this->da_members_.size () == 0)
    return 0;

  if (!this->da_members_[this->current_index_].in ())
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (CORBA_DynAny::_nil ());

      this->da_members_[this->current_index_] =
        TAO_DynAny_i::create_dyn_any (tc.in (),
                                      ACE_TRY_ENV);
      ACE_CHECK_RETURN (CORBA_DynAny::_nil ());
    }

  return CORBA_DynAny::_duplicate (
            this->da_members_[this->current_index_].in ()
          );
}

CORBA::Boolean
TAO_DynArray_i::next (CORBA::Environment &)
{
  CORBA::Long size = (CORBA::Long) this->da_members_.size ();

  if (size == 0 || this->current_index_ + 1 == size)
    return 0;

  ++this->current_index_;
  return 1;
}

CORBA::Boolean
TAO_DynArray_i::seek (CORBA::Long slot,
                      CORBA::Environment &)
{
  if (slot < 0 || slot >= (CORBA::Long) this->da_members_.size ())
    {
      return 0;
    }

  this->current_index_ = slot;
  return 1;
}

void
TAO_DynArray_i::rewind (CORBA::Environment &)
{
  this->current_index_ = 0;
}

// The insert-primitive and get-primitive functions are required by
// the spec of all types of DynAny, although if the top level members
// aren't primitive types, these functions aren't too helpful.  Also,
// while not mentioned in the spec, the example code seems to indicate
// that next() is called in the body of each of these, and it has been
// so implemented here.

// Insert functions

void
TAO_DynArray_i::insert_boolean (CORBA::Boolean value,
                                CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_boolean)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_boolean (value,
                          ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_octet (CORBA::Octet value,
                              CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_octet)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_octet (value,
                        ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_char (CORBA::Char value,
                             CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_char)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_char (value,
                       ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_short (CORBA::Short value,
                              CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_short)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_short (value,
                        ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_ushort (CORBA::UShort value,
                               CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_ushort)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_ushort (value,
                         ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_long (CORBA::Long value,
                             CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_long)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_long (value,
                       ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_ulong (CORBA::ULong value,
                              CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_ulong)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_ulong (value,
                        ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_float (CORBA::Float value,
                              CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_float)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_float (value,
                        ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_double (CORBA::Double value,
                               CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_double)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_double (value,
                         ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_string (const char * value,
                               CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_string)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_string (value,
                         ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_reference (CORBA::Object_ptr value,
                                  CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_objref)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_reference (value,
                            ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_typecode (CORBA::TypeCode_ptr value,
                                 CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_TypeCode)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_typecode (value,
                           ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_longlong (CORBA::LongLong value,
                                 CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_longlong)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_longlong (value,
                           ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_ulonglong (CORBA::ULongLong value,
                                  CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_ulonglong)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_ulonglong (value,
                            ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_wchar (CORBA::WChar value,
                              CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_wchar)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_wchar (value,
                        ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

void
TAO_DynArray_i::insert_any (const CORBA::Any& value,
                            CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_var type = this->get_element_type (ACE_TRY_ENV);
  ACE_CHECK;

  CORBA::TCKind kind = type->kind (ACE_TRY_ENV);
  ACE_CHECK;

  if (kind == CORBA::tk_any)
    {
      CORBA_DynAny_ptr dp = this->current_component (ACE_TRY_ENV);
      ACE_CHECK;

      dp->insert_any (value,
                      ACE_TRY_ENV);
      ACE_CHECK;

      this->next (ACE_TRY_ENV);
      ACE_CHECK;
    }
  else
    {
      ACE_THROW (CORBA_DynAny::InvalidValue ());
    }
}

// Get functions

// If the current component has not been intialized, these
// raise Invalid, which is not required by the spec, but which
// seems like a courteous thing to do.

CORBA::Boolean
TAO_DynArray_i::get_boolean (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Boolean val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_boolean)
        {
          val = dp->get_boolean (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Octet
TAO_DynArray_i::get_octet (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Octet val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_octet)
        {
          val = dp->get_octet (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Char
TAO_DynArray_i::get_char (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Char val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_char)
        {
          val = dp->get_char (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Short
TAO_DynArray_i::get_short (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Short val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_short)
        {
          val = dp->get_short (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::UShort
TAO_DynArray_i::get_ushort (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::UShort val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_ushort)
        {
          val = dp->get_ushort (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Long
TAO_DynArray_i::get_long (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Long val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_long)
        {
          val = dp->get_long (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::ULong
TAO_DynArray_i::get_ulong (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::ULong val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_ulong)
        {
          val = dp->get_ulong (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Float
TAO_DynArray_i::get_float (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Float val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_float)
        {
          val = dp->get_float (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Double
TAO_DynArray_i::get_double (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Double val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_double)
        {
          val = dp->get_double (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

char *
TAO_DynArray_i::get_string (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::Char *val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_string)
        {
          val = dp->get_string (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Object_ptr
TAO_DynArray_i::get_reference (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_Object_ptr val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_objref)
        {
          val = dp->get_reference (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::TypeCode_ptr
TAO_DynArray_i::get_typecode (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_TypeCode_ptr val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_TypeCode)
        {
          val = dp->get_typecode (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::LongLong
TAO_DynArray_i::get_longlong (CORBA::Environment &ACE_TRY_ENV)
{
#if defined (ACE_LACKS_LONGLONG_T)
  CORBA::LongLong val = {0, 0};
#else  /* ! ACE_LACKS_LONGLONG_T */
  CORBA::LongLong val = 0;
#endif /* ! ACE_LACKS_LONGLONG_T */

  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_longlong)
        {
          val = dp->get_longlong (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::ULongLong
TAO_DynArray_i::get_ulonglong (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::ULongLong val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_ulonglong)
        {
          val = dp->get_ulonglong (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::WChar
TAO_DynArray_i::get_wchar (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::WChar val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_wchar)
        {
          val = dp->get_wchar (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

CORBA::Any_ptr
TAO_DynArray_i::get_any (CORBA::Environment &ACE_TRY_ENV)
{
  CORBA_Any_ptr val = 0;
  CORBA_DynAny_ptr dp = this->da_members_[this->current_index_].in ();

  if (dp)
    {
      CORBA_TypeCode_var tc = this->get_element_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      CORBA::TCKind kind = tc->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (val);

      if (kind == CORBA::tk_any)
        {
          val = dp->get_any (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);

          this->next (ACE_TRY_ENV);
          ACE_CHECK_RETURN (val);
        }
      else
        ACE_THROW_RETURN (CORBA_DynAny::TypeMismatch (),
                          val);
    }
  else
    ACE_THROW_RETURN (CORBA_DynAny::Invalid (),
                      val);

  return val;
}

// Private utility function.

CORBA::TypeCode_ptr
TAO_DynArray_i::get_element_type (CORBA::Environment& ACE_TRY_ENV)
{
  CORBA::TypeCode_var element_type =
    CORBA_TypeCode::_duplicate (this->type_.in ());

  // Strip away aliases (if any) on top of the outer type
  CORBA::TCKind kind = element_type->kind (ACE_TRY_ENV);
  ACE_CHECK_RETURN (CORBA_TypeCode::_nil ());

  while (kind != CORBA::tk_array)
    {
      element_type = element_type->content_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (CORBA_TypeCode::_nil ());

      kind = element_type->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (CORBA_TypeCode::_nil ());
    }

  // Return the content type.
  CORBA_TypeCode_ptr retval = element_type->content_type (ACE_TRY_ENV);
  ACE_CHECK_RETURN (CORBA_TypeCode::_nil ());

  return retval;
}

// Get the argument length from the (aliased) typecode.
CORBA::ULong
TAO_DynArray_i::get_arg_length (CORBA::TypeCode_ptr tc,
                                CORBA::Environment &ACE_TRY_ENV)
{
  CORBA::TypeCode_var tctmp = CORBA::TypeCode::_duplicate (tc);
  CORBA::TCKind kind = tctmp->kind (ACE_TRY_ENV);
  ACE_CHECK_RETURN (0);

  while (kind == CORBA::tk_alias)
    {
      tctmp = tctmp->content_type (ACE_TRY_ENV);
      ACE_CHECK_RETURN (0);
      kind = tctmp->kind (ACE_TRY_ENV);
      ACE_CHECK_RETURN (0);
    }

  CORBA::ULong retval = tctmp->length (ACE_TRY_ENV);
  ACE_CHECK_RETURN (0);

  return retval;
}

// **********************************************************

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Array_Base<CORBA_DynAny_var>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Array_Base<CORBA_DynAny_var>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

#endif /* TAO_HAS_MINIMUM_CORBA */
