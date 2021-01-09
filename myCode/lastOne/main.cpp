#include "header.hpp"
#include <stdio.h>
#include <algorithm>
#include <thread>
#include <cstring>
#include <time.h>
#include <iostream>
#include <sstream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#define TEST
static const int threadCount = 4;
static const int MAXID = 220000;
// 以下数组存储图
static int Gout[MAXID][100]; // 记录出边ID
static int Gin[MAXID][100]; // 记录入边ID
static int pOut[MAXID]; // Gout[i]数组索引
static int pIn[MAXID]; // Gin[i]数组索引
static int inputs[MAXID];
static unsigned int maxNodeOut; // 有出度的节点的最大id

// 以下变量与线程有关，每个线程需要单独享有一份
static bool vis[threadCount][MAXID];
static int vis1[threadCount][MAXID]; // dfs3i得到该数组，dfs6o遍历时与他做交集进行减枝
static int vis2[threadCount][MAXID]; // for 6 + 1, vis2[i] = head + 1表示i是head的入边节点
static int totalCircles[threadCount];
static int tmpPath[threadCount][10];
static short int tmpPathIdx[threadCount];
static int tmpFlag[threadCount];
/* for multi-thread */
static char ans[threadCount][5][90000000]; // 每个线程将结果写入对应的数组
static int ansIndex[threadCount][5];

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

void addans(int length, int threadNo)
{
	register int j, tmpLen;
	for(j = 0; j < tmpPathIdx[threadNo]; ++j) {
		tmpLen = strlen(intToChar[tmpPath[threadNo][j]]);
		memcpy(ans[threadNo][length] + ansIndex[threadNo][length], intToChar[tmpPath[threadNo][j]], tmpLen);
		ansIndex[threadNo][length] += tmpLen;
		ans[threadNo][length][ansIndex[threadNo][length]] = ',';
		++ansIndex[threadNo][length];
	}
	ans[threadNo][length][ansIndex[threadNo][length] - 1] = '\n';
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
        if (depth >= 4 && vis1[threadNo][v] != tmpFlag[threadNo]) continue;  // 第四层以后有交集才访问(剪枝)
        
        if(vis2[threadNo][v] == tmpFlag[threadNo] && depth >= 2) {   // 存在路径，得到一条结果
            tmpPath[threadNo][tmpPathIdx[threadNo]] = v;
            ++tmpPathIdx[threadNo];
            addans(depth - 2, threadNo);  // depth-2映射为 0 - 4 之间
            --tmpPathIdx[threadNo];    
        }
        if (depth < 6) {
            dfs6o(head, v, depth + 1, threadNo);
        }
    }
    vis[threadNo][cur] = false;
    --tmpPathIdx[threadNo];
}

// 反向遍历3层： 如果将图看做是无向图，一个点数为7的环中，距离起点最远的点距离不超过3
// 所以寻找可以到达的深度为4以内的素有路劲，按理说BFS会快些，但尝试了下，感觉更慢
void dfs3i(int head, int cur, int depth, int threadNo) 
{
    register int v, i;
    vis[threadNo][cur] = true;
    for(i = 0; i < pIn[cur]; ++i) {
        v = Gin[cur][i];
        if(v < head || vis[threadNo][v]) continue;
        vis1[threadNo][v] = tmpFlag[threadNo];          // 记录距离起点反向距离不超过3的所有节点
        if(depth == 1) {                                // for 6 + 1, 记录head的直接入度节点，即最后一层
            vis2[threadNo][v] = tmpFlag[threadNo] ;     // 标记
        }
        if (depth < 3) {
            dfs3i(head, v, depth + 1, threadNo);
        }
    }
    vis[threadNo][cur] = false;
}

void process(int start, int end, int threadNo)
{
	register int i;
	for(i = start; i < end; ++i) {
		tmpFlag[threadNo] = inputs[i] + 1;  // for 6 + 1， 标记入度节点， head + 1的目的是因为有id为0的节点 
		dfs3i(inputs[i], inputs[i], 1, threadNo);
		dfs6o(inputs[i], inputs[i], 1, threadNo);
    }
}

void solve() 
{
    register int i, idx = 0;
    for(i = 0; i <= maxNodeOut; ++i) {
        if (pOut[i]) {
            // 对出度所连接的节点进行排序，这样查找是id都是从小到大，对结果就不用排序了
            std::sort(Gout[i], Gout[i] + pOut[i]);
            if(pIn[i]) {
            	inputs[idx++] = i;
			}
        }
    }
    
    // 划分区间
    int d = idx >> 3;
    int s1 = 0 , e1 = s1 + d;
    int s2 = e1, e2 = s2 + d + d;
    int s3 = e2, e3 = s3 + d + d;
    int s4 = e3, e4 = idx;
    
    std::thread t1 = std::thread(&process, s1, e1, 0);
    std::thread t2 = std::thread(&process, s2, e2, 1);
    std::thread t3 = std::thread(&process, s3, e3, 2);
    process(s4, e4, 3);
    t1.join();
    t2.join();
    t3.join();
}

void save(std::string &outputFile)
{
    std::ostringstream os;
    os << totalCircles[0] + totalCircles[1] + totalCircles[2] + totalCircles[3] << std::endl;
    FILE * fp = fopen(outputFile.c_str(), "w" );
    fwrite(os.str().c_str(), os.str().length(), 1, fp);

    // printf("%d %d %d %d\n", totalCircles[0], totalCircles[1], totalCircles[2], totalCircles[3]);

    register int i, j;
    for(i = 0; i < 5; i++) {
    	for(j = 0; j < threadCount; j++) {
    		fwrite(ans[j][i], ansIndex[j][i], 1, fp);
		} 
    }
    fclose(fp);
}

int main()
{
    std::string testFile = "../std/stdin/3512444/test_data.txt";
    std::string outputFile = "../std/testout/351244.txt";
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
