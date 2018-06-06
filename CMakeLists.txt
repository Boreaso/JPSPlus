cmake_minimum_required(VERSION 3.10)
project(JPSPlus)

#set(CMAKE_CXX_STANDARD 11)


#set(CMAKE_CXX_FLAGS
#        "${CMAKE_CXX_FLAGS} \
#        -g -std=c++11 \
#        -fPIC \
#        -lboost_python36 \
#        "
#        )

set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
        -g -std=c++11 \
        -fPIC \
        "
        )

# 设置变量，表示所有的源文件
set(SOURCE_FILES
        BucketPriorityQueue.cpp
        FastStack.cpp
        JPSPlus.cpp
        PrecomputeMap.cpp
        SimpleUnsortedPriorityQueue.cpp
        stdafx.cpp
        UnsortedPriorityQueue.cpp
        Wrapper.cpp
        WrapperInterface.cpp
        Map.cpp
        Timer.cpp
        main.cpp
        )

# BOOST_LIBRARIES使用静态库保证没有boost.python环境下运行
set(BOOST_LIBRARIES
        libboost_numpy36.a
#        libboost_numpy36.so
        libboost_python36.a
#        libboost_python36.so
        )

set(PYTHON_LIBRARIES
        libpython3.6m.a
#        libpython3.6m.so
        )

# 预处理器定义
add_definitions(-BOOST_PYTHON_STATIC_LIB) # 包含Boost.Python库

# 相关头文件的目录
include_directories(
#        /usr/include/python3.6
#        /usr/include/x86_64-linux-gnu
        /home/boreas/anaconda3/include/python3.6m
        /home/boreas/software/boost_1_67_0/build/include
)

# 相关库文件的目录
link_directories(
#        /usr/lib/python3.6/config-3.6m-x86_64-linux-gnu
#        /home/boreas/anaconda3/lib
        /usr/lib/x86_64-linux-gnu
#        /home/boreas/software/PythonLib/python_install/lib
#        /home/boreas/software/boost_1_67_0/build/lib
)

# 链接库名
link_libraries(
#        libc.a
#        ${BOOST_LIBRARIES}
#        ${PYTHON_LIBRARIES}
)

#生成静态链接库libjpsp.a
#add_library(jpsp ${SOURCE_FILES})

#生成动态链接库libjpsp.so
add_library(jpsp SHARED ${SOURCE_FILES})

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# 依赖的库文件
#target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES} ${BOOST_LIBRARIES})


#表示生成的执行文件所在路径
#set_target_properties(utility PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY> "${PROJECT_SOURCE_DIR}/lib")