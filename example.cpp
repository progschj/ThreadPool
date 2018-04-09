#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <chrono>

//添加add测试
int Add(int a,int b){
	
	std::cout<<"test Add\n";
	//预留线程执行时间。
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return (a+b);
	
}

int main()
{
    
    ThreadPool pool(4);
    std::vector< std::future<int> > results;
	
#if 0
    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1)); 
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }
	
#else	
	for(int i=0;i<8;i++){
		
		results.emplace_back(pool.enqueue(Add,i,i));
						
	}

#endif
    for(auto && result: results)
        std::cout << result.get() << '\n';
    std::cout << std::endl;
    
    return 0;
}
