#include "xop/RtspPusher.h"
#include "net/Timer.h"
#include <thread>
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <H264File.h>
#include <AACFile.h>


void sendFrameThread(xop::RtspPusher *rtsp_pusher, H264File *h264_file, AACFile *aac_file);

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		std::cout << "Usage: " << argv[0] << " <rtsp_url> <h264_file> <aac_file>" << std::endl;
		std::cout << "Example: " << argv[0] << " rtsp://127.0.0.1:554/live test.h264 test.aac" << std::endl;
		return 0;
	}

	std::string rtsp_url = argv[1];
	std::string h264_file_path = argv[2];
	std::string aac_file_path = argv[3];

	H264File h264_file;
	if (!h264_file.Open(h264_file_path.c_str()))
	{
		std::cout << "Error: open " << h264_file_path << " failed." << std::endl;
		return -1;
	}

	AACFile aac_file;
	if (!aac_file.Open(aac_file_path.c_str()))
	{
		std::cout << "Error: open " << aac_file_path << " failed." << std::endl;
		return -1;
	}

	std::shared_ptr<xop::EventLoop> event_loop(new xop::EventLoop());
	std::shared_ptr<xop::RtspPusher> rtsp_pusher = xop::RtspPusher::Create(event_loop.get());

	xop::MediaSession *session = xop::MediaSession::CreateNew();
	session->AddSource(xop::channel_0, xop::H264Source::CreateNew());
	session->AddSource(xop::channel_1, xop::AACSource::CreateNew(44100, 2, true)); // 44100Hz, 2 channels, has ADTS header
	rtsp_pusher->AddSession(session);

	if (rtsp_pusher->OpenUrl(rtsp_url, 3000) < 0)
	{
		std::cout << "Open " << rtsp_url << " failed." << std::endl;
		getchar();
		return 0;
	}

	std::cout << "Push stream to " << rtsp_url << " ..." << std::endl;

	std::thread thread(sendFrameThread, rtsp_pusher.get(), &h264_file, &aac_file);
	thread.detach();

	while (1)
	{
		xop::Timer::Sleep(100);
	}

	getchar();
	return 0;
}

void sendFrameThread(xop::RtspPusher *rtsp_pusher, H264File *h264_file, AACFile *aac_file)
{
	const int video_frame_rate = 25;
	const int audio_samples_per_frame = 1024;
	const int audio_sample_rate = 44100;

	int video_buf_size = 2000000;
	int audio_buf_size = 5000;
	std::unique_ptr<uint8_t> video_buf(new uint8_t[video_buf_size]);
	std::unique_ptr<uint8_t> audio_buf(new uint8_t[audio_buf_size]);

	auto start_time = std::chrono::steady_clock::now();
	int64_t next_video_send_time = 0;
	int64_t next_audio_send_time = 0;

	// 计算音视频帧的发送间隔 (ms)
	const int64_t video_interval_ms = 1000 / video_frame_rate;
	const int64_t audio_interval_ms = (1000 * audio_samples_per_frame) / audio_sample_rate;

	while (rtsp_pusher->IsConnected())
	{
		auto current_time = std::chrono::steady_clock::now();
		int64_t elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();

		// 1. 发送视频
		if (elapsed_time_ms >= next_video_send_time)
		{
			bool end_of_frame = false;
			int frame_size = h264_file->ReadFrame((char *)video_buf.get(), video_buf_size, &end_of_frame);
			if (frame_size > 0)
			{
				xop::AVFrame videoFrame;
				videoFrame.type = (video_buf.get()[0] & 0x1F) == 5 ? xop::VIDEO_FRAME_I : xop::VIDEO_FRAME_P;
				videoFrame.size = frame_size;
				videoFrame.timestamp = xop::H264Source::GetTimestamp();
				videoFrame.buffer.reset(new uint8_t[videoFrame.size]);
				memcpy(videoFrame.buffer.get(), video_buf.get(), videoFrame.size);

				rtsp_pusher->PushFrame(xop::channel_0, videoFrame); // 推流到服务器, 接口线程安全
			}
			next_video_send_time += video_interval_ms;
		}

		// 2. 发送音频
		if (elapsed_time_ms >= next_audio_send_time)
		{
			std::vector<uint8_t> audio_vec;
			int frame_size = aac_file->ReadFrame(audio_vec);
			if (frame_size > 0 && audio_vec.size() >= (size_t)frame_size)
			{
				memcpy(audio_buf.get(), audio_vec.data(), frame_size);

				xop::AVFrame audioFrame;
				audioFrame.type = xop::AUDIO_FRAME;
				audioFrame.size = frame_size;
				audioFrame.timestamp = xop::AACSource::GetTimestamp(audio_sample_rate);
				audioFrame.buffer.reset(new uint8_t[audioFrame.size]);
				memcpy(audioFrame.buffer.get(), audio_buf.get(), audioFrame.size);

				rtsp_pusher->PushFrame(xop::channel_1, audioFrame); // 推流到服务器, 接口线程安全
			}
			next_audio_send_time += audio_interval_ms;
		}

		xop::Timer::Sleep(1);
	}
}
