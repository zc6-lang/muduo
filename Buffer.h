#pragma once

#include <vector>
#include <string>

//网络库底层的缓冲区类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend=8;
    static const size_t kInitialSize=1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        :buffer_(kCheapPrepend+initialSize)
        ,readerIndex_(kCheapPrepend)
        ,writerIndex_(kCheapPrepend)
    {

    }

    size_t readableBytes() const
    {
        return writerIndex_-readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size()-writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }
private:
    char* begin()
    {
        return &(*buffer_.begin());//vector底层数组首元素的地址，也就是数组的起始地址
    }

    const char* begin()const
    {
        return &(*buffer_.begin());
    }

    void makeSpace(size_t len)
    {
        if(writableBytes()+prependableBytes()<len+kCheapPrepend)
        {
            buffer_.resize(writerIndex_+len);
        }
        else
        {
            size_t readable=readableBytes();//未读数据
            std::copy(begin()+readerIndex_,begin()+writerIndex_,begin()+kCheapPrepend);
            readerIndex_=kCheapPrepend;
            writerIndex_=readerIndex_+readable;
        }
    }

public:

    //返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin()+readerIndex_;
    } 

    void retrieve(size_t len)
    {
        if(len<readableBytes())
        {
            readerIndex_+=len;
        }
        else retrieveAll();
    }

    void retrieveAll()
    {
        readerIndex_=writerIndex_=kCheapPrepend;
    }

    //把onMessage函数上报的buffer数据，转成string类型的数据返回
    std::string retrieveAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(),len);
        retrieve(len);
        return result;
    }

    void ensureWriteableBytes(size_t len)
    {
        if(writableBytes()<len)
        {
            makeSpace(len);//扩容函数
        }
    }
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};