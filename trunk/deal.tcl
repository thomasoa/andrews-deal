#
# deal.tcl - this file is sourced at startup by Deal 3.0 or later
#
# Copyright (C) 1996-2001, Thomas Andrews
#
# $Id$
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Mac default value for tcl_library
#set tcl_library /System/Library/Frameworks/Tcl.framework/Versions/8.4/Resources/Scripts

# Windows default value for tcl_library
if {[string first "Windows" $tcl_platform(os)]>=0} {
    set tcl_library C:/tcl/lib/tcl8.5
}

catch { deal_init_tcl }

source lib/features.tcl

# If set to non-zero, and your terminal supports UTF-8, deal's default output format will write 
#the deal with unicode suit symbols.
set deal::unicode 0

# Edit this line to make your default format any format
source format/default
