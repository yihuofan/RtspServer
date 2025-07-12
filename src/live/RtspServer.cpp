#include "RtspServer.h"
#include "RtspConnection.h"
#include "Log.h"

RtspServer* RtspServer::createNew(UsageEnvironment* env, MediaSessionManager* sessMgr, Ipv4Address& addr) {

    return new RtspServer(env, sessMgr,addr);
}
RtspServer::RtspServer(UsageEnvironment* env, MediaSessionManager* sessMgr, Ipv4Address& addr) :
        mSessMgr(sessMgr),
        mEnv(env),
        mAddr(addr),
        mListen(false)
{

    mFd = sockets::createTcpSock();//创建一个非阻塞的tcp描述符
    sockets::setReuseAddr(mFd, 1);//设置地址复用
    if (!sockets::bind(mFd, addr.getIp(), mAddr.getPort())) {
        return;
    }//绑定地址和端口     

    LOGI("rtsp://%s:%d fd=%d",addr.getIp().data(),addr.getPort(), mFd);

    mAcceptIOEvent = IOEvent::createNew(mFd, this);//创建一个IO事件
    mAcceptIOEvent->setReadCallback(readCallback);
    mAcceptIOEvent->enableReadHandling();//设置有读事件

    mCloseTriggerEvent = TriggerEvent::createNew(this);
    mCloseTriggerEvent->setTriggerCallback(cbCloseConnect);//处理已经断开的客户端链接，清理资源

}

RtspServer::~RtspServer()
{
    if (mListen)
        mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);

    delete mAcceptIOEvent;
    delete mCloseTriggerEvent;

    sockets::close(mFd);
}



void RtspServer::start(){
    LOGI("");
    mListen = true;
    sockets::listen(mFd,60);
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);
}

void RtspServer::readCallback(void* arg) {
    RtspServer* rtspServer = (RtspServer*)arg;
    rtspServer->handleRead();

}

void RtspServer::handleRead() {
    int clientFd = sockets::accept(mFd);
    if (clientFd < 0)
    {
        LOGE("handleRead error,clientFd=%d",clientFd);
        return;
    }
    RtspConnection* conn = RtspConnection::createNew(this, clientFd);
    conn->setDisConnectCallback(RtspServer::cbDisConnect, this);//设置断开连接的回调函数指针
    mConnMap.insert(std::make_pair(clientFd, conn));//将链接加入到连接映射表中
}

void RtspServer::cbDisConnect(void* arg, int clientFd) {
    RtspServer* server = (RtspServer*)arg;

    server->handleDisConnect(clientFd);
}

void RtspServer::handleDisConnect(int clientFd) {
 
    std::lock_guard <std::mutex> lck(mMtx);
    mDisConnList.push_back(clientFd);//将断开的链接加入到断开连接列表中

    mEnv->scheduler()->addTriggerEvent(mCloseTriggerEvent); // 添加触发事件到事件调度器中，当连接断开时可以通知服务器进行清理
}

void RtspServer::cbCloseConnect(void* arg) {
    RtspServer* server = (RtspServer*)arg;
    server->handleCloseConnect();
}
void RtspServer::handleCloseConnect() {

    std::lock_guard <std::mutex> lck(mMtx);

    for (std::vector<int>::iterator it = mDisConnList.begin(); it != mDisConnList.end(); ++it) {

        int clientFd = *it;
        std::map<int, RtspConnection*>::iterator _it = mConnMap.find(clientFd);
        assert(_it != mConnMap.end());
        delete _it->second;
        mConnMap.erase(clientFd);
    }

    mDisConnList.clear();



}