# ************** <auto-copyright.pl BEGIN do not edit this line> **************
# Doozer++ is (C) Copyright 2000-2004 by Iowa State University
#
# Original Author:
#   Patrick Hartling
# -----------------------------------------------------------------------------
# VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          dpp.obj-common.mk,v
# Date modified: 2004/01/29 04:28:00
# Version:       1.20
# -----------------------------------------------------------------
# *************** <auto-copyright.pl END do not edit this line> ***************

# =============================================================================
# dpp.obj-common.mk,v 1.20 2004/01/29 04:28:00 patrickh Exp
#
# This file <dpp.obj-subdir.mk> is intended to be used by makefiles that need
# to build object files AND to recurse into subdirectories.  It defines
# variables and targets that are common to all such makefiles.  It also
# includes other .mk files that are typically necessary for this process.
# -----------------------------------------------------------------------------
# The makefile including this file must define the following variables:
#
# srcdir                - The directory containing the source files to compile.
# OBJEXT                - The suffix used for object files (usually .o or
#                         .obj).
# SRCS                  - The C++ sources to be compiled.
# JAVA_SRCS             - The Java sources to be compiled.
# MKINSTALLDIRS         - Path to shell script for making directories.
# C_WARNS_LEVEL_0       - C warning level 0.  This would disable warnings.
# C_WARNS_LEVEL_[1-5]   - C warning levels 1 through 5.  Each level should be
#                         more strict than the previous.
# CXX_WARNS_LEVEL_0     - C++ warning level 0.  This would disable warnings.
# CXX_WARNS_LEVEL_[1-5] - C++ warning levels 1 through 5.  Each level should be
#                         more strict than the previous.
#
# Optionally, it can define the following variables for added functionality:
#
# EXTRA_SRCS_PATH - Directories besides $(srcdir) where source files may be
#                   found.
# OBJDIR          - The directory where the object files will go.  This
#                   defaults to the current directory.
# WARNS           - The warning level for all compiled code.  If not specified,
#                   the warning level defaults to 0 (no warnings).
# C_WARNS         - The warning level for C code.  If not specified, this
#                   defaults to the value of $(WARNS).
# CXX_WARNS       - The warning level for C++ code.  If not specified, this
#                   defaults to the value of $(WARNS).
# IDL_CPP_SRCS    - The C++ source files generated by the IDL compiler that
#                   need to be compiled with the other sources.
# IDL_JAVA_SRCS   - The Java source files generated by the Java IDL compiler
#                   that need to be compiled with the other sources.
# =============================================================================

ifdef BASE_OBJDIR
   OBJDIR=	$(BASE_OBJDIR)
else
   OBJDIR?=	.
endif

ifneq ($(OBJDIR), .)
   ifdef SUBOBJDIR
      OBJDIR:=	$(OBJDIR)/$(SUBOBJDIR)
   endif
endif

# Set the default warning levels for C and C++.  The variable $(WARNS) is for
# coarse-grained settings.
WARNS?=		0
C_WARNS?=	$(WARNS)
CXX_WARNS?=	$(WARNS)

ifeq ($(C_WARNS), 0)
   C_WARN_FLAGS=		$(C_WARNS_LEVEL_0)
else
   ifeq ($(C_WARNS), 1)
      C_WARN_FLAGS=		$(C_WARNS_LEVEL_1)
   else
      ifeq ($(C_WARNS), 2)
         C_WARN_FLAGS=		$(C_WARNS_LEVEL_2)
      else
         ifeq ($(C_WARNS), 3)
            C_WARN_FLAGS=	$(C_WARNS_LEVEL_3)
         else
            ifeq ($(C_WARNS), 4)
               C_WARN_FLAGS=	$(C_WARNS_LEVEL_4)
            else
               C_WARN_FLAGS=	$(C_WARNS_LEVEL_5)
            endif  # C_WARNS == 4
         endif  # C_WARNS == 3
      endif  # C_WARNS == 2
   endif  # C_WARNS == 1
endif  # C_WARNS == 0

ifeq ($(CXX_WARNS), 0)
   CXX_WARN_FLAGS=		$(CXX_WARNS_LEVEL_0)
else
   ifeq ($(CXX_WARNS), 1)
      CXX_WARN_FLAGS=		$(CXX_WARNS_LEVEL_1)
   else
      ifeq ($(CXX_WARNS), 2)
         CXX_WARN_FLAGS=	$(CXX_WARNS_LEVEL_2)
      else
         ifeq ($(CXX_WARNS), 3)
            CXX_WARN_FLAGS=	$(CXX_WARNS_LEVEL_3)
         else
            ifeq ($(CXX_WARNS), 4)
               CXX_WARN_FLAGS=	$(CXX_WARNS_LEVEL_4)
            else
               CXX_WARN_FLAGS=	$(CXX_WARNS_LEVEL_5)
            endif  # CXX_WARNS == 4
         endif  # CXX_WARNS == 3
      endif  # CXX_WARNS == 2
   endif  # CXX_WARNS == 1
endif  # CXX_WARNS == 0

vpath %.h $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.hh $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.H $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.c $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.C $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.CC $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.cc $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.cpp $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.c++ $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.cxx $(srcdir) $(EXTRA_SRCS_PATH)
vpath %.$(OBJEXT) $(OBJDIR)
vpath %.java $(srcdir) $(EXTRA_JAVA_SRCS_PATH)
vpath %.class $(srcdir) $(CLASSDIR)

# Define the list of supported source file suffixes.
_suffix_list=	c C CC cc cpp c++ cxx

makeobjs=	$(filter %.$(OBJEXT), $(foreach _suffix, $(_suffix_list), $(1:.$(_suffix)=.$(OBJEXT))))

BASIC_OBJECTS=	$(call makeobjs, $(IDL_CPP_SRCS)) $(call makeobjs, $(SRCS))
BASIC_CLASSES=	$(IDL_JAVA_SRCS:.java=.class) $(JAVA_SRCS:.java=.class)

OBJECTS=	$(addprefix $(OBJDIR)/, $(BASIC_OBJECTS))
CLASSES=	$(addprefix $(OBJDIR)/, $(BASIC_CLASSES))

do-build: $(OBJECTS) $(TARGETS)

do-java-build: $(CLASSES) $(JAVA_TARGETS)

do-beforebuild:
	@$(SHELL) $(MKINSTALLDIRS) $(OBJDIR)
