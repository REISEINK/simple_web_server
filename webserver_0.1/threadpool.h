#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include "locker.h"

template<typename T>
class threadpool {
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request);

private:
    static void *worker(void* args);
    void run();

private:
    int m_thread_number; // 线程池中的线程数

    pthread_t * m_threads; // 描述线程池的数组

    int m_max_requests; // 请求队列允许的最大请求数

    std::list<T *> m_workqueue; // 请求队列

    locker m_queuelocker; // 保护请求队列的互斥锁

    sem m_queuestat; // 是否有任务需要处理

    bool m_stop; // 是否结束线程
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) :
    m_thread_number(thread_number), m_max_requests(max_requests),
    m_stop(false), m_threads(NULL) {

        if (thread_number <= 0 || max_requests <= 0) { // 线程数量 < 0 || 请求队列最大长度 < 0 抛出异常
            throw std::exception();
        }
        // 线程ID初始化
        m_threads = new pthread_t[m_thread_number];

        if (!m_threads) {
            throw std::exception();
        }
        
        for (int i = 0; i < thread_number; ++i) {
            // 循环创建线程，并将工作线程按要求进行允许
            if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
                delete [] m_threads;
                throw std::exception();
            }
            // 将线程进行分离后，不用单独对工作线程进行回收
            if (pthread_detach(m_threads[i])) {
                delete [] m_threads;
                throw std::exception();
            }
        }
    }

template<typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T * request) {
    // 操作工作队列时必须加锁，因为它被所有线程共享
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    // 添加任务
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    // 信号量提醒有任务要处理
    m_queuestat.post();
    printf("append\n");
    return true;
}

template<typename T>
void* threadpool<T>::worker(void * arg) {
    // 将传入参数强转为线程池类，调用成员方法
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run() {
    while(!m_stop) {
        // 信号量等待
        m_queuestat.wait();
        // 被唤醒后先加互斥锁
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue; 
        }
        // 从请求队列中取出第一个任务
        T *request = m_workqueue.front();
        // 将任务从请求队列删除
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if (!request) {
            continue;
        }
        // 调用模板类中的方法进行处理
        request->process();
    }
}

#endif