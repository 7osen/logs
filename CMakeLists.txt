# CMakeList.txt: LogServer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
add_executable (LogServer "LogServer.cpp" "LogServer.hpp" "base/memkv.hpp" "base/skiplist.hpp" "base/timecount.hpp" "base/logfile.hpp" "base/kv.hpp" "base/setting.hpp" "net/Channel.hpp" "net/CellServer.hpp" "net/Epoller.hpp")

# TODO: 如有需要，请添加测试并安装目标。
