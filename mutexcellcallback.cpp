#include "mutexcellcallback.h"
#include "globaltools.h"
namespace MutexCellCallbackSpace{
const int g_threadPredictIdleTime = 3;//per second

MutexCellCallback::MutexCellCallback(QString objectName):m_thread(NULL) , m_threadFlag(false) , m_cellName(objectName)
{
    m_thread = std::make_shared<std::thread>(&MutexCellCallback::Run , this);
    if(m_thread.get()){
        m_thread->detach();
    }
}

MutexCellCallback::~MutexCellCallback(){
    ////////////////release the m_mutexCallbackQueue
    ReleaseQueue();
}

void MutexCellCallback::PushCallback(CallBack callback){
    std::unique_lock<std::mutex> lock(m_mutex);
    m_mutexCallbackQueue.push(callback);
    m_event.notify_one();
}

std::shared_ptr<MutexCallback> MutexCellCallback::GetFirstElement(){
    return m_mutexCallbackQueue.front();
}

void MutexCellCallback::ReleaseQueue(){
    LOG_INFO(QString("enter the ReleaseQueue Mutex name: %1!").arg(m_cellName));
    while(true){
        if(!m_threadFlag){
            if(!m_mutexCallbackQueue.empty()){
                while(m_mutexCallbackQueue.size()){
                    auto cell = m_mutexCallbackQueue.front();
                    if(cell){
                        cell.reset();
                        cell = NULL;
                    }
                    m_mutexCallbackQueue.pop();
                }
            }else{
                LOG_INFO(QString(" the mutex %1 is alreadly release!").arg(m_cellName));
                break;
            }
        }else{
            LOG_ERROR(QString("the %1 callback queue thread is not stop!please stop the thread!").arg(m_cellName));
            m_threadFlag = false;
            if(!m_mutexCallbackQueue.empty()){
                if(m_thread.get()){
                    m_threadFlag = false;
                    if(m_thread->joinable()){
                        m_thread->join();
                        m_thread.reset();
                    }
                }
            }else{
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_event.notify_one();
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    LOG_INFO(QString("quit the ReleaseQueue Mutex name: %1!").arg(m_cellName));
}


void MutexCellCallback::Run(){
    LOG_INFO(QString("enter MutexCellCallback::Run Mutex name:%1").arg(m_cellName));
    m_threadFlag = true;
    while(m_threadFlag){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_mutexCallbackQueue.empty()){
            LOG_INFO(QString("wait a callback to push into queue the m_cellName:%1!").arg(m_cellName));
            m_event.wait_for(lock , std::chrono::seconds(g_threadPredictIdleTime));
            LOG_INFO(QString("the thread run is revieve notify m_cellName:%1!").arg(m_cellName));
        }
        lock.unlock();//unlock the mutex
        //////////////do the work callback
        if(!m_mutexCallbackQueue.empty() && m_threadFlag){
            CallBack cell = m_mutexCallbackQueue.front();
            if(cell.get()){
               CallbackT callback = cell->callback;
               MutexT  mutex = cell->mutex;
               if(mutex.get()){
                   auto startTime = std::chrono::system_clock::now();
                   while(true){
                       if(mutex->try_lock()){
                           if(callback.get()){
                               auto timeTemp = std::chrono::system_clock::now();
                               std::chrono::duration<double , std::milli> interTime = timeTemp  - startTime;
                               LOG_INFO(QString("callback interTime =%1  ms").arg(interTime.count()));
                               (*callback)();
                               break;
                           }
                       }else{
                          NSSleep(5);
                       }
                       auto nowTime = std::chrono::system_clock::now();
                       std::chrono::duration<double , std::milli> interTime = nowTime  - startTime;
                       if(interTime.count() >= cell->validTime  ){
                           LOG_ERROR(QString("callback is could not get the lock!cell Name = %1  validTime is %2").arg(this->m_cellName).arg(cell->validTime));
                           break;
                       }
                   }
               }
           }
            m_mutexCallbackQueue.pop();
        }
    }
     LOG_INFO(QString("Quit MutexCellCallback::Run Mutex name:%1").arg(m_cellName));
}

}
