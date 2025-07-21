# RtspServer

项目介绍
-
* 基于C++11实现的、跨平台的RTSP（实时流协议）服务器和推流客户端库。

目前情况
-
* 支持 Windows 和 Linux平台。
* 支持 H264, AAC 音视频格式的转发。
* 支持同时传输音视频。
* 支持单播(RTP_OVER_UDP, RTP_OVER_RTSP), 组播。
* 支持心跳检测(单播)。
* 支持RTSP推流(TCP)。

### 核心架构和组件

1.  **网络层 (`src/net`)**: 这是项目的基础。
    *   `EventLoop`: 事件驱动的核心，封装了 `Epoll` (Linux) 或 `Select` (Windows) 的I/O多路复用。它在独立的线程中运行，处理所有网络事件和定时器。
    *   `TaskScheduler`: `EventLoop` 内部的管理单元，负责处理具体的I/O事件、定时器和跨线程触发事件。
    *   `Channel`: 代表一个文件描述符（SOCKET）及其关注的事件（读、写、错误）。
    *   `Acceptor`: 用于服务端，监听端口，接受新的TCP连接。
    *   `TcpConnection`: 封装了一个TCP连接，负责数据的异步读写。
    *   `BufferReader` / `BufferWriter`: 读写缓冲区，简化网络数据的处理。

2.  **RTSP/RTP协议层 (`src/xop`)**: 这是项目的核心业务逻辑。
    *   `RtspServer`: RTSP服务器的主类。它继承自 `TcpServer`，负责管理 `RtspConnection` 和 `MediaSession`。
    *   `RtspPusher`: RTSP推流器的主类。它负责主动连接到远端RTSP服务器并推送媒体流。
    *   `RtspConnection`: 代表一个RTSP客户端连接。它继承自 `TcpConnection`，负责解析和响应RTSP信令（如OPTIONS, DESCRIBE, SETUP, PLAY）。
    *   `MediaSession`: 媒体会话，代表一个可供播放的流资源（例如 `rtsp://.../live`）。它包含一个或多个 `MediaSource`。
    *   `MediaSource`: 媒体源的基类。它的派生类（`H264Source`, `H265Source`, `AACSource`等）负责将原始的音视频帧（`AVFrame`）打包成RTP包。
    *   `RtpConnection`: 负责RTP/RTCP数据的传输。它可以工作在RTP over UDP、RTP over TCP或多播模式下。
    *   `DigestAuthentication`: 实现摘要认证，用于RTSP连接的身份验证。


### 总结

- **核心是事件驱动**: `EventLoop` 是整个项目的心脏，所有操作都是异步的，避免阻塞。
- **职责分离清晰**:
    - `net` 模块负责底层网络通信。
    - `xop` 模块负责高层RTSP/RTP协议逻辑。
    - `RtspConnection` 处理信令，`RtpConnection` 处理数据。
    - `MediaSession` 是媒体资源的抽象，`MediaSource` 负责媒体格式与RTP的转换。
- **统一的推/拉模型**: 无论是服务器向客户端推流（PLAY），还是客户端向服务器推流（RECORD），最终都是通过调用 `PushFrame` API，将数据注入 `MediaSession`，再由 `MediaSource` 打包并通过 `RtpConnection` 发送。数据流向是单向且清晰的。