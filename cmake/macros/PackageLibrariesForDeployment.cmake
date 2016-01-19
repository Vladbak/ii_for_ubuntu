# 
#  PackageLibrariesForDeployment.cmake
#  cmake/macros
# 
#  Copyright 2015 High Fidelity, Inc.
#  Created by Stephen Birarda on February 17, 2014
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 

macro(PACKAGE_LIBRARIES_FOR_DEPLOYMENT)
  
  if (WIN32)
    configure_file(
      ${HIFI_CMAKE_DIR}/templates/FixupBundlePostBuild.cmake.in  
      ${CMAKE_CURRENT_BINARY_DIR}/FixupBundlePostBuild.cmake
      @ONLY
    )
    
    set(PLUGIN_PATH "plugins")
    
    # add a post-build command to copy DLLs beside the executable
    add_custom_command(
      TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND}
        -DBUNDLE_EXECUTABLE=$<TARGET_FILE:${TARGET_NAME}>
        -DBUNDLE_PLUGIN_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/${PLUGIN_PATH}
        -P ${CMAKE_CURRENT_BINARY_DIR}/FixupBundlePostBuild.cmake
    )
    
    find_program(WINDEPLOYQT_COMMAND windeployqt PATHS ${QT_DIR}/bin NO_DEFAULT_PATH)
    
    if (NOT WINDEPLOYQT_COMMAND)
      message(FATAL_ERROR "Could not find windeployqt at ${QT_DIR}/bin. windeployqt is required.")
    endif ()
  
    # add a post-build command to call windeployqt to copy Qt plugins
    add_custom_command(
      TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND CMD /C "SET PATH=%PATH%;${QT_DIR}/bin && ${WINDEPLOYQT_COMMAND} ${EXTRA_DEPLOY_OPTIONS} $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>:--release> \"$<TARGET_FILE:${TARGET_NAME}>\""
    )
  elseif (DEFINED BUILD_BUNDLE AND BUILD_BUNDLE AND APPLE)
    find_program(MACDEPLOYQT_COMMAND macdeployqt PATHS ${QT_DIR}/bin NO_DEFAULT_PATH)
    
    if (NOT MACDEPLOYQT_COMMAND)
      message(FATAL_ERROR "Could not find macdeployqt at ${QT_DIR}/bin. macdeployqt is required.")
    endif ()
  
    if (NOT APP_NAME)
        set(APP_NAME TARGET_NAME)
    endif ()
    # add a post-build command to call macdeployqt to copy Qt plugins
    add_custom_command(
      TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${MACDEPLOYQT_COMMAND} ${CMAKE_CURRENT_BINARY_DIR}/\${CONFIGURATION}/${APP_NAME}.app -verbose 0
    )
  endif ()
endmacro()
