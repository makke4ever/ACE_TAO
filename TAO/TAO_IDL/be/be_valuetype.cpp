// $Id$

// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    be_valuetype.cpp
//
// = DESCRIPTION
//    Extension of class AST_Interface and be_interface that provides
//    additional means for C++ mapping of an valuetype.
//
// = AUTHOR
//    Torsten Kuepper  <kuepper2@lfa.uni-wuppertal.de>
//    derived from be_interface.cpp
//
// ============================================================================


#include "idl.h"
#include "idl_extern.h"
#include "be.h"

#ifdef IDL_HAS_VALUETYPE

ACE_RCSID(be, be_valuetype, "$Id$")

// Default constructor.
be_valuetype::be_valuetype (void)
  : full_obv_skel_name_ (0),
    abstract_ (0)
{
  // Always the case.
  this->size_type (be_decl::VARIABLE);

  AST_Module *m = AST_Module::narrow_from_scope (this->defined_in ());

  if (m != 0)
    {
      m->set_has_nested_valuetype ();
    }

  // Always the case.
  this->has_constructor (I_TRUE);
}

// Constructor used to build the AST.
be_valuetype::be_valuetype (UTL_ScopedName *n,
                            AST_Interface **ih,
                            long nih,
                            UTL_StrList *p,
                            idl_bool set_abstract)
  : be_interface (n, 
                  ih, 
                  nih, 
                  0, 
                  0, 
                  p, 
                  0, 
                  0),
    AST_Interface (n, 
                   ih, 
                   nih, 
                   0, 
                   0, 
                   p, 
                   0, 
                   0),
    AST_Decl (AST_Decl::NT_interface,  // It's like an interface.
              n, 
              p),
    UTL_Scope (AST_Decl::NT_interface),
    full_obv_skel_name_ (0),
    abstract_ (set_abstract)
{
  // Check that redefine() copies all members.

  // Always the case.
  this->size_type (be_decl::VARIABLE);

  AST_Module *m = AST_Module::narrow_from_scope (this->defined_in ());

  if (m != 0)
    {
      m->set_has_nested_valuetype ();
    }

  // Always the case.
  this->has_constructor (I_TRUE);
}

be_valuetype::~be_valuetype (void)
{

}

void
be_valuetype::redefine (AST_Interface *from,
                        UTL_StrList *p)
{
  this->AST_Interface::redefine (from, 
                                 p);

  abstract_ = from->is_abstract_valuetype ();
}

// Is true if non-virtual accessor and modifier should be generated
// If #pragma TAO OBV opt_accessor (todo) is used or -Wb,obv_opt_accessor.
idl_bool
be_valuetype::opt_accessor (void)
{
  return be_global->obv_opt_accessor ();
}

idl_bool
be_valuetype::is_valuetype (void)
{
  return 1;
}

idl_bool
be_valuetype::is_abstract_valuetype (void)
{
  return this->abstract_;
}

void
be_valuetype::set_abstract_valuetype (void)
{
  this->abstract_ = 1;
}

// Compute stringified fully scoped skeleton name (OBV_name).
void
be_valuetype::compute_fullobvskelname (void)
{
  this->compute_full_skel_name ("OBV_",
                                this->full_obv_skel_name_);
}


// Retrieve the fully scoped skeleton name.
const char*
be_valuetype::full_obv_skel_name (void)
{
  if (!this->full_obv_skel_name_)
    {
      compute_fullobvskelname ();
    }

  return this->full_obv_skel_name_;
}



// Generate the var definition.
int
be_valuetype::gen_var_defn (char *)
{
  TAO_OutStream *ch = 0;
  TAO_NL nl;
  char namebuf [NAMEBUFSIZE];

  ACE_OS::memset (namebuf,
                  '\0',
                  NAMEBUFSIZE);

  ACE_OS::sprintf (namebuf,
                   "%s_var",
                   this->local_name ());

  ch = tao_cg->client_header ();

  // Generate the var definition (always in the client header).
  // Depending upon the data type, there are some differences which we account
  // for over here.

  // Start with whatever was our current indent level.
  ch->indent ();
  *ch << "class " << be_global->stub_export_macro ()
      << " " << namebuf << nl;
  *ch << "{" << nl;
  *ch << "public:\n";
  ch->incr_indent ();

  // Default constructor.
  *ch << namebuf << " (void); // default constructor" << nl;
  *ch << namebuf << " (" << this->local_name () << "*);" << nl;
  *ch << namebuf << " (const " << this->local_name ()
      << "*); // (TAO extension)" << nl;

  // Copy constructor.
  *ch << namebuf << " (const " << namebuf <<
    " &); // copy constructor" << nl;

  // Destructor.
  *ch << "~" << namebuf << " (void); // destructor" << nl;
  *ch << nl;

  // Assignment operator from a pointer.
  *ch << namebuf << " &operator= (" << this->local_name ()
      << "*);" << nl;

  // Assignment from _var.
  *ch << namebuf << " &operator= (const " << namebuf <<
    " &);" << nl;

  // Arrow operator.
  *ch << local_name () << "* operator-> (void) const;" << nl;

  *ch << nl;

  // Other extra types (cast operators, [] operator, and others).
  *ch << "operator const " << this->local_name ()
      << "* () const;" << nl;
  *ch << "operator " << this->local_name () << "* ();" << nl;

  *ch << "// in, inout, out, _retn " << nl;
  // The return types of in, out, inout, and _retn are based on the parameter
  // passing rules and the base type.
  *ch << this->local_name () << "* in (void) const;" << nl;
  *ch << this->local_name () << "* &inout (void);" << nl;
  *ch << this->local_name () << "* &out (void);" << nl;
  *ch << this->local_name () << "* _retn (void);" << nl;

  // Generate an additional member function that returns 
  // the underlying pointer.
  *ch << this->local_name () << "* ptr (void) const;\n";

  *ch << "\n";
  ch->decr_indent ();

  // Private.
  *ch << "private:\n";
  ch->incr_indent ();
  *ch << this->local_name () << "* ptr_;\n";

  ch->decr_indent ();
  *ch << "};\n\n";

  return 0;
}

// Implementation of the _var class. All of these get generated in the inline
// file.
int
be_valuetype::gen_var_impl (char *,
                            char *)
{
  TAO_OutStream *ci = 0;
  TAO_NL nl;

  // To hold the full and local _var names.
  char fname [NAMEBUFSIZE];
  char lname [NAMEBUFSIZE];

  ACE_OS::memset (fname,
                  '\0',
                  NAMEBUFSIZE);

  ACE_OS::sprintf (fname,
                   "%s_var",
                   this->full_name ());

  ACE_OS::memset (lname,
                  '\0',
                  NAMEBUFSIZE);

  ACE_OS::sprintf (lname,
                   "%s_var",
                   this->local_name ());

  ci = tao_cg->client_inline ();

  // Generate the var implementation in the inline file
  // Depending upon the data type, there are some differences which we account
  // for over here.

  ci->indent (); // start with whatever was our current indent level

  *ci << "// *************************************************************"
      << nl;
  *ci << "// Inline operations for class " << fname << nl;
  *ci << "// *************************************************************\n\n";

  // Default constructor.
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname <<
    " (void) // default constructor" << nl;
  *ci << "  " << ": ptr_ (0)" << nl;
  *ci << "{}\n\n";

  // Constructor from a pointer.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (" 
      << this->local_name () << "* p)" << nl;
  *ci << "  : ptr_ (p)" << nl;
  *ci << "{}\n\n";

  // Constructor from a const pointer.
  // TAO extension - it appears that there are problems with at least g++
  // which reclaims amguity between T(T*) and T(const T_var &)
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (const "
      << this->local_name () << "* p)" << nl;
  *ci << "  : ptr_ (ACE_const_cast(" 
      << this->local_name () << "*, p))" << nl;
  *ci << "{}\n\n";

  // The additional ptr () member function. This member function must be
  // defined before the remaining member functions including the copy
  // constructor because this inline function is used elsewhere. Hence to make
  // inlining of this function possible, we must define it before its use.
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* " << nl;
  *ci << fname << "::ptr (void) const" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Copy constructor.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (const " << lname <<
    " &p) // copy constructor" << nl;
  *ci << "{" << be_idt_nl
      <<    "CORBA::add_ref (p.ptr ());" << be_nl
      <<    "this->ptr_ = p.ptr ();" << be_uidt_nl
      << "}\n\n";

  // Destructor.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::~" << lname << " (void) // destructor" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::remove_ref (this->ptr_);\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (" << this->local_name () 
      << "* p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::remove_ref (this->ptr_);" << nl;
  *ci << "this->ptr_ = p;" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator from _var.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (const " << lname 
      << " &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "if (this != &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::remove_ref (this->ptr_);" << nl
      << this->local_name() << "* tmp = p.ptr ();" << be_nl
      << "CORBA::add_ref (tmp);" << be_nl
      << "this->ptr_ = tmp;\n";
  ci->decr_indent ();
  *ci << "}" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Other extra methods - cast operator ().
  ci->indent ();
  *ci << "ACE_INLINE " << nl;
  *ci << fname << "::operator const " << this->name ()
      << "* () const // cast" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << nl;
  *ci << fname << "::operator " << this->name ()
      << "* () // cast " << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // operator->
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* " << nl;
  *ci << fname << "::operator-> (void) const" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // in, inout, out, and _retn.
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "*" << nl;
  *ci << fname << "::in (void) const" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* &" << nl;
  *ci << fname << "::inout (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* &" << nl;
  *ci << fname << "::out (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::remove_ref (this->ptr_);" << nl;
  *ci << "this->ptr_ = 0;" << nl;
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* " << nl;
  *ci << fname << "::_retn (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "// yield ownership of managed obj reference" << nl;
  *ci << this->local_name () << "* tmp = this->ptr_;" << nl;
  *ci << "this->ptr_ = 0;" << nl;
  *ci << "return tmp;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  return 0;
}

// Generate the _out definition.
int
be_valuetype::gen_out_defn (char *)
{
  TAO_OutStream *ch = 0;
  TAO_NL nl;
  char namebuf [NAMEBUFSIZE];

  ACE_OS::memset (namebuf,
                  '\0',
                  NAMEBUFSIZE);

  ACE_OS::sprintf (namebuf,
                   "%s_out",
                   this->local_name ());

  ch = tao_cg->client_header ();

  // Generate the out definition (always in the client header).

  // Start with whatever was our current indent level.
  ch->indent ();

  *ch << "class " << be_global->stub_export_macro ()
      << " " << namebuf << nl;
  *ch << "{" << nl;
  *ch << "public:\n";
  ch->incr_indent ();

  // No default constructor.

  // Constructor from a pointer.
  *ch << namebuf << " (" << this->local_name () << "* &);" << nl;

  // Constructor from a _var &.
  *ch << namebuf << " (" << this->local_name () << "_var &);" << nl;

  // Constructor from a _out &.
  *ch << namebuf << " (const " << namebuf << " &);" << nl;

  // Assignment operator from a _out &.
  *ch << namebuf << " &operator= (const " << namebuf << " &);" << nl;

  // Assignment operator from a pointer &, cast operator, ptr fn, operator
  // -> and any other extra operators.
  // Only interface allows assignment from var &.
  *ch << namebuf << " &operator= (const " << this->local_name ()
      << "_var &);" << nl;
  *ch << namebuf << " &operator= (" << this->local_name ()
      << "*);" << nl;

  // Cast.
  *ch << "operator " << this->local_name () << "* &();" << nl;

  // ptr function.
  *ch << this->local_name () << "* &ptr (void);" << nl;

  // operator ->
  *ch << this->local_name () << "* operator-> (void);" << nl;

  *ch << "\n";
  ch->decr_indent ();
  *ch << "private:\n";
  ch->incr_indent ();
  *ch << this->local_name () << "* &ptr_;\n";

  ch->decr_indent ();
  *ch << "};\n\n";
  return 0;
}

int
be_valuetype::gen_out_impl (char *, 
                            char *)
{
  TAO_OutStream *ci = 0;
  TAO_NL nl;

  // To hold the full and local _out names.
  char fname [NAMEBUFSIZE];
  char lname [NAMEBUFSIZE];

  ACE_OS::memset (fname,
                  '\0',
                  NAMEBUFSIZE);

  ACE_OS::sprintf (fname,
                   "%s_out",
                   this->full_name ());

  ACE_OS::memset (lname,
                  '\0',
                  NAMEBUFSIZE);

  ACE_OS::sprintf (lname,
                   "%s_out",
                   this->local_name ());

  ci = tao_cg->client_inline ();

  // Generate the var implementation in the inline file
  // Depending upon the data type, there are some differences which we account
  // for over here.

  // Start with whatever was our current indent level.
  ci->indent ();

  *ci << "// *************************************************************"
      << nl;
  *ci << "// Inline operations for class " << fname << nl;
  *ci << "// *************************************************************\n\n";

  // Constructor from a pointer.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (" << this->local_name ()
      << "* &p)" << nl;
  *ci << "  : ptr_ (p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = 0;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Constructor from _var &.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (" << this->local_name () 
      << "_var &p) // constructor from _var" << nl;
  *ci << "  : ptr_ (p.out ())" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::remove_ref (this->ptr_);" << nl;
  *ci << "this->ptr_ = 0;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Copy constructor.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (const " << lname 
      << " &p) // copy constructor" << nl;
  *ci << "  : ptr_ (ACE_const_cast (" << lname << "&,p).ptr_)" << nl;
  *ci << "{}\n\n";

  // Assignment operator from _out &.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (const " << lname <<
    " &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = ACE_const_cast (" << lname << "&,p).ptr_;" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator from _var.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (const " << this->local_name () 
      << "_var &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << this->local_name () << "* tmp = p.ptr ();" << be_nl
      << "CORBA::add_ref (tmp);" << be_nl
      << "this->ptr_ = tmp;" << be_nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator from *.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (" << this->local_name () 
      << "* p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = p;" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Other extra methods - cast operator ().
  ci->indent ();
  *ci << "ACE_INLINE " << nl;
  *ci << fname << "::operator " << this->name () 
      << "* &() // cast" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // ptr function.
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* &" << nl;
  *ci << fname << "::ptr (void) // ptr" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // operator->
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "* " << nl;
  *ci << fname << "::operator-> (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  return 0;
}

// For building the pre and postfix of private data fields.
const char*
be_valuetype::field_pd_prefix (void)
{
  return "_pd_";
}

const char*
be_valuetype::field_pd_postfix (void)
{
  return "";
}

be_valuetype*
be_valuetype::statefull_inherit (void)
{
  be_valuetype *rval = 0;

  if (this->n_inherits () > 0)
    {
      rval = be_valuetype::narrow_from_decl (this->inherits ()[0]);

      if (rval && (rval->is_valuetype ()) 
          && (!rval->is_abstract_valuetype ()))
        {
          return rval;
        }
    }

  return 0;
}

// Accept a visitor.
int
be_valuetype::accept (be_visitor *visitor)
{
  return visitor->visit_valuetype (this);
}

// Narrowing.
IMPL_NARROW_METHODS1 (be_valuetype, be_interface)
IMPL_NARROW_FROM_DECL (be_valuetype)
IMPL_NARROW_FROM_SCOPE (be_valuetype)

#endif /* IDL_HAS_VALUETYPE */
