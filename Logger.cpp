#include "Logger.h"

#include <iostream>
//获取日志唯一的实例对象
Logger& Logger::instance()
{
    static Logger logger;//局部静态变量的**初始化过程**由编译器保证，多线程同时进入这个函数时，只会构造一次，其他线程会阻塞等待初始化完成
    return logger;
}
 //设置日志级别
void Logger::setLogLevel(int Level)
{
    logLeveal_=Level;    
}
//写日志 [级别信息] time :msg
void Logger::log(std::string msg)
{
    switch(logLeveal_)
    {
        case INFO:
            std::cout<<"[INFO]";
            break;
        case ERROR:
            std::cout<<"[INFO]";
            break;
        case FATAL:
            std::cout<<"[INFO]";
            break;
        case DEBUG:
            std::cout<<"[INFO]";
            break;
        default:
            break;    
    }

    //打印时间和msg
    std::cout<<"print time"<<":"<<msg;
}