#ifndef CONSUMERKEYBOARDVALUE_H
#define CONSUMERKEYBOARDVALUE_H
#include <vector>
#include <QMutex>
#include <QWaitCondition>
//#include <windows.h>
#include <memory>
#include <QObject>
#include <functional>
#include <QThread>
#include <queue>
#include <map>
namespace ConsumerKeyboardValueSpace{
    enum KeyBoardType{
        NO_TYPE = 0,
        NORMAL_KEY_ACTION ,
        HANDLE_SHAKE_DIRECT_KEY,
        MOUSE_ACTION
    };


    typedef std::function<void()> keyCallback;
    struct Products{
        KeyBoardType m_type;
        keyCallback m_callback;
        void *  m_pointer;
    };

    template<class T>
    class ConsumerKeyBoardValue : public QThread
    {
    public:
        ConsumerKeyBoardValue(QObject * parent = nullptr):QThread(parent) , m_threadFlag(true), m_pauseComsumer(false){
            m_mutex = std::make_shared<QMutex>();
            m_event = std::make_shared<QWaitCondition>();
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(NO_TYPE , false));
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(NORMAL_KEY_ACTION , false));
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(HANDLE_SHAKE_DIRECT_KEY , false));
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(MOUSE_ACTION , false));
        }
        ~ConsumerKeyBoardValue(){
            Stop();
            if(m_mutex.get()){
                m_mutex.reset();
            }
            if(m_event.get()){
                m_event.reset();
            }
        }
        public:
            virtual void Stop(){
                if(m_threadFlag){
                    m_threadFlag = false;
                    if(m_mutex->tryLock()){
                        m_event->wakeAll();
                        m_mutex->unlock();
                    }
                }
                terminate();
            }

            virtual int  Produce(T value){
                int ret = -1;
                if(m_keyboardVector.empty()){
                    PushTValue(value);
                    m_mutex->lock();
                    m_event->wakeAll();
                    m_mutex->unlock();
                }else{
                    PushTValue(value);
                }
                return ret;
            }

        virtual void FilterKeyType(KeyBoardType keyType , bool valid){
            if(m_filterOutKeyMap.count(keyType)){
                m_filterOutKeyMap[keyType] = valid;
            }
            if(!valid){
                m_keyboardVectorShadow.swap(m_keyboardVector);
                m_pauseComsumer = true;
            }
        }

        virtual bool IsFilterOutKeyType(KeyBoardType keyType){
            if(m_filterOutKeyMap.count(keyType)){
                return m_filterOutKeyMap[keyType];
            }
            return false;
        }


        private:
        virtual void run(){
            while(m_threadFlag){
                if(m_keyboardVector.empty() ){
                     m_mutex->lock();
                     m_event->wait(m_mutex.get());
                     m_mutex->unlock();

                }
                if(m_pauseComsumer){
                    clearBadCallback();
                }else{
                    if(!m_keyboardVector.empty()){
                        auto func = m_keyboardVector.front();
                        func.m_callback();
                        m_keyboardVector.pop();
                    }
                }
            }
        }


        void clearBadCallback(){
            while(m_keyboardVectorShadow.size()){
                auto func = m_keyboardVectorShadow.front();
                if(IsFilterOutKeyType(func.m_type)){
                    func.m_callback();
                }
                m_keyboardVectorShadow.pop();
            }
            m_filterOutKeyMap[NO_TYPE ] = true;
            m_filterOutKeyMap[NORMAL_KEY_ACTION] = true;
            m_filterOutKeyMap[HANDLE_SHAKE_DIRECT_KEY] =  true;
            m_filterOutKeyMap[MOUSE_ACTION] =  true;
            m_pauseComsumer = false;
        }

        void PushTValue(T value){
                m_keyboardVector.push(value);
        }

        private:
            std::shared_ptr<QMutex> m_mutex;
            std::shared_ptr<QWaitCondition> m_event;
            bool m_threadFlag;
            std::queue<T> m_keyboardVector;
            std::queue<T> m_keyboardVectorShadow;
            std::map<KeyBoardType , bool> m_filterOutKeyMap;

            bool m_pauseComsumer;
    };
}

#endif // CONSUMERKEYBOARDVALUE_H
