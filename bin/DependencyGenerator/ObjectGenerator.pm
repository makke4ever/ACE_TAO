package ObjectGenerator;

# ************************************************************
# Description   : Base class for all Object Generators.
# Author        : Chad Elliott
# Create Date   : 5/23/2003
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

# ************************************************************
# Subroutine Section
# ************************************************************

sub new {
  my($class) = shift;
  return bless {
               }, $class;
}


sub process {
  #my($self) = shift;
  #my($file) = shift;
  return ();
}


1;
