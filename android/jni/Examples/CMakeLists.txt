set(headers ${headers} Examples
)
set(currsources
)
set(sources ${sources} ${currsources})
include(jni/Examples/AndroidExampleControllerView/CMakeLists.txt)
include(jni/Examples/JavaHudCrossThreadCommunication/CMakeLists.txt)
include(jni/Examples/PinsWithAttachedJavaUI/CMakeLists.txt)
include(jni/Examples/PositionJavaPinButton/CMakeLists.txt)
include(jni/Examples/RouteMatching/CMakeLists.txt)
include(jni/Examples/RouteSimulation/CMakeLists.txt)
include(jni/Examples/ShowJavaPlaceJumpUI/CMakeLists.txt)