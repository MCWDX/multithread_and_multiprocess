#include "thread_pool2.h"

int main() {
    {
        ThreadPool2 thread_pool2(4);
        thread_pool2.launch();

        auto func1 = [] (int x, int y) {
            return x * y;
        };

        try {
            auto res1 = thread_pool2.addTask(func1, 10, 4);
            std::cout << res1.get() << std::endl;
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        auto func2 = []() {
            std::cout << "hello world" << std::endl;
        };

        thread_pool2.addTask(std::move(func2));

    }
    
    system("pause");
    return 0;
}