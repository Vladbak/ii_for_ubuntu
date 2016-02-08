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
    set(BUILD_ORGANIZATION "High Fidelity")
    set(HIGH_FIDELITY_PROTOCOL "hifi")
    set(INTERFACE_BUNDLE_NAME "High Fidelity")
    set(INTERFACE_ICON_PREFIX "interface")
  elseif (RELEASE_TYPE STREQUAL "PR")
    set(DEPLOY_PACKAGE TRUE)
    set(PR_BUILD 1)
    set(BUILD_VERSION "PR${RELEASE_NUMBER}")
    set(BUILD_ORGANIZATION "High Fidelity - ${BUILD_VERSION}")
    set(INTERFACE_BUNDLE_NAME "High Fidelity")
    set(INTERFACE_ICON_PREFIX "interface-beta")
  else ()
    set(DEV_BUILD 1)
    set(BUILD_VERSION "dev")
    set(BUILD_ORGANIZATION "High Fidelity - ${BUILD_VERSION}")
    set(INTERFACE_BUNDLE_NAME "Interface")
    set(INTERFACE_ICON_PREFIX "interface-beta")
  endif ()

  if (APPLE)
    set(DMG_SUBFOLDER_NAME "${BUILD_ORGANIZATION}")

    set(ESCAPED_DMG_SUBFOLDER_NAME "")
    string(REPLACE " " "\\ " ESCAPED_DMG_SUBFOLDER_NAME ${DMG_SUBFOLDER_NAME})

    set(DMG_SUBFOLDER_ICON "${HF_CMAKE_DIR}/installer/install-folder.rsrc")

    set(CONSOLE_INSTALL_DIR ${DMG_SUBFOLDER_NAME})
    set(INTERFACE_INSTALL_DIR ${DMG_SUBFOLDER_NAME})

    set(CONSOLE_EXEC_NAME "Server Console.app")
    set(CONSOLE_INSTALL_APP_PATH "${CONSOLE_INSTALL_DIR}/${CONSOLE_EXEC_NAME}")

    set(CONSOLE_APP_CONTENTS "${CONSOLE_INSTALL_APP_PATH}/Contents")
    set(COMPONENT_APP_PATH "${CONSOLE_APP_CONTENTS}/MacOS/Components.app")
    set(COMPONENT_INSTALL_DIR "${COMPONENT_APP_PATH}/Contents/MacOS")

    set(INTERFACE_INSTALL_APP_PATH "${CONSOLE_INSTALL_DIR}/${INTERFACE_BUNDLE_NAME}.app")
    set(INTERFACE_ICON_FILENAME "${INTERFACE_ICON_PREFIX}.icns")
  else ()
    if (WIN32)
      set(CONSOLE_INSTALL_DIR "server-console")
    else ()
      set(CONSOLE_INSTALL_DIR ".")
    endif ()

    set(COMPONENT_INSTALL_DIR ".")
    set(INTERFACE_INSTALL_DIR ".")
  endif ()

  if (WIN32)
    set(INTERFACE_EXEC_PREFIX "interface")
    set(INTERFACE_ICON_FILENAME "${INTERFACE_ICON_PREFIX}.ico")

    set(CONSOLE_EXEC_NAME "server-console.exe")

    set(DS_EXEC_NAME "domain-server.exe")
    set(AC_EXEC_NAME "assignment-client.exe")

    # shortcut names
    if (PRODUCTION_BUILD)
      set(INTERFACE_SHORTCUT_NAME "High Fidelity")
      set(CONSOLE_SHORTCUT_NAME "Server Console")
    else ()
      set(INTERFACE_SHORTCUT_NAME "High Fidelity - ${BUILD_VERSION}")
      set(CONSOLE_SHORTCUT_NAME "Server Console - ${BUILD_VERSION}")
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
    set(CONSOLE_DESKTOP_SHORTCUT_REG_KEY "ConsoleDesktopShortcut")
    set(CONSOLE_STARTUP_REG_KEY "ConsoleStartupShortcut")
    set(LAUNCH_NOW_REG_KEY "LaunchAfterInstall")
  endif ()

  # setup component categories for installer
  set(DDE_COMPONENT dde)
  set(CLIENT_COMPONENT client)
  set(SERVER_COMPONENT server)

  # create a header file our targets can use to find out the application version
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/includes")
  configure_file("${HF_CMAKE_DIR}/templates/BuildInfo.h.in" "${CMAKE_BINARY_DIR}/includes/BuildInfo.h")

endmacro(SET_PACKAGING_PARAMETERS)
