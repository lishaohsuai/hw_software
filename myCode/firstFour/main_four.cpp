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
#include "header.hpp"
#ifdef _LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#define TEST

static const int INF = 300000;
static bool vis[300000];
static int vis1[300000];             // dsf3i得到该数组, dfs6o遍历时与他做交集进行减枝
static int vis2[300000];             // for 6 + 1, vis2[i] = head + 1 表示 i 是 head的入边节点
static int Gout[300000][50];         // 记录出边id
static int Gin[300000][50];          // 记录入边id
static int pOut[300000];             // Gout[i] 数组索引
static int pIn[300000];              // Gin[i]数组索引
static unsigned int maxNodeOut;      // 有出度的节点的最大idid  
static char ans[5][100000000];       // 路径长度只有3~7 映射为 0~4 
static int ansIndex[5];              // ans[i]
static int totalCircles = 0;
static int tmpPath[10];
static int tmpPathIdx = 0;
static int tmpLen = 0;
static int tmpFlag = 0;

void parseInput(std::string &testFile) 
{
    FILE* file = fopen(testFile.c_str(), "r");
    register unsigned int u, v, c;
    maxNodeOut = 0;
    memset(pOut, 0, sizeof(pOut));
    memset(pIn, 0, sizeof(pIn));
    while (fscanf(file, "%u,%u,%u", &u, &v, &c) != EOF) {
        Gout[u][pOut[u]++] = v;
        Gin[v][pIn[v]++] = u;
        if(u > maxNodeOut) maxNodeOut = u;
    }
    fclose(file);
}

void addans(int idx)
{
    register int j;
    for(j = 0; j < tmpPathIdx; ++j) {
        tmpLen = strlen(intToChar[tmpPath[j]]);
        memcpy(ans[idx] + ansIndex[idx], intToChar[tmpPath[j]], tmpLen);
        ansIndex[idx] += tmpLen;
        ans[idx][ansIndex[idx]++] = ',';
    }
    ans[idx][ansIndex[idx] - 1] = '\n';
    ++totalCircles;
}

// 正向遍历6层
void dfs6o(int head, int cur, int depth) 
{
    register int v, i;
    vis[cur] = true;
    tmpPath[tmpPathIdx++] = cur;
    for(i = 0; i < pOut[cur]; ++i) {
        v = Gout[cur][i];
        if(v < head || vis[v]) continue;                 // id < head 继续访问
        if (depth >= 4 && vis1[v] != tmpFlag) continue;  // 第四层以后有交集才访问
        
        if(vis2[v] == tmpFlag && depth >= 2) {   // 存在路径得到一条结果
            tmpPath[tmpPathIdx++] = v;
            addans(depth - 2);  // depth-2ӳ��Ϊ0~4֮��
            tmpPathIdx--;    
        }
        if (depth < 6) {
            dfs6o(head, v, depth + 1);
        }
    }
    vis[cur] = false;
    tmpPathIdx--;
}

// 反向遍历3层：如果将图看做是无向图，一个点数为7的换中，距离起点最远的点距离不超做3
void dfs3i(int head, int cur, int depth) 
{
    register int v, i;
    vis[cur] = true;
    for(i = 0; i < pIn[cur]; ++i) {
        v = Gin[cur][i];
        if(v < head || vis[v]) continue;
        vis1[v] = tmpFlag;          // 记录距离起点反向距离不超过3的所有点，为了后面做交集
        if(depth == 1) {            // for 6 + 1, 记录head的直接入度节点，即最后一层
            vis2[v] = tmpFlag ;     // 标记
        }
        if (depth < 3) {
            dfs3i(head, v, depth + 1);
        }
    }
    vis[cur] = false;
}

void solve() 
{
    memset(ansIndex, 0, sizeof(ansIndex));
    totalCircles = 0;
    register int i;
    for(i = 0; i <= maxNodeOut; ++i) {
        if (pOut[i]) {
            // 对出度所链接的节点进行排序，这样查找时id都是从小到大，对结果就不用排序了
            std::sort(Gout[i], Gout[i] + pOut[i]);
        }
    }
    for(i = 0; i <= maxNodeOut; ++i) {
        // 只有出入度不为0的节点才会构成环
        if(pOut[i] && pIn[i]) {
            tmpFlag = i + 1;  // for 6 + 1, 标记入度节点，head + 1 的目的是因为有id为0的节点
            dfs3i(i, i, 1);
            dfs6o(i, i, 1);
        }
    }
}

void save(std::string &outputFile)
{
    std::ostringstream os;
    os << totalCircles << std::endl;
    FILE * fp = fopen(outputFile.c_str(), "w" );
    fwrite(os.str().c_str(), os.str().length(), 1, fp);
    register int i;
    for(i = 0; i < 5; i++) {
        fwrite(ans[i], ansIndex[i], 1, fp);
    }
    fclose(fp);
}

int main()
{
    std::string testFile = "../std/stdin/highin/test_data.txt";
    std::string outputFile = "../std/testout/firstfff.txt";
#ifdef TEST
    auto t0 = clock();
#endif
    parseInput(testFile); // 读取数据并构建图
#ifdef TEST
    auto t1 = clock();
    std::cout << "[DEBUG] 读取文件并构建图所消耗时间" <<  t1 - t0 << std::endl;
#endif
    solve();
#ifdef TEST
    auto t2 = clock();
    std::cout << "[DEBUG] 处理时间" << t2 - t1 << std::endl;
#endif
    save(outputFile);
#ifdef TEST
    auto t3 = clock();
    std::cout << "[DEBUG] 存储文件时间" << t3 - t2 << std::endl;
#endif
    return 0;
}