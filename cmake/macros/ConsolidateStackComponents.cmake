macro(CONSOLIDATE_STACK_COMPONENTS)

  if (DEFINED DEPLOY_PACKAGE AND DEPLOY_PACKAGE)
    if (WIN32)
      # Copy all the output for this target into the common deployment location
      add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory $<TARGET_FILE_DIR:${TARGET_NAME}> ${CMAKE_BINARY_DIR}/utii-client-deployment
      )

      # Copy icon files for interface and stack manager
      if (TARGET_NAME STREQUAL "interface" OR TARGET_NAME STREQUAL "stack-manager")
        if (TARGET_NAME STREQUAL "interface")
          set (ICON_FILE_PATH "${PROJECT_SOURCE_DIR}/icon/UTII.ico")
          set (ICON_DESTINATION_NAME "UTII.ico")
        elseif (TARGET_NAME STREQUAL "stack-manager")
          set (ICON_FILE_PATH "${PROJECT_SOURCE_DIR}/assets/icon.ico")
          set (ICON_DESTINATION_NAME "stack-manager.ico")
        endif ()
        add_custom_command(
          TARGET ${TARGET_NAME} POST_BUILD
          COMMAND "${CMAKE_COMMAND}" -E copy ${ICON_FILE_PATH} ${CMAKE_BINARY_DIR}/utii-client-deployment/${ICON_DESTINATION_NAME}
        )
      endif ()
    endif ()
  endif ()

endmacro()