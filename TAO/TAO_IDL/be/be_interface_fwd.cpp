// $Id$

// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    be_interface.h
//
// = DESCRIPTION
//    Extension of class AST_Interface_Fwd that provides additional means for C++
//    mapping of an interface.
//
// = AUTHOR
//    Copyright 1994-1995 by Sun Microsystems, Inc.
//    and
//    Aniruddha Gokhale
//
// ============================================================================

#include        "idl.h"
#include        "idl_extern.h"
#include        "be.h"

ACE_RCSID(be, be_interface_fwd, "$Id$")

be_interface_fwd::be_interface_fwd (void)
{
  // Always the case.
  this->size_type (be_decl::VARIABLE); 
}

be_interface_fwd::be_interface_fwd (AST_Interface *dummy,
                                    UTL_ScopedName *n, 
                                    UTL_StrList *p)
  : AST_InterfaceFwd (dummy, 
                      n, 
                      p),
    AST_Decl (AST_Decl::NT_interface_fwd, 
              n, 
              p)
{
  // Always the case.
  this->size_type (be_decl::VARIABLE); 
}

be_interface_fwd::~be_interface_fwd (void)
{
}

// Generate the var definition.
int
be_interface_fwd::gen_var_defn (char *)
{
  TAO_OutStream *ch = 0;
  TAO_NL nl;      
  char namebuf [NAMEBUFSIZE]; 

  ACE_OS::memset (namebuf, 
                  '\0', 
                  NAMEBUFSIZE);

  ACE_OS::sprintf (namebuf, 
                   "%s_var", 
                   this->local_name ()->get_string ());

  ch = tao_cg->client_header ();

  // Generate the var definition (always in the client header).
  // Depending upon the data type, there are some differences which we account
  // for over here.

  // Start with whatever was our current indent level.
  ch->indent (); 
  *ch << "class " << be_global->stub_export_macro ()
      << " " << namebuf << " : public TAO_Base_var" << nl;
  *ch << "{" << nl;
  *ch << "public:\n";
  ch->incr_indent ();

  // Default constructor.
  *ch << namebuf << " (void); // default constructor" << nl;
  *ch << namebuf << " (" << this->local_name () << "_ptr p)" 
      << " : ptr_ (p) {} " << nl;

  // Copy constructor.
  *ch << namebuf << " (const " << namebuf <<
    " &); // copy constructor" << nl;

  // Destructor.
  *ch << "~" << namebuf << " (void); // destructor" << nl;
  *ch << nl;

  // Assignment operator from a pointer.
  *ch << namebuf << " &operator= (" << this->local_name () 
      << "_ptr);" << nl;

  // Assignment from _var.
  *ch << namebuf << " &operator= (const " << namebuf <<
    " &);" << nl;

  // Arrow operator.
  *ch << this->local_name () << "_ptr operator-> (void) const;" << nl;

  *ch << nl;

  // Other extra types (cast operators, [] operator, and others).
  *ch << "operator const " << this->local_name () 
      << "_ptr &() const;" << nl;
  *ch << "operator " << this->local_name () << "_ptr &();" << nl;

  *ch << "// in, inout, out, _retn " << nl;
  // The return types of in, out, inout, and _retn are based on the parameter
  // passing rules and the base type.
  *ch << this->local_name () << "_ptr in (void) const;" << nl;
  *ch << this->local_name () << "_ptr &inout (void);" << nl;
  *ch << this->local_name () << "_ptr &out (void);" << nl;
  *ch << this->local_name () << "_ptr _retn (void);" << nl;

  // Generate an additional member function that returns the 
  // underlying pointer.
  *ch << this->local_name () << "_ptr ptr (void) const;\n";

  *ch << "\n";
  ch->decr_indent ();

  // Private.
  *ch << "private:\n";
  ch->incr_indent ();
  *ch << this->local_name () << "_ptr ptr_;" << nl;
  *ch << "// Unimplemented - prevents widening assignment." << nl;
  *ch << this->local_name () << "_var (const TAO_Base_var &rhs);" << nl;
  *ch << this->local_name () << "_var &operator= (const TAO_Base_var &rhs);\n";

  ch->decr_indent ();
  *ch << "};\n\n";

  return 0;
}

// Implementation of the _var class. All of these get generated in the inline
// file.
int
be_interface_fwd::gen_var_impl (char *, 
                                char *)
{
  TAO_OutStream *ci = 0;
  TAO_NL nl;
  char fname [NAMEBUFSIZE];  // To hold the full and
  char lname [NAMEBUFSIZE];  // local _var names.

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
                   this->local_name ()->get_string ());

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

  // Default constructor.
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname <<
    " (void) // default constructor" << nl;
  *ci << "  " << ": ptr_ (" << this->name () << "::_nil ())" << nl;
  *ci << "{}\n\n";

  // The additional ptr () member function. This member function must be
  // defined before the remaining member functions including the copy
  // constructor because this inline function is used elsewhere. Hence to make
  // inlining of this function possible, we must define it before its use.
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr " << nl;
  *ci << fname << "::ptr (void) const" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Copy constructor.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (const " << fname <<
    " &p) // copy constructor" << nl;
  *ci << "  : TAO_Base_var ()," << nl;
  *ci << "    ptr_ (" << this->local_name () << "::_duplicate (p.ptr ()))" << nl;
  *ci << "{}\n\n";

  // Destructor.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::~" << lname << " (void) // destructor" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::release (this->ptr_);\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (" << this->name () <<
    "_ptr p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::release (this->ptr_);" << nl;
  *ci << "this->ptr_ = p;" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator from _var.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (const " << fname <<
    " &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "if (this != &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::release (this->ptr_);" << nl;
  *ci << "this->ptr_ = " << this->name () 
      << "::_duplicate (p.ptr ());\n";
  ci->decr_indent ();
  *ci << "}" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Other extra methods - cast operator ().
  ci->indent ();
  *ci << "ACE_INLINE " << nl;
  *ci << fname << "::operator const " << this->name () <<
    "_ptr &() const // cast" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << nl;
  *ci << fname << "::operator " << this->name () 
      << "_ptr &() // cast " << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // operator->
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr " << nl;
  *ci << fname << "::operator-> (void) const" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // in, inout, out, and _retn
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr" << nl;
  *ci << fname << "::in (void) const" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr &" << nl;
  *ci << fname << "::inout (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr &" << nl;
  *ci << fname << "::out (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::release (this->ptr_);" << nl;
  *ci << "this->ptr_ = " << this->name () << "::_nil ();" << nl;
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr " << nl;
  *ci << fname << "::_retn (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "// yield ownership of managed obj reference" << nl;
  *ci << this->name () << "_ptr val = this->ptr_;" << nl;
  *ci << "this->ptr_ = " << this->name () << "::_nil ();" << nl;
  *ci << "return val;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  return 0;
}

// Generate the _out definition.
int
be_interface_fwd::gen_out_defn (char *)
{
  TAO_OutStream *ch;
  TAO_NL  nl;
  char namebuf [NAMEBUFSIZE];

  ACE_OS::memset (namebuf, 
                  '\0', 
                  NAMEBUFSIZE);

  ACE_OS::sprintf (namebuf, 
                   "%s_out", 
                   this->local_name ()->get_string ());

  ch = tao_cg->client_header ();

  // Generate the out definition (always in the client header)
  ch->indent (); // start with whatever was our current indent level.

  *ch << "class " << be_global->stub_export_macro ()
      << " " << namebuf << nl;
  *ch << "{" << nl;
  *ch << "public:\n";
  ch->incr_indent ();

  // No default constructor

  // Constructor from a pointer.
  *ch << namebuf << " (" << this->local_name () << "_ptr &);" << nl;

  // Constructor from a _var &.
  *ch << namebuf << " (" << this->local_name () << "_var &);" << nl;

  // Constructor from a _out &.
  *ch << namebuf << " (const " << namebuf << " &);" << nl;

  // Assignment operator from a _out &.
  *ch << namebuf << " &operator= (const " << namebuf << " &);" << nl;

  // Assignment operator from a pointer &, cast operator, ptr fn, operator
  // -> and any other extra operators.
  // Only interface allows assignment from var &.
  *ch << namebuf << " &operator= (const " 
      << this->local_name () << "_var &);" << nl;
  *ch << namebuf << " &operator= (" 
      << this->local_name () << "_ptr);" << nl;

  // cast
  *ch << "operator " << this->local_name () << "_ptr &();" << nl;

  // ptr fn
  *ch << this->local_name () << "_ptr &ptr (void);" << nl;

  // operator ->
  *ch << this->local_name () << "_ptr operator-> (void);" << nl;

  *ch << "\n";
  ch->decr_indent ();
  *ch << "private:\n";
  ch->incr_indent ();
  *ch << this->local_name () << "_ptr &ptr_;\n";

  ch->decr_indent ();
  *ch << "};\n\n";

  return 0;
}

int
be_interface_fwd::gen_out_impl (char *, char *)
{
  TAO_OutStream *ci = 0;
  TAO_NL nl;
  char fname [NAMEBUFSIZE];  // To hold the full and
  char lname [NAMEBUFSIZE];  // local _out names.

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
                   this->local_name ()->get_string ());

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

      // Constructor from a _ptr.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (" 
      << this->name () << "_ptr &p)" << nl;
  *ci << "  : ptr_ (p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = " << this->name () << "::_nil ();\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Constructor from _var &.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (" << this->name () <<
    "_var &p) // constructor from _var" << nl;
  *ci << "  : ptr_ (p.out ())" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "CORBA::release (this->ptr_);" << nl;
  *ci << "this->ptr_ = " << this->name () << "::_nil ();\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Copy constructor.
  ci->indent ();
  *ci << "ACE_INLINE" << nl;
  *ci << fname << "::" << lname << " (const " << fname <<
    " &p) // copy constructor" << nl;
  *ci << "  : ptr_ (ACE_const_cast (" << fname
      << "&,p).ptr_)" << nl;
  *ci << "{}\n\n";

  // Assignment operator from _out &.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (const " << fname <<
    " &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = ACE_const_cast (" << fname << "&,p).ptr_;" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator from _var.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (const " << this->name () <<
    "_var &p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = " << this->name () 
      << "::_duplicate (p.ptr ());" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Assignment operator from _ptr.
  ci->indent ();
  *ci << "ACE_INLINE " << fname << " &" << nl;
  *ci << fname << "::operator= (" << this->name () <<
    "_ptr p)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "this->ptr_ = p;" << nl;
  *ci << "return *this;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // Other extra methods - cast operator ().
  ci->indent ();
  *ci << "ACE_INLINE " << nl;
  *ci << fname << "::operator " << this->name () <<
    "_ptr &() // cast" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // ptr function
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr &" << nl;
  *ci << fname << "::ptr (void) // ptr" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  // operator->
  ci->indent ();
  *ci << "ACE_INLINE " << this->name () << "_ptr " << nl;
  *ci << fname << "::operator-> (void)" << nl;
  *ci << "{\n";
  ci->incr_indent ();
  *ci << "return this->ptr_;\n";
  ci->decr_indent ();
  *ci << "}\n\n";

  return 0;
}

void
be_interface_fwd::destroy (void)
{
  // Do nothing.
}

int
be_interface_fwd::accept (be_visitor *visitor)
{
  return visitor->visit_interface_fwd (this);
}

// Narrowing
IMPL_NARROW_METHODS2 (be_interface_fwd, AST_InterfaceFwd, be_type)
IMPL_NARROW_FROM_DECL (be_interface_fwd)
