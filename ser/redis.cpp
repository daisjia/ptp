#include"redis.h"

Redis::~Redis()
{
    redisFree(_redis);
    _redis = nullptr;
}

Redis::Redis()
{
    _redis = redisConnect("127.0.0.1", 6379);
    if (_redis->err)
    {
        redisFree(_redis);
        cout<<"connect to reidsServer fail!"<<endl;
        exit(0);
    }
}

void Redis::ReConnect()
{
    if (_redis != nullptr)
    {
        redisFree(_redis);
    }
    _redis = redisConnect(_ip.c_str(), _port);
    if (_redis->err)
    {
        redisFree(_redis);
        cout << "ReConnect to reidsServer fail!" << endl;
        exit(0);
    }
}

bool Redis::Query(const char *redisCmd, std::map<std::string, std::string> &result)
{
    std::unique_lock<std::mutex> lok(mtx);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PING");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
    {
        if (strcasecmp(reply->str, "PONG"))
        {
            ReConnect();
        }
    }
    else
    {
        redisFree(_redis);
        cout<<"Query connect to reidsServer fail!"<<endl;
        return false;
    }
    freeReplyObject(reply);
    reply = (redisReply *)redisCommand(_redis, redisCmd);
    if (reply == NULL)
    {
        cout << "redisCommand error!" << endl;
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis exe error!" << endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (int i = 0; i < reply->elements;)
        {
            result.insert(std::make_pair(reply->element[i]->str, reply->element[i + 1]->str));
            i += 2;
        }
        freeReplyObject(reply);
        return true;
    }

    if (reply->type == REDIS_REPLY_NIL)
    {
        result.clear();
        freeReplyObject(reply);
        return true;
    }
    return false;
}

bool Redis::Keys(const char *redisCmd, vector<string> &result)
{
    std::unique_lock<std::mutex> lok(mtx);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PING");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
    {
        if (strcasecmp(reply->str, "PONG"))
        {
            ReConnect();
        }
    }
    else
    {
        redisFree(_redis);
        cout << "Query connect to reidsServer fail!" << endl;
        return false;
    }
    freeReplyObject(reply);
    reply = (redisReply *)redisCommand(_redis, redisCmd);
    if (reply == NULL)
    {
        cout << "redisCommand error!" << endl;
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis exe error!" << endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (int i = 0; i < reply->elements; i++)
        {
            result.push_back(reply->element[i]->str);
        }
        freeReplyObject(reply);
        return true;
    }

    if (reply->type == REDIS_REPLY_NIL)
    {
        result.clear();
        freeReplyObject(reply);
        return true;
    }
    return false;
}

bool Redis::Insert(const char *redisCmd)
{
    std::unique_lock<std::mutex> lok(mtx);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PING");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
    {
        if (strcasecmp(reply->str, "PONG"))
        {
            ReConnect();
        }
    }
    else
    {
        redisFree(_redis);
        cout << "Insert connect to reidsServer fail!" << endl;
        exit(0);
    }
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(_redis, redisCmd);
    if (reply == NULL)
    {
        cout << "redisCommand error!" << endl;
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis command fail!" << endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type == REDIS_REPLY_STATUS)
    {
        if (!strcasecmp(reply->str, "ok"))
        {
            freeReplyObject(reply);
            return true;
        }
        else
        {
            freeReplyObject(reply);
            return false;
        }
    }

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        if (reply->integer == 0)
        {
            freeReplyObject(reply);
            return false;
        }
        else
        {
            freeReplyObject(reply);
            return true;
        }
    }
    freeReplyObject(reply);
    return false;
}

int Redis::Exist(const char *redisCmd)
{
    std::unique_lock<std::mutex> lok(mtx);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PING");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
    {
        if (strcasecmp(reply->str, "PONG"))
        {
            ReConnect();
        }
    }
    else
    {
        redisFree(_redis);
        cout << "Exist connect to reidsServer fail!" << endl;
        exit(0);
    }
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(_redis, redisCmd);
    if (reply == NULL)
    {
        cout << "redisCommand error!" << endl;
        return -1;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis command fail!" << endl;
        freeReplyObject(reply);
        return -1;
    }

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        if (reply->integer == 0)
        {
            freeReplyObject(reply);
            return 0;
        }
        else
        {
            freeReplyObject(reply);
            return 1;
        }
    }
    freeReplyObject(reply);
    return -1;
}

bool Redis::ExeCmd(const char *redisCmd)
{
    std::unique_lock<std::mutex> lok(mtx);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PING");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
    {
        if (strcasecmp(reply->str, "PONG"))
        {
            ReConnect();
        }
    }
    else
    {
        redisFree(_redis);
        cout << "Insert connect to reidsServer fail!" << endl;
        exit(0);
    }
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(_redis, redisCmd);
    if (reply == NULL)
    {
        cout << "redisCommand error!" << endl;
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis command fail!" << endl;
        freeReplyObject(reply);
        return false;
    }

    return true;
}

bool Redis::Del(const char *redisCmd)
{
    std::unique_lock<std::mutex> lok(mtx);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PING");
    if (reply != nullptr && reply->type == REDIS_REPLY_STATUS)
    {
        if (strcasecmp(reply->str, "PONG"))
        {
            ReConnect();
        }
    }
    else
    {
        redisFree(_redis);
        cout << "Delete connect to reidsServer fail!" << endl;
        exit(0);
    }
    freeReplyObject(reply);
    reply = (redisReply *)redisCommand(_redis, redisCmd);
    if (reply == NULL)
    {
        cout << "redisCommand error!" << endl;
        return false;
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis command fail!" << endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type == REDIS_REPLY_INTEGER)
    {
        if (reply->integer == 0)
        {
            freeReplyObject(reply);
            return false;
        }
        else
        {
            freeReplyObject(reply);
            return true;
        }
    }
    freeReplyObject(reply);
    return false;
}
