#include <mutex.h>

std::mutex::mutex() {
    pthread_mutex_init(&handle, NULL);
}

std::mutex::~mutex() {
    pthread_mutex_destroy(&handle);
}

void std::mutex::unlock() {
    pthread_mutex_unlock(&handle);
    locked = false;
}

MUTEX_NATIVE_HANDLE std::mutex::native_handle() {
    return handle;
}

void std::mutex::lock() {
    locked = true;
    pthread_mutex_lock(&handle);
}


void std::mutex::try_lock() {
    if(!locked) {
        pthread_mutex_lock(&handle);
    }
}

template <class M> std::lock_guard<M>::lock_guard(M _m){
    m = _m;
    m.lock();
}


template <class M> std::lock_guard<M>::~lock_guard(){
    m.unlock();
}
