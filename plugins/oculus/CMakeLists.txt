#
#  Created by Bradley Austin Davis on 2015/10/25
#  Copyright 2015 High Fidelity, Inc.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http:#www.apache.org/licenses/LICENSE-2.0.html
#

#UTII if (WIN32)
#UTII 
#UTII     # we're using static GLEW, so define GLEW_STATIC
#UTII     add_definitions(-DGLEW_STATIC)
#UTII 
#UTII     set(TARGET_NAME oculus)
#UTII     setup_hifi_plugin(Multimedia)
#UTII     link_hifi_libraries(shared gl gpu gpu-gl controllers ui 
#UTII         plugins ui-plugins display-plugins input-plugins
#UTII         audio-client networking render-utils)
#UTII     
#UTII     include_hifi_library_headers(octree)
#UTII     
#UTII     add_dependency_external_projects(LibOVR)
#UTII     find_package(LibOVR REQUIRED)
#UTII     target_include_directories(${TARGET_NAME} PRIVATE ${LIBOVR_INCLUDE_DIRS})
#UTII     target_link_libraries(${TARGET_NAME} ${LIBOVR_LIBRARIES})
#UTII     target_link_libraries(${TARGET_NAME} Winmm.lib)
#UTII 
#UTII endif()
