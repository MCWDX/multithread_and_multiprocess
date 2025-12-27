#include "thread_pool.h"

#include <iostream>
#include <chrono>
#include <thread>

using system_clock = std::chrono::system_clock;
using time_point = std::chrono::time_point<system_clock>;
using std::cout;
using std::endl;

int main() {
    ThreadPool tp(3);
    tp.start();
    if ( tp.addTask([] () { cout << "Hello world" << endl; }) ) {
        cout << "task added" << endl;
    } else {
        cout << "failed to add task" << endl;
    }


    if ( tp.addTask([] () {
        time_point begin = system_clock::now();
        for (int i = 0; i < 100000; i++) {}
        time_point end = system_clock::now();
        cout << (end - begin).count() << endl;
    }) ) {
        cout << "task added" << endl;
    } else {
        cout << "failed to add task" << endl;
    }

    if ([] (int a, int b) { cout << a + b << endl; }, 3, 5) {
        cout << "task added" << endl;
    } else {
        cout << "failed to add task" << endl;
    }

    tp.addTask([] (int a, int b) {
        cout << a + b << endl;
    }, 3, 5);

    //不sleep一下最后加的那个task经常会没开始就被掐掉
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    tp.stop();
    system("pause");
    return 0;
}

/*
输出
added thread 0
added thread 1
added thread 2
task added
Hello world
task added
task added
68000
8
At the time of stopping the thread pool,there were 0 tasks in the queue
thread joined
thread joined
thread joined
 */