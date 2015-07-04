#
#   libcdim - a library for manipulation CBM imagefiles (mainly d64)
# 
#   Copyright (C) [2015]  [Thomas Martens]
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 3 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
#   for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, see <http://www.gnu.org/licenses/>.
#


# Find libcdim.h
#
# Small cmake modul to find the location of libcdim.h file

FIND_PATH(CDIM_INCLUDE_DIR libcdim.h)

FIND_LIBRARY(CDIM_LIBRARIES NAMES cdim)

IF (CDIM_LIBRARIES AND CDIM_INCLUDE_DIR)
	SET(CDIM_FOUND "YES")
ELSE (CDIM_LIBRARIES AND CDIM_INCLUDE_DIR)
	SET(CDOM_FOUND "NO")
ENDIF (CDIM_LIBRARIES AND CDIM_INCLUDE_DIR)

IF (CDIM_FOUND)
	IF (NOT CDIM_FIND_QUIETLY)
		MESSAGE(STATUS "Found cdim library: ${CDIM_LIBRARIES}")
	ENDIF (NOT CDIM_FIND_QUIETLY)
ELSE (CDIM_FOUND)
	IF (CDIM_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find CDIM library")
	ENDIF (CDIM_FIND_REQUIRED)
ENDIF (CDIM_FOUND)

MARK_AS_ADVANCED(CDIM_LIBRARIES CDIM_INCLUDE_DIR)
