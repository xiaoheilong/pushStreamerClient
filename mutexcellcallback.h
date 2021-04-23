#ifndef MUTEXCELLCALLBACK_H
#define MUTEXCELLCALLBACK_H
#include <mutex>
#include <chrono>
#include <iostream>
#include <queue>
#include <condition_variable>
#include <functional>
#include <QObject>
namespace MutexCellCallbackSpace{
////CallbackT and MutexT type is std::shared_ptr
//template<class CallbackT , class MutexT>
typedef std::function<void()> Function_Callback;
typedef std::shared_ptr<Function_Callback> CallbackT;
typedef std::shared_ptr<std::mutex> MutexT;
struct MutexCallback{

    MutexCallback(){
        callback = NULL;
        mutex = NULL;
        validTime = 0;
    }

    MutexCallback(CallbackT c , MutexT m , long time){
        callback = c;
        mutex = m;
        validTime = time;
    }

    MutexCallback(const MutexCallback &obj){
        callback = obj.callback;
        mutex = obj.mutex;
        validTime = obj.validTime;
    }

    MutexCallback & operator=(const MutexCallback &obj){
        callback = obj.callback;
        mutex = obj.mutex;
        validTime = obj.validTime;
    }

    ~MutexCallback(){
        callback.reset();
        mutex.reset();
    }

    CallbackT callback;
    MutexT  mutex;
    long   validTime;
};


class MutexCellCallback
{
public:
    MutexCellCallback(QString objectName);
    ~MutexCellCallback();
     typedef std::shared_ptr<MutexCallback>  CallBack;
public:
    void PushCallback(CallBack callback);
    std::shared_ptr<MutexCallback> GetFirstElement();
private:
    MutexCellCallback(const MutexCellCallback & );
    MutexCellCallback & operator=(const MutexCellCallback &);

    void ReleaseQueue();
protected:
    void Run();
private:
    std::shared_ptr<std::thread> m_thread;
    std::queue<CallBack> m_mutexCallbackQueue;
    bool m_threadFlag;
    QString m_cellName;
    std::condition_variable m_event;
    std::mutex m_mutex;
};


}

#endif // MUTEXCELLCALLBACK_H
