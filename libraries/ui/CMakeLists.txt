set(TARGET_NAME ui)
setup_hifi_library(OpenGL Network Qml Quick Script WebChannel WebSockets XmlPatterns)
link_hifi_libraries(shared networking gl)

# UTII we need this for reloading the scripts on role-change:
include_hifi_library_headers(script-engine)
