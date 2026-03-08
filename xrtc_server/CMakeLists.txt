project(xrtc_server)
cmake_minimum_required(VERSION 3.10)

# add_definitions("-g -pipe -W -Wall -fPIC -std=gnu++14 -DWEBRTC_MAC -DWEBRTC_POSIX -DNDEBUG -Wno-unused-parameter")
# 编译选项：针对Mac M3（ARM64架构）优化
add_compile_options(
    -g                  # 保留调试信息
    -pipe               # 管道编译，加速编译过程
    -Wall               # 开启所有警告（便于发现问题）
    -Wextra             # 额外警告
    -fPIC               # 生成位置无关代码（动态库必要）
    -arch arm64         # 明确指定M3的ARM64架构（关键！）
    -march=armv8.5-a    # 针对M3的ARM架构版本优化
    -Wno-unused-parameter  # 忽略未使用参数的警告
    -DNDEBUG            # 关闭调试宏（发布模式）
    -std=c++14          # 强制启用C++14标准（解决jsoncpp链接问题）
    -Wthread-safety     # 启用 GCC 的线程安全检查（可选，视编译器而定
)

# WebRTC必要的宏定义（Mac环境适配）
add_definitions(
    -DWEBRTC_MAC        # 标识Mac系统
    -DWEBRTC_POSIX      # POSIX标准兼容（Mac属于POSIX系统）
)

# 查找 OpenSSL (使用系统默认版本，并通过 openssl_compat.cpp 提供旧 API 兼容)
find_package(OpenSSL REQUIRED)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    "./src"
    "./thirdlib/yaml-cpp/out/include"
    "./thirdlib/rtcbase_v1/src"
    "./thirdlib/rtcbase_v1/third_party/include"
    "./thirdlib/libev/configDir/include"
    "./thirdlib/jsoncpp/include"
)

set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)

link_directories(
    "./thirdlib/rtcbase_v1/out"
    "./thirdlib/yaml-cpp/out/lib"
    "./thirdlib/libev/configDir/lib"
    "./thirdlib/jsoncpp/build/lib"
)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/base/*.cpp" "src/server/*.cpp")

# 查找 OpenSSL (使用系统默认版本，并通过 openssl_compat.cpp 提供旧 API 兼容)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(xrtc_server ${SRC_FILES})

target_link_libraries(xrtc_server "-framework CoreFoundation"
libyaml-cpp.a librtcbase.a libev.a libjsoncpp.a
${OPENSSL_LIBRARIES}
)
