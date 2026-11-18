#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>

//从fd上读取数据
ssize_t Buffer::readFd(int fd,int *saveErrno)
{
    char extrabuf[65536]={0};//栈上的内存空间

    struct iovec vec[2];

    const size_t writable=writableBytes();//Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base=begin()+writerIndex_;//第一个缓冲区的首元素地址
    vec[0].iov_len=writable;//第一个缓冲区的大小

    vec[1].iov_base=extrabuf;//第二个缓冲区的首元素地址
    vec[1].iov_len=sizeof extrabuf;//第二个缓冲区的大小
    const int iovcnt=(writable<sizeof extrabuf)?2:1;//表示一次性最多只能读64k的数据
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;
    }
    else if(n<=writable)//buffer的可写缓冲区已经足够存储fd上的数据
    {
        writerIndex_+=n;
    }
    else    //栈上的缓冲区也有数据
    {
        writerIndex_=buffer_.size();
        append(extrabuf,n-writable);
    }

    return n;
}