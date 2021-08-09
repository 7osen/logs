#pragma once
#include <string>
using std::string;
const string Filepath = "/home/log/";

const long MaxFileSize = 1024 * 1024 * 1024;

const int MaxFileNum = 64;

const int EveryFlush = 16;

const int LruCacheSize = 24;

const int BlockSize = 16;

const int MatchTimes = 1<<16;

const int MaxFindNum = 1<<20;