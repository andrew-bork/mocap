#ifndef MUTEX_H_GUARD
#define MUTEX_H_GUARD

#define MUTEX_NATIVE_HANDLE pthread_mutex_t
#include <pthread.h>

namespace std {
    class mutex {
        MUTEX_NATIVE_HANDLE handle = -1;
        bool locked = false;
        public:
            mutex();
            ~mutex();
            void lock();
            void try_lock();
            void unlock();
            MUTEX_NATIVE_HANDLE native_handle();
    };

    template<class Mutex> class lock_guard{
        Mutex m;
        public:
            lock_guard(Mutex _m);
            ~ lock_guard();
    };
};

#endif