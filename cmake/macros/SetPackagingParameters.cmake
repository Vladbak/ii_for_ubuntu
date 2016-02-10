#
#  SetPackagingParameters.cmake
#  cmake/macros
#
#  Created by Leonardo Murillo on 07/14/2015.
#  Copyright 2015 High Fidelity, Inc.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

# This macro checks some Jenkins defined environment variables to determine the origin of this build
# and decides how targets should be packaged.

macro(SET_PACKAGING_PARAMETERS)
  set(PR_BUILD 0)
  set(PRODUCTION_BUILD 0)
  set(DEV_BUILD 0)

  set(RELEASE_TYPE $ENV{RELEASE_TYPE})
  set(RELEASE_NUMBER $ENV{RELEASE_NUMBER})

  if (RELEASE_TYPE STREQUAL "PRODUCTION")
    set(DEPLOY_PACKAGE TRUE)
    set(PRODUCTION_BUILD 1)
    set(BUILD_VERSION ${RELEASE_NUMBER})
    set(BUILD_ORGANIZATION "Infinity Island")
    set(HIGH_FIDELITY_PROTOCOL "utii")
    set(INTERFACE_BUNDLE_NAME "Infinity Island")
  elseif (RELEASE_TYPE STREQUAL "PR")
    set(DEPLOY_PACKAGE TRUE)
    set(PR_BUILD 1)
    set(BUILD_VERSION "PR${RELEASE_NUMBER}")
    set(BUILD_ORGANIZATION "Infinity Island - ${BUILD_VERSION}")
    set(INTERFACE_BUNDLE_NAME "Infinity Island")
  else ()
    set(DEV_BUILD 1)
    set(BUILD_VERSION "dev")
    set(BUILD_ORGANIZATION "Infinity Island - ${BUILD_VERSION}")
    set(INTERFACE_BUNDLE_NAME "Infinity Island")
  endif ()
  set(INTERFACE_ICON_PREFIX "UTII")

  if (APPLE)
    set(DMG_SUBFOLDER_NAME "${BUILD_ORGANIZATION}")

    set(ESCAPED_DMG_SUBFOLDER_NAME "")
    string(REPLACE " " "\\ " ESCAPED_DMG_SUBFOLDER_NAME ${DMG_SUBFOLDER_NAME})

    set(DMG_SUBFOLDER_ICON "${HF_CMAKE_DIR}/installer/install-folder.rsrc")

#UTII     set(CONSOLE_INSTALL_DIR ${DMG_SUBFOLDER_NAME})
    set(INTERFACE_INSTALL_DIR ${DMG_SUBFOLDER_NAME})

#UTII     set(CONSOLE_EXEC_NAME "Server Console.app")
#UTII     set(CONSOLE_INSTALL_APP_PATH "${CONSOLE_INSTALL_DIR}/${CONSOLE_EXEC_NAME}")

#UTII     set(CONSOLE_APP_CONTENTS "${CONSOLE_INSTALL_APP_PATH}/Contents")
    set(COMPONENT_APP_PATH "${CONSOLE_APP_CONTENTS}/MacOS/Components.app")
    set(COMPONENT_INSTALL_DIR "${COMPONENT_APP_PATH}/Contents/MacOS")

    set(INTERFACE_INSTALL_APP_PATH "${CONSOLE_INSTALL_DIR}/${INTERFACE_BUNDLE_NAME}.app")
    set(INTERFACE_ICON_FILENAME "${INTERFACE_ICON_PREFIX}.icns")
  else ()
#UTII     if (WIN32)
#UTII       set(CONSOLE_INSTALL_DIR "server-console")
#UTII     else ()
#UTII       set(CONSOLE_INSTALL_DIR ".")
#UTII     endif ()

    set(COMPONENT_INSTALL_DIR ".")
    set(INTERFACE_INSTALL_DIR ".")
  endif ()

  if (WIN32)
    set(INTERFACE_EXEC_PREFIX "Infinity Island")
    set(INTERFACE_ICON_FILENAME "${INTERFACE_ICON_PREFIX}.ico")

#UTII     set(CONSOLE_EXEC_NAME "server-console.exe")

#UTII     set(DS_EXEC_NAME "domain-server.exe")
#UTII     set(AC_EXEC_NAME "assignment-client.exe")

    # shortcut names
    if (PRODUCTION_BUILD)
      set(INTERFACE_SHORTCUT_NAME "High Fidelity")
#UTII       set(CONSOLE_SHORTCUT_NAME "Server Console")
    else ()
      set(INTERFACE_SHORTCUT_NAME "High Fidelity - ${BUILD_VERSION}")
#UTII       set(CONSOLE_SHORTCUT_NAME "Server Console - ${BUILD_VERSION}")
    endif ()
    # check if we need to find signtool
    if (PRODUCTION_BUILD OR PR_BUILD)
      find_program(SIGNTOOL_EXECUTABLE signtool PATHS "C:/Program Files (x86)/Windows Kits/8.1" PATH_SUFFIXES "bin/x64")

      if (NOT SIGNTOOL_EXECUTABLE)
        message(FATAL_ERROR "Code signing of executables was requested but signtool.exe could not be found.")
      endif ()
    endif ()

    set(GENERATED_UNINSTALLER_EXEC_NAME "Uninstall.exe")
    set(REGISTRY_HKLM_INSTALL_ROOT "Software")
    set(POST_INSTALL_OPTIONS_REG_GROUP "PostInstallOptions")
    set(CLIENT_DESKTOP_SHORTCUT_REG_KEY "ClientDesktopShortcut")
#UTII     set(CONSOLE_DESKTOP_SHORTCUT_REG_KEY "ConsoleDesktopShortcut")
#UTII     set(CONSOLE_STARTUP_REG_KEY "ConsoleStartupShortcut")
    set(LAUNCH_NOW_REG_KEY "LaunchAfterInstall")
  endif ()

  # setup component categories for installer
#UTII   set(DDE_COMPONENT dde)
  set(CLIENT_COMPONENT client)
#UTII   set(SERVER_COMPONENT server)

  # create a header file our targets can use to find out the application version
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/includes")
  configure_file("${HF_CMAKE_DIR}/templates/BuildInfo.h.in" "${CMAKE_BINARY_DIR}/includes/BuildInfo.h")

endmacro(SET_PACKAGING_PARAMETERS)
