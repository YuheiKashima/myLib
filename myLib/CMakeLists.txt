option(MYLIB_BUILD_SHARED_LIBS "Build myLib as a shared library" OFF)

IF(MYLIB_BUILD_SHARED_LIBS)
add_library(myLibsrc SHARED
    CPUPerfCounter.cpp
    CPUSupportChecker.cpp
    InstantInput.cpp
    Log.cpp
    ThreadPool.cpp
    TimeStamp.cpp
)
else()
add_library(myLibsrc STATIC
    CPUPerfCounter.cpp
    CPUSupportChecker.cpp
    InstantInput.cpp
    Log.cpp
    ThreadPool.cpp
    TimeStamp.cpp
)
endif()
