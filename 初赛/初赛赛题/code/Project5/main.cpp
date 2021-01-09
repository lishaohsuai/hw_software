
#include "solution.h"
#include <iostream>

int main()
{
    std::string testFile = "test_data.txt";
    std::string outputFile = "result.txt";
    
#ifdef TEST
    std::string answerFile = "result.txt";
#endif

    auto t0 = clock();
    for(int i = 0; i < 1; i++)
    {   
        auto t = clock();
        parseInput(testFile); // ��ȡ���ݲ�����ͼ
        std::cout << "���ļ��͹���ͼTime: " << clock() - t << std::endl;
        
		t = clock();
        solve();
        std::cout << "�㷨Time: " << clock() - t << std::endl;
        
        t = clock();
        save(outputFile);
        std::cout << "д�ļ�Time: " << clock() - t << std::endl;
    }
    
    std::cout << "��Time: " << clock() - t0 << std::endl;
    return 0;
}
