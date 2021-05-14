#include "thread.h"
#include "examples/peerconnection/client/linux/log.h"

namespace base {

static thread_local Thread *t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";


Semaphore::Semaphore(uint32_t count){
    if(sem_init(&m_semaphore, 0, count)) {
        //throw std::logic_error("sem_init error");
        LOG_DEBUG("sem_init error");
    }
}


Semaphore::~Semaphore() {
    sem_destroy(&m_semaphore);
}

void Semaphore::wait() {
    if(sem_wait(&m_semaphore)) {
        //throw std::logic_error("sem_wait error");
        LOG_DEBUG("sem_wait error");
    }
}

void Semaphore::notify() {
    if(sem_post(&m_semaphore)) {
        //throw std::logic_error("sem_post error");
        LOG_DEBUG("sem_post error");
    }
}

Thread *Thread::GetThis() {
    return t_thread;
}
const std::string &Thread::GetName() {
    return t_thread_name;
}
void Thread::SetName(const std::string &name) {
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string &name)
    :m_cb(cb)
    ,m_name(name){
    if(name.empty()) {
        m_name = "UNKNOW";
    }

    int rt = pthread_create(&m_thread, NULL, &Thread::run, this);
    if(rt != 0) {
        LOG_DEBUG("pthread_create thread fail, rt =%d", rt);
        //throw std::logic_error("pthread_create error");
    }
    //为了不出线程构造函数，确保线程函数跑起来了再出
    m_semaphore.wait();
}

Thread::~Thread() {
    if(m_thread != 0) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if(m_thread != 0) {
        int rt = pthread_join(m_thread, NULL);
        if(rt != 0) {
            //WEBSERVER_LOG_ERROR(g_logger) << "pthread_join thread fail, rt = " << rt;
            LOG_DEBUG("pthread_join thread fail, rt = %d", rt);
           // throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void *Thread::run(void *arg) {
    Thread *thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    //thread->m_id = server_name::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void() > cb;
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();

    cb();
    return 0;
}


}
