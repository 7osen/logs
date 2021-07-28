# CMakeList.txt: LogServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project(LogServer)


add_definitions(-std=c++11)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)


# 将源代码添加到此项目的可执行文件。
add_executable (LogServer "main.cpp"  "base/memtable.hpp" "base/skiplist.hpp" "base/timecount.hpp" "base/logfile.hpp" "base/storager.hpp" "base/setting.hpp" "net/Channel.hpp" "net/CellServer.hpp" "net/Epoller.hpp" "net/TcpConnection.hpp" "base/buffer.hpp" "net/TcpServer.hpp" "net/HttpHeader.hpp" "net/HttpServer.hpp" "net/LogServer.hpp" "base/lock_free_queue.hpp" "base/message.hpp" "base/noncopyable.hpp")

target_link_libraries(LogServer Threads::Threads)

# TODO: 如有需要，请添加测试并安装目标。
