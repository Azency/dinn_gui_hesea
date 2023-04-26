cmake_minimum_required(VERSION 3.5)

project(dinn_hesea LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/home/azency/Qt5.12.10/5.12.10/gcc_64")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Core Gui WebEngineWidgets WebChannel REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui WebEngineWidgets WebChannel REQUIRED)

# link hesea lib
include_directories(${CMAKE_SOURCE_DIR}/src/libs/ubuntu/include)
include_directories(${CMAKE_SOURCE_DIR}/src/libs/ubuntu/include/hesea)
include_directories(${CMAKE_SOURCE_DIR}/src/libs/ubuntu/include/hesea/pke)
include_directories(${CMAKE_SOURCE_DIR}/src/libs/ubuntu/include/hesea/core)
include_directories(${CMAKE_SOURCE_DIR}/src/libs/ubuntu/include/hesea/binfhe)
include_directories(${CMAKE_SOURCE_DIR}/src/libs/ubuntu/include/hesea/cereal)


# set(SOURCE_FILES src/*.cpp src/*.h src/*.ui)
aux_source_directory(./src srcs)
add_executable(dinn_hesea
  ${srcs}
)

target_link_libraries(dinn_hesea 
PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::WebChannel
${CMAKE_SOURCE_DIR}/src/libs/ubuntu/lib/libHESEApke.so
${CMAKE_SOURCE_DIR}/src/libs/ubuntu/lib/libHESEAcore.so
${CMAKE_SOURCE_DIR}/src/libs/ubuntu/lib/libHESEAbinfhe.so)
