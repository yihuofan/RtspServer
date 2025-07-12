#include "ThreadPool.h"
#include "Log.h"
#include <thread>

ThreadPool *ThreadPool::createNew()
{
    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0)
        threadCount = 1; // 至少1个线程
    return new ThreadPool(threadCount);
}

ThreadPool::ThreadPool(unsigned int threadCount)
    : mThreads(threadCount), mQuit(false)
{
    createThreads();// 创建线程池中的线程
}

ThreadPool::~ThreadPool()
{
    cancelThreads();
}

void ThreadPool::addTask(ThreadPool::Task &task)
{
    std::unique_lock<std::mutex> lck(mMtx);
    mTaskQueue.push(task);
    mCon.notify_one();
}

void ThreadPool::loop()
{
    while (!mQuit)
    {
        std::unique_lock<std::mutex> lck(mMtx);
        mCon.wait(lck, [this]
                  { return mQuit || !mTaskQueue.empty(); });// 等待任务队列不为空或线程池被取消

        if (mQuit && mTaskQueue.empty())
            break;

        if (mTaskQueue.empty())
            continue;

        Task task = mTaskQueue.front();
        mTaskQueue.pop();
        lck.unlock();

        task.handle();
    }
}

void ThreadPool::createThreads()
{
    std::unique_lock<std::mutex> lck(mMtx);
    for (auto &mThread : mThreads)
        mThread.start(this);
}

void ThreadPool::cancelThreads()
{
    std::unique_lock<std::mutex> lck(mMtx);
    mQuit = true;
    mCon.notify_all();
    for (auto &mThread : mThreads)
        mThread.join();

    mThreads.clear();
}

void ThreadPool::MThread::run(void *arg)
{
    ThreadPool *threadPool = (ThreadPool *)arg;
    threadPool->loop();
}