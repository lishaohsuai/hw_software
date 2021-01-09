
#include "solution.h"
#include <iostream>

int main()
{
    std::string testFile = "../std/stdin/primaryin/test_data.txt";
    std::string outputFile = "../std/testout/first4.txt";
    
#ifdef TEST
    std::string answerFile = "result.txt";
#endif

    auto t0 = clock();
    for(int i = 0; i < 1; i++)
    {   
        auto t = clock();
        parseInput(testFile); //
        std::cout << "读文件Time: " << clock() - t << std::endl;
        
		t = clock();
        solve();
        std::cout << "算法Time: " << clock() - t << std::endl;
        
        t = clock();
        save(outputFile);
        std::cout << "写文件Time: " << clock() - t << std::endl;
    }
    
    std::cout << "总共Time: " << clock() - t0 << std::endl;
    return 0;
}
