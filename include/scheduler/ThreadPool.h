#ifndef RTSPSERVER_THREADPOOL_H
#define RTSPSERVER_THREADPOOL_H
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "Thread.h"

class ThreadPool
{
public:
    class Task
    {
    public:
        typedef void (*TaskCallback)(void *);
        Task() : mTaskCallback(nullptr), mArg(nullptr) {}

        void setTaskCallback(TaskCallback cb, void *arg)
        {
            mTaskCallback = cb;
            mArg = arg;
        }

        void handle()
        {
            if (mTaskCallback)
                mTaskCallback(mArg);
        }

        Task &operator=(const Task &task)
        {
            if (this != &task)
            {
                this->mTaskCallback = task.mTaskCallback;
                this->mArg = task.mArg;
            }
            return *this;
        }

    private:
        TaskCallback mTaskCallback;
        void *mArg;
    };

    static ThreadPool *createNew();

    explicit ThreadPool(unsigned int threadCount);
    ~ThreadPool();

    void addTask(Task &task);

private:
    void loop();

    class MThread : public Thread
    {
    protected:
        void run(void *arg) override;
    };

    void createThreads();
    void cancelThreads();

private:
    std::queue<Task> mTaskQueue;
    std::mutex mMtx;
    std::condition_variable mCon;// 条件变量，用于线程间的同步
    std::vector<MThread> mThreads;
    bool mQuit;
};

#endif 