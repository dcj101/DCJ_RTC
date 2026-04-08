cmake_minimum_required (VERSION 3.5)

project(rtcbase)

include_directories(
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/third_party/include"
)

link_directories(
    "${PROJECT_SOURCE_DIR}/third_party/lib"
)

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
    -std=c++17          # 强制启用C++17标准（解决>>语法问题的核心！）
    -Wthread-safety     # 启用 GCC 的线程安全检查（可选，视编译器而定
)

# WebRTC必要的宏定义（Mac环境适配）
add_definitions(
    -DWEBRTC_MAC        # 标识Mac系统
    -DWEBRTC_POSIX      # POSIX标准兼容（Mac属于POSIX系统）
)

file(GLOB all_src
    "./src/rtc_base/*.cc"
    "./src/rtc_base/strings/*.cc"
    "./src/rtc_base/synchronization/*.cc"
    "./src/rtc_base/deprecated/*.cc"
    "./src/rtc_base/internal/*.cc"
    "./src/rtc_base/task_utils/*.cc"
    "./src/rtc_base/third_party/sigslot/*.cc"
    "./src/rtc_base/third_party/base64/*.cc"
    "./src/api/task_queue/*.cc"
    "./src/api/units/*.cc"
    "./src/api/crypto/*.cc"
    "./src/system_wrappers/source/*.cc"
)

add_library(rtcbase ${all_src})

target_sources(rtcbase PRIVATE "src/rtc_base/system/cocoa_threading.mm")
target_link_libraries(rtcbase libssl.a libcrypto.a)


