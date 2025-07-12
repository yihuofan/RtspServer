#include "TcpConnection.h"
#include "SocketsOps.h"
#include "Log.h"


TcpConnection::TcpConnection(UsageEnvironment* env, int clientFd) :
        mEnv(env),
        mClientFd(clientFd)
{
    mClientIOEvent = IOEvent::createNew(clientFd, this);
    mClientIOEvent->setReadCallback(readCallback);
    mClientIOEvent->enableReadHandling(); // 开启读事件


    mClientIOEvent->setWriteCallback(writeCallback);// 设置读写事件的回调函数未实现
    mClientIOEvent->setErrorCallback(errorCallback);// 设置错误事件的回调函数未实现
    mClientIOEvent->enableWriteHandling();//开启写事件当前未实现
    mClientIOEvent->enableErrorHandling();//开启错误事件当前未实现

    mEnv->scheduler()->addIOEvent(mClientIOEvent);// 添加到事件调度器
}

TcpConnection::~TcpConnection()
{
    mEnv->scheduler()->removeIOEvent(mClientIOEvent);
    delete mClientIOEvent;
    //    Delete::release(mClientIOEvent);

    sockets::close(mClientFd);
}

void TcpConnection::setDisConnectCallback(DisConnectCallback cb, void* arg)
{
    mDisConnectCallback = cb;
    mArg = arg;
}

void TcpConnection::enableReadHandling()
{
    if (mClientIOEvent->isReadHandling())
        return;

    mClientIOEvent->enableReadHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::enableWriteHandling()
{
    if (mClientIOEvent->isWriteHandling())
        return;

    mClientIOEvent->enableWriteHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::enableErrorHandling()
{
    if (mClientIOEvent->isErrorHandling())
        return;

    mClientIOEvent->enableErrorHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableReadeHandling()
{
    if (!mClientIOEvent->isReadHandling())
        return;

    mClientIOEvent->disableReadeHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableWriteHandling()
{
    if (!mClientIOEvent->isWriteHandling())
        return;

    mClientIOEvent->disableWriteHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::disableErrorHandling()
{
    if (!mClientIOEvent->isErrorHandling())
        return;

    mClientIOEvent->disableErrorHandling();
    mEnv->scheduler()->updateIOEvent(mClientIOEvent);
}

void TcpConnection::handleRead() {

    LOGI("处理客户端发来的读事件, mClientFd=%d", mClientFd);
    int ret = mInputBuffer.read(mClientFd);

    if (ret <= 0)
    {
        LOGE("read error,fd=%d,ret=%d", mClientFd,ret);
        handleDisConnect();
        return;
    }

    handleReadBytes();// 调用RtspConnecton对象的实现函数 
}

void TcpConnection::handleReadBytes() {
    LOGI("");

    mInputBuffer.retrieveAll();// 清空输入缓冲区
}
void TcpConnection::handleDisConnect()
{
    if (mDisConnectCallback) {
        mDisConnectCallback(mArg, mClientFd);
    }
}
void TcpConnection::handleWrite()
{
    LOGI("");
    mOutBuffer.retrieveAll();

}

void TcpConnection::handleError()
{
    LOGI("");
}

void TcpConnection::readCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleRead();
}

void TcpConnection::writeCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleWrite();
}

void TcpConnection::errorCallback(void* arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleError();
}

