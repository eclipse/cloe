# - Try to find VTD installation
#
# There are two include directories that are important:
#
# ${VTD_ROOT}/Develop/Framework/inc
#   CoordFr.hh
#   CoordTrans.hh
#   Iface.hh
#   Plugin.hh
#   scpIcd.h
#   ScpParser.hh
#   viRDBIcd.h
#
# ${VTD_ROOT}/Develop/Framework/RDBHandler
#   example.cc
#   RDBHandler.cc
#   RDBHandler.hh

include(FindPackageHandleStandardArgs)

find_path(VTD_ROOT_DIR
    NAMES
        "Develop/Framework/inc/viRDBIcd.h"
    HINTS
        ENV VTD_ROOT
        ENV HOME
    PATHS
        ~/vires
        /opt
        /opt/vires
        /opt/VIRES
        /vires
        /VIRES
        /
    PATH_SUFFIXES
        VTD
        VTD.2.2
        VTD.2.1
        VTD.2.0
)
mark_as_advanced(VTD_ROOT_DIR)

set(VTD_RDBHANDLER_DIR "${VTD_ROOT_DIR}/Develop/Framework/RDBHandler")
mark_as_advanced(VTD_RDBHANDLER_DIR)

find_package_handle_standard_args(VTD
    FOUND_VAR
        VTD_FOUND
    REQUIRED_VARS
        VTD_ROOT_DIR
        VTD_RDBHANDLER_DIR
    FAIL_MESSAGE
        "Could NOT find VTD"
)

set(VTD_INCLUDE_DIRS "${VTD_ROOT_DIR}/Develop/Framework/inc")
set(VTD_RDBHANDLER_INCLUDE_DIRS "${VTD_RDBHANDLER_DIR}")
set(VTD_RDBHANDLER_SOURCES "${VTD_RDBHANDLER_DIR}/RDBHandler.cc")
