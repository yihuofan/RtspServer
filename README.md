# RTSP Server (Linux Only)

本项目是一个基于 C++ 开发的 RTSP 服务器，支持本地媒体文件读取与推流。**仅支持在 Linux 平台构建和运行**。

## 📁 项目结构

```
RtspServer/
├── CMakeLists.txt        # 构建配置
├── main.cpp              # 主函数入口
├── live/                 # 核心模块：媒体源、Sink、Buffer 等
├── bin/                  # 可执行程序输出目录
├── build/                # CMake 构建目录（构建后生成）
└── .vscode/              # VSCode 配置（可选）
```

## 🔧 构建方法（仅适用于 Linux）

确保已安装以下依赖项：

- C++ 编译器（如 g++，支持 C++11 及以上）
- CMake 3.x 或更高版本
- make 工具链

### 1. 安装依赖（如未安装）

```bash
sudo apt update
sudo apt install build-essential cmake
```

### 2. 编译项目

```bash
cd RtspServer/RtspServer
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

编译成功后，会在 `../bin/` 目录下生成可执行文件 `RtspServer`。

### 3. 运行 RTSP 服务器

```bash
./bin/RtspServer
```

> 默认监听端口为 8554，可通过修改 `main.cpp` 自定义行为或配置。

## 📷 项目功能流程图

项目包含简略流程图 `大致流程图.jpg`，可辅助理解模块间调用关系。

## 📌 特性

- 支持 H.264、AAC 文件作为媒体源
- 自定义 MediaSink 管理推流数据
- 支持非阻塞 I/O 模式（采用 epoll）
- 面向对象设计，模块清晰可扩展

