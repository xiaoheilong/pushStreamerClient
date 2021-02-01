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

    enum ProductStatus{
        Normal_ROLE = 0 , // can  eat
        DEVIL_ROLE,       // can't eat
        ANGEL_ROLE        // could eat more
    };

    typedef std::function<void()> keyCallback;
    struct Products{
        KeyBoardType m_type;
        keyCallback m_callback;
        void *  m_pointer;
        ProductStatus  m_role;
    };

    template<class T>
    class ConsumerKeyBoardValue : public QThread
    {
    public:
        ConsumerKeyBoardValue(QObject * parent = nullptr):QThread(parent) , m_threadFlag(true){
            m_mutex = std::make_shared<QMutex>();
            m_event = std::make_shared<QWaitCondition>();
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(NO_TYPE , true));
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(NORMAL_KEY_ACTION , true));
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(HANDLE_SHAKE_DIRECT_KEY , true));
            m_filterOutKeyMap.insert(std::make_pair<KeyBoardType , bool>(MOUSE_ACTION , true));
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
                if(isRunning()){
                    wait();
                }
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
        }

        virtual bool IsFilterOutKeyType(KeyBoardType keyType){
            if(m_filterOutKeyMap.count(keyType)){
                return m_filterOutKeyMap[keyType];
            }
            return false;
        }


        private:
        const T & GetFirstElement(){
            return m_keyboardVector.front();
        }

        void    EraseFirstElemt(){
            m_keyboardVector.pop();
        }

        virtual void run(){
            while(m_threadFlag){
                    if(m_keyboardVector.empty() ){
                         m_mutex->lock();
                         m_event->wait(m_mutex.get());
                         m_mutex->unlock();

                    }
                    if(!m_keyboardVector.empty()){
                        auto func = GetFirstElement();
                        func.m_callback();
//                        if(m_filterOutKeyMap[func.m_type]){//this type should been consumer
//                            if(ProductStatus::DEVIL_ROLE == func.m_role){
//                                FilterKeyType(func.m_type , false);
//                            }else{
//                                if(func.m_callback){
//                                    func.m_callback();
//                                }
//                            }
//                        }else{//unvalid type
//                            if(ProductStatus::ANGEL_ROLE == func.m_role){
//                               FilterKeyType(func.m_type , true);
//                               if(func.m_callback){
//                                    func.m_callback();
//                               }
//                            }
//                        }
                        EraseFirstElemt();
                    }
                }
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
    };
}

#endif // CONSUMERKEYBOARDVALUE_H
