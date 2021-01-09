/*
 * @Author: your name
 * @Date: 2020-04-24 10:43:11
 * @LastEditTime: 2020-04-17 10:43:42
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \code\lesson_1.cpp
 */
 
// #define _LINUX

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <thread>
#include <cstring>
#include <time.h>
#include "header.hpp"

#ifdef _LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define _CRT_SECURE_NO_WARNINGS

static const int threadCount = 2;
static const int MAXN = 280000;
/* 以下数组存储图 */
static int Gout[MAXN][50];       // 记录出边id
static int Gin[MAXN][50];        // 记录入边id
static int pOut[MAXN];           // Gout[i]数组索引
static int pIn[MAXN];            // Gin[i]数组索引
static unsigned int maxNodeOut;    // 有出度结点的最大id

/* 以下变量与线程相关，每个线程需要单独享有一份 */ 
static bool vis[threadCount][MAXN];
static int vis1[threadCount][MAXN];           // dsf3i得到该数组, dfs6o遍历时与它做交集进行剑指
static int vis2[threadCount][MAXN];           // for 6 + 1, vis2[i] = head + 1 表示i是head的入边节点   
static int totalCircles[threadCount];
static int tmpPath[threadCount][10];
static short int tmpPathIdx[threadCount];
static int tmpFlag[threadCount];

/* for multi-thread */
static char signalAns[MAXN][5][1000];
static int signalAnsIdx[MAXN][5];
static char ans[150000000];

void parseInput(std::string &testFile) 
{
    FILE* fp = fopen(testFile.c_str(), "r");
    register unsigned int u, v, c, tmpMax = 0;
    memset(pOut, 0, sizeof(pOut));
    memset(pIn, 0, sizeof(pIn));
    while (fscanf(fp, "%u,%u,%u", &u, &v, &c) != EOF) {
        Gout[u][pOut[u]++] = v;
        Gin[v][pIn[v]++] = u;
        if(u > tmpMax) tmpMax = u;
    }
    maxNodeOut = tmpMax;
    fclose(fp);
}

void addSignalAns(int head, int length, int threadNo)
{
	register int j, tmpLen;
	for(j = 0; j < tmpPathIdx[threadNo]; ++j) {
		tmpLen = strlen(intToChar[tmpPath[threadNo][j]]);
		memcpy(signalAns[head][length] + signalAnsIdx[head][length], intToChar[tmpPath[threadNo][j]], tmpLen);
		signalAnsIdx[head][length] += tmpLen;
		signalAns[head][length][signalAnsIdx[head][length]] = ',';
		++signalAnsIdx[head][length];
	}
	signalAns[head][length][signalAnsIdx[head][length] - 1] = '\n';
	++totalCircles[threadNo];
}

// 正向遍历6层
void dfs6o(int head, int cur, int depth, int threadNo) 
{
    register int v, i;
    vis[threadNo][cur] = true;
    tmpPath[threadNo][tmpPathIdx[threadNo]] = cur;
    ++tmpPathIdx[threadNo];
    for(i = 0; i < pOut[cur]; ++i) {
        v = Gout[cur][i];
        if(v < head || vis[threadNo][v]) continue;                 // id < head 才继续访问
        if (depth >= 4 && vis1[threadNo][v] != tmpFlag[threadNo]) continue;  // 第四层以后有交集才访问
        
        if(vis2[threadNo][v] == tmpFlag[threadNo] && depth >= 2) {   // 存在路径，得到一条结果
            tmpPath[threadNo][tmpPathIdx[threadNo]] = v;
            ++tmpPathIdx[threadNo];
            addSignalAns(head, depth - 2, threadNo);  // depth-2映射为0~4之间
            --tmpPathIdx[threadNo];    
        }
        if (depth < 6) {
            dfs6o(head, v, depth + 1, threadNo);
        }
    }
    vis[threadNo][cur] = false;
    --tmpPathIdx[threadNo];
}

// 反向遍历3层：如果将图看做是无向图，一个点数为7的环中，距离起点最远的点距离不超过3
// 所以寻找可以到大深度为4以内的所有路径
void dfs3i(int head, int cur, int depth, int threadNo) 
{
    register int v, i;
    vis[threadNo][cur] = true;
    for(i = 0; i < pIn[cur]; ++i) {
        v = Gin[cur][i];
        if(v < head || vis[threadNo][v]) continue;
        vis1[threadNo][v] = tmpFlag[threadNo];          // 记录距离起点反向距离不超过3的素有节点，为了后面做交集
        if(depth == 1) {                                // for 6 + 1, 记录head的直接入度节点，即最后一层
            vis2[threadNo][v] = tmpFlag[threadNo] ;     // 标记
        }
        if (depth < 3) {
            dfs3i(head, v, depth + 1, threadNo);
        }
    }
    vis[threadNo][cur] = false;
}

void processEven()
{
	register int i;
	for(i = 0; i <= maxNodeOut; i += 2) {
        // 只有出入度不为0的节点才会构成环
        if(pOut[i] && pIn[i]) {
            tmpFlag[0] = i + 1;  // for 6 + 1 标记入度节点，head+1的目的是因为有id为0的节点
            dfs3i(i, i, 1, 0);
            dfs6o(i, i, 1, 0);
        }
    }
}

void processOdd()
{
	register int i;
	for(i = 1; i <= maxNodeOut; i += 2) {
        //只有出入度不为0的节点才会构成环
        if(pOut[i] && pIn[i]) {
            tmpFlag[1] = i + 1;  // for 6 + 1 标记入度节点，head+1 的目的是因为有id为0的节点 
            dfs3i(i, i, 1, 1);
            dfs6o(i, i, 1, 1);
        }
    }
}

void solve() 
{
    memset(totalCircles, 0, sizeof(totalCircles));
    register int i;
    for(i = 0; i <= maxNodeOut; ++i) {
        if (pOut[i]) {
            // 对出度所连接的节点进行排序，这样查找时id都是从小到大，对结果就不用排序了
            std::sort(Gout[i], Gout[i] + pOut[i]);
        }
    }
    std::thread t = std::thread(&processEven);
//    processEven();
    processOdd();
    t.join();
}

void save(std::string &outputFile)
{   
    register int i, j, length = 0;
	for(i = 0; i < 5; ++i) {
		for(j = 0; j < MAXN; ++j) {
			memcpy(ans + length, signalAns[j][i], signalAnsIdx[j][i]);
			length += signalAnsIdx[j][i];
		}
	}
	std::ostringstream os;
    os << totalCircles[0] + totalCircles[1] << std::endl;
    FILE * fp = fopen(outputFile.c_str(), "w" );
    fwrite(os.str().c_str(), os.str().length(), 1, fp);
    fwrite(ans, length, 1, fp);
    fclose(fp);
}

int main01()
{
    std::string testFile = "/data/test_data.txt";
    std::string outputFile = "/projects/student/result.txt";
    parseInput(testFile); //
    solve();
    save(outputFile);
    return 0;
}

