#pragma once
#include<iostream>
#include <string>
#include <hiredis/hiredis.h>
#include<map>
#include<vector>
#include<mutex>
#include<string.h>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();
    void ReConnect();
    bool Query(const char *redisCmd, std::map<std::string, std::string> &result);
    bool Insert(const char *redisCmd);
    int Exist(const char *redisCmd);
    bool ExeCmd(const char *redisCmd);
    bool Del(const char *redisCmd);
    bool Keys(const char *redisCmd, vector<string> &result);

private:
    redisContext *_redis;
    std::string _ip;
    int _port;
    std::mutex mtx;
};