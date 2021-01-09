/*
 * @Author: your name
 * @Date: 2020-04-17 10:43:11
 * @LastEditTime: 2020-04-17 10:43:42
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \code\lesson_1.cpp
 */

#define _LINUX

#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <thread>
#include <cstring>
#include <time.h>

#ifdef _LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#define TEST
#define _CRT_SECURE_NO_WARNINGS  

static bool vis[600000]; 
static int Gout[600000][100];    // 记录出边，Gout[i][j] 表示节点i第j条出边所连接的节点
static int Gin[600000][500];     // 记录入边, Gin[i][j] 表示节点i第j条入边所连接的节点 
static int pOut[600000];         // 每个节点的出边索引
static int pIn[600000];          // 每个节点的入边索引

struct Path {
    //ID最小的第一个输出；
    //总体按照循环转账路径长度升序排序；
    //同一级别的路径长度下循环转账账号ID序列，按照字典序（ID转为无符号整数后）升序排序
    int length;
    std::vector<unsigned int> path;

    Path(int length, const std::vector<unsigned int> &path) : length(length), path(path) {}

    bool operator<(const Path&rhs)const 
    {   ////在排序的时候有用
        if (length != rhs.length) return length < rhs.length; // length 从小到大
        for (int i = 0; i < length; i++) {
            if (path[i] != rhs.path[i])   // 字典序
                return path[i] < rhs.path[i];
        }
    }
};

class Solution {
public:
    std::unordered_map<unsigned int, int> idHash;    // sorted id to 0...n 
    std::vector<unsigned int> ids;                   // 0...n to sorted id
    std::vector<unsigned int> inputs;                // u-v pairs 
    std::vector<int> path;
    std::vector<Path> ans;
    int nodeCnt;
    
#ifdef _LINUX
    void parseInput(std::string &testFile)
    {
        int fd = open(testFile.c_str(), O_RDONLY);
        int len = lseek(fd, 0, SEEK_END);
        char *mbuf = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if(fd != -1) {
            close(fd);  
        }
        unsigned int offset = 0, cnt = 0;;
        unsigned int u, v, c;
        for(int i = 0; i < len; i++) {
            if(mbuf[i] == '\r') {
                mbuf[i] = '\0';
                sscanf(mbuf + offset, "%u,%u,%u", &u, &v, &c);
                offset = i + 2;   // 跳过 /r/n 
                inputs.push_back(u);
                inputs.push_back(v);
                ++cnt;
            }
        }
        munmap(mbuf, len); 
    }
#else
    void parseInput(std::string &testFile) 
    {
        FILE* file = fopen(testFile.c_str(), "r");
        unsigned int u, v, c;
        int cnt = 0;
        while (fscanf(file, "%u,%u,%u", &u, &v, &c) != EOF) {
            inputs.push_back(u);
            inputs.push_back(v);
            ++cnt;
        }
        if(file != NULL) {
            fclose(file);
        }
        
#ifdef TEST
        printf("%d Records in Total\n", cnt);
#endif

    }
#endif

    void constructGraph() 
    {
        std::vector<unsigned int> tmp = inputs;
        sort(tmp.begin(), tmp.end());
        tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
        ids = tmp;
        nodeCnt = 0;
        
        for(int i = 0; i < tmp.size(); i++) {
            idHash[tmp[i]] = nodeCnt++;
        }
        
#ifdef TEST
        printf("%d Nodes in Total\n", nodeCnt);
#endif

        int sz = inputs.size();
        memset(pOut, 0, sizeof(pOut));
		memset(pIn, 0, sizeof(pIn));
        for (int i = 0; i < sz; i += 2) {
            int u = idHash[inputs[i]], v = idHash[inputs[i + 1]];
            Gout[u][pOut[u]++] = v;
            Gin[v][pIn[v]++] = u;
        }
    }

    void dfs(int head, int cur, int depth) 
    {
        vis[cur] = true;      
        path.push_back(cur);   
        for(int i = 0; i < pOut[cur]; i++) {
            int v = Gout[cur][i];
            if (v == head && depth >= 3 && depth <= 7) {
                std::vector<unsigned int> tmp;
                for(int j = 0; j < path.size(); j++)
                    tmp.push_back(ids[path[j]]);
                ans.emplace_back(Path(depth, tmp)); 
            }
            if (depth < 7 && !vis[v] && v > head) {
                dfs(head, v, depth + 1);
            }
        }
        vis[cur] = false;
        path.pop_back();
    }

    // search from 0...n
    // 由于要求id最小的在前，因此搜索的全过程中不考虑比起点id更小的节点
    void solve() 
    {
        for (int i = 0; i < nodeCnt; i++) {
            if (pOut[i] && pIn[i]) { // 这个节点有出度和入度的话开启dfs
                dfs(i, i, 1);
            }
        }
        sort(ans.begin(), ans.end());
    }

    void save(std::string &outputFile)
    {
        std::ostringstream os;
        os << ans.size() << std::endl;
        
        for(int k = 0; k < ans.size(); k++) {
            std::vector<unsigned int> path = ans[k].path;
            int sz = path.size();
            os << path[0];
            for (int i = 1; i < sz; i++) {
                os << "," << path[i];
            }
            os << std::endl;
        }
        FILE * fp = fopen(outputFile.c_str(), "w" );
        fwrite(os.str().c_str(), os.str().length(), 1, fp);
        fclose(fp);
    }
};

int main()
{
    std::string testFile = "../std/stdin/primaryin/test_data.txt";
    std::string outputFile = "../std/testout/firsttwo1.txt";
#ifdef TEST
    std::string answerFile = "C:/Users/lee/Desktop/code/华为软件精英挑战赛/初赛/初赛赛题/result.txt";
#endif
    Solution solution;// 构建一个解决方案类
#ifdef TEST
    auto t0 = clock();
#endif
    solution.parseInput(testFile); // 读取数据将 id1,id2,money  id1和id2 存储在 std::vector<unsigned int> inputs; 
#ifdef TEST
    auto t1 = clock();
    std::cout << "[DEBUG] 读取文件所消耗时间" <<  t1 - t0 << std::endl;
#endif

    solution.constructGraph();// 构建图
#ifdef TEST
    auto t2 = clock();
    std::cout << "[DEBUG] 构建图所用时间" << t2 - t1 << std::endl;
#endif
    //solution.topoSort();
    solution.solve();//解决它
#ifdef TEST
    auto t3 = clock();
    std::cout << "[DEBUG] DFS 处理时间" << t3 - t2 << std::endl;
#endif
    solution.save(outputFile);
#ifdef TEST
    auto t4 = clock();
    std::cout << "[DEBUG] 存储文件时间" << t4 - t3 << std::endl;
#endif
    return 0;
}

