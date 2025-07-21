#include "xop/RtspServer.h"
#include "net/Timer.h"
#include <thread>
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <H264File.h>
#include <AACFile.h>

void SendFrameThread(xop::RtspServer *rtsp_server, xop::MediaSessionId session_id, int &clients, H264File &h264_file, AACFile &aac_file);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s <h264_file> <aac_file>\n", argv[0]);
        return -1;
    }

    H264File h264_file;
    if (!h264_file.Open(argv[1]))
    {
        printf("Error: failed to open h264 file.\n");
        return -1;
    }

    AACFile aac_file;
    if (!aac_file.Open(argv[2]))
    {
        printf("Error: failed to open aac file.\n");
        return -1;
    }

    int clients = 0;
    std::string ip = "0.0.0.0";
    std::string rtsp_url = "rtsp://127.0.0.1:554/live";

    std::shared_ptr<xop::EventLoop> event_loop(new xop::EventLoop());
    std::shared_ptr<xop::RtspServer> server = xop::RtspServer::Create(event_loop.get());
    if (!server->Start(ip, 554))
    {
        return -1;
    }

    xop::MediaSession *session = xop::MediaSession::CreateNew("live");
    session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
    session->AddSource(xop::channel_1, xop::AACSource::CreateNew(44100, 2, true));

    session->AddNotifyConnectedCallback([&clients](xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port)
                                        {
		printf("RTSP client connect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
		clients++; });

    session->AddNotifyDisconnectedCallback([&clients](xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port)
                                           {
		printf("RTSP client disconnect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
		clients--; });

    std::cout << "URL: " << rtsp_url << std::endl;

    xop::MediaSessionId session_id = server->AddSession(session);

    std::thread thread(SendFrameThread, server.get(), session_id, std::ref(clients), std::ref(h264_file), std::ref(aac_file));
    thread.detach();

    while (1)
    {
        xop::Timer::Sleep(100);
    }

    return 0;
}




void SendFrameThread(xop::RtspServer *rtsp_server, xop::MediaSessionId session_id, int &clients, H264File &h264_file, AACFile &aac_file)
{
    const int framerate = 25;
    const int sleep_interval = 1000 / framerate;

    // 分配足够大的缓冲区来存放从文件中读取的单帧数据
    std::vector<char> frame_buf(2000000); 
    std::vector<uint8_t> aac_frame_buf;

    while (1)
    {
        if (clients > 0)
        {
            // 发送视频帧
            {
                bool end_of_frame = false;
                int frame_size = h264_file.ReadFrame(frame_buf.data(), frame_buf.size(), &end_of_frame);
                if (frame_size > 0)
                {
                    xop::AVFrame videoFrame(frame_size);
                    videoFrame.type = xop::VIDEO_FRAME_I; // H264File不区分I/P帧，为简单起见都标为I帧
                    videoFrame.timestamp = xop::H264Source::GetTimestamp();
                    memcpy(videoFrame.buffer.get(), frame_buf.data(), frame_size);
                    rtsp_server->PushFrame(session_id, xop::channel_0, videoFrame);
                }
            }

            // 发送音频帧
            {
                if (aac_file.ReadFrame(aac_frame_buf))
                {
                    xop::AVFrame audioFrame(aac_frame_buf.size());
                    audioFrame.type = xop::AUDIO_FRAME;
                    audioFrame.timestamp = xop::AACSource::GetTimestamp(44100);
                    memcpy(audioFrame.buffer.get(), aac_frame_buf.data(), audioFrame.size);
                    rtsp_server->PushFrame(session_id, xop::channel_1, audioFrame);
                }
            }
        }

        xop::Timer::Sleep(sleep_interval);
    }
}
