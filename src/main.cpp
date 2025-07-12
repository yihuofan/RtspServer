#include "EventScheduler.h"
#include "ThreadPool.h"
#include "UsageEnvironment.h"
#include "MediaSessionManager.h"
#include "RtspServer.h"
#include "H264FileMediaSource.h"
#include "H264FileSink.h"
#include "AACFileMediaSource.h"

#include "AACFileSink.h"
#include "Log.h"


int main() {
    /*
    * 
    程序初始化了一份session名为test的资源，访问路径如下

    // rtp over tcp
    ffplay -i -rtsp_transport tcp  rtsp://127.0.0.1:8554/test

    // rtp over udp
    ffplay -i rtsp://127.0.0.1:8554/test
    
    */

    srand(time(NULL));//时间初始化

    EventScheduler *scheduler = EventScheduler::createNew();// 创建事件调度器
    ThreadPool* threadPool = ThreadPool::createNew();// 创建线程池
    MediaSessionManager* sessMgr = MediaSessionManager::createNew();// 创建媒体会话管理器
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool);// 创建使用环境（线程池和事件调度器）
    Ipv4Address rtspAddr("127.0.0.1", 8554);
    RtspServer* rtspServer = RtspServer::createNew(env, sessMgr,rtspAddr);// 创建 RTSP 服务器


    // 媒体会话管理器创建一个媒体会话，名为 test。
    // 该会话包含两个轨道：TrackId0 用于 H264 视频流，TrackId1 用于 AAC 音频流。
    // H264FileMediaSource 从文件读取 H264 视频数据，AACFileMeidaSource 从文件读取 AAC 音频数据。
    // H264FileSink 和 AACFileSink 分别将这些数据打包成 RTP 包并发送。
    // MediaSource 从文件或设备读取原始媒体数据；Sink 负责将这些数据写入 RTP 打包发送。 
    MediaSession *session = MediaSession::createNew("test");// 创建一个新的媒体会话，名为 "test"
    MediaSource* source = H264FileMediaSource::createNew(env, "../data/daliu.h264");// 创建 H264 文件媒体源
    Sink* sink = H264FileSink::createNew(env, source);// 创建 H264 文件 Sink
    session->addSink(MediaSession::TrackId0, sink);

    source = AACFileMediaSource::createNew(env, "../data/daliu.aac");// 创建 AAC 文件媒体源
    sink = AACFileSink::createNew(env, source);
    session->addSink(MediaSession::TrackId1, sink);

    session->startMulticast(); //多播
    sessMgr->addSession(session);
 
    rtspServer->start();

    env->scheduler()->loop();
    return 0;

}