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

static const int INF = 300000;
static bool vis[300000];
static int vis1[300000];             // 减枝
static int Gout[300000][55];         // 记录出边，Gout[i][j] 表示节点i第j条出边所连接的节点
static int Gin[300000][55];          // 记录入边, Gin[i][j] 表示节点i第j条入边所连接的节点
static int pOut[300000];             // 每个节点的出边索引
static int pIn[300000];              // 每个节点的入边索引
static unsigned int maxNodeOut;      // 记录有出度的节点的最大索引
static int path[10];
static int pathIndex = 0;
static int totalCircles = 0;
static int negVal = 0;
static char ans[5][90000000];  // 路径长度只有3~7，这里映射为0~4
static int ansIndex[5];        // 每种长度的路径的指针

void parseInput(std::string &testFile) 
{
    FILE* file = fopen(testFile.c_str(), "r");
    unsigned int u, v, c;
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
    int tmpLen = 0;
    for(int j = 0; j < pathIndex - 1; ++j) {
        tmpLen = strlen(intToChar[path[j]]);
        memcpy(ans[idx] + ansIndex[idx], intToChar[path[j]], tmpLen);
        ansIndex[idx] += tmpLen;
        ans[idx][ansIndex[idx]++] = ',';
    }
    tmpLen = strlen(intToChar[path[pathIndex - 1]]);
    memcpy(ans[idx] + ansIndex[idx], intToChar[path[pathIndex - 1]], tmpLen);
    ansIndex[idx] += tmpLen;
    ans[idx][ansIndex[idx]++] = '\n';
    ++totalCircles;
}

void dfs(int head, int cur, int depth) 
{
    vis[cur] = true;
    path[pathIndex++] = cur;
    for(int i = 0; i < pOut[cur]; ++i) {
        int v = Gout[cur][i];
        // 要求head最小，因此搜索的全过程中不考虑比起点更小的节点
        if (v < head || vis[v]) {
            continue;
        }
        if (vis1[v] != negVal && vis1[v] != head) {
            continue;
        }
        // v->head 有边v为起始点
        if (vis1[v] == negVal) {
            path[pathIndex++] = v;
            addans(depth - 2); // depth-2映射为0 - 4 之间
            pathIndex--;
        }
        if (depth < 6) {
            dfs(head, v, depth + 1);
        }
    }
    vis[cur] = false;
    pathIndex--;
}

// 对每个节点做7层遍历前，先做深度为3的dfs
// 这里由于pOut和pIn维度不一样，所以分为 dfs3o 和 dfs3i 
void dfs3o(int head, int cur, int depth) 
{
    vis[cur] = true;
    path[pathIndex++] = cur;
    for(int i = 0; i < pOut[cur]; ++i) {
        int v = Gout[cur][i];
        if(v < head || vis[v]) continue;
        if(vis1[v] == negVal) {   // v->head有边 v为起始点
            if(depth >= 2) {
                path[pathIndex++] = v;
                addans(depth - 2);  // depth-2映射为0~4之间
                pathIndex--;    
            }
        }
        if (depth < 3) {
            dfs3o(head, v, depth + 1);
        }
        if (depth == 3) {
            dfs(head, v, depth + 1);    
        }
    }
    vis[cur] = false;
    pathIndex--;
}

void dfs3i(int head, int cur, int depth) 
{
    vis[cur] = true;
    for(int i = 0; i < pIn[cur]; ++i) {
        int v = Gin[cur][i];
        if(v < head || vis[v]) continue;
        vis1[v] = head;
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
    for(int i = 0; i <= maxNodeOut; ++i) {
        if (pOut[i]) {
            // 对出度所连接的节点进行排序，这样查找是id都是从小到大，对结果就不用排序了
            std::sort(Gout[i], Gout[i] + pOut[i]);
        }
    }
    for(int i = 0; i <= maxNodeOut; ++i) {
        if(pOut[i] && pIn[i]) {
            negVal = -1 * (i == 0 ? INF : i);
            dfs3i(i, i, 1);
            for(int j = 0; j < pIn[i]; ++j) {
                int v = Gin[i][j];
                vis1[v] = negVal;
            }
            dfs3o(i, i, 1);
        }
    }
}

void save(std::string &outputFile)
{
    std::ostringstream os;
    os << totalCircles << std::endl;
    FILE * fp = fopen(outputFile.c_str(), "w" );
    fwrite(os.str().c_str(), os.str().length(), 1, fp);
    for(int i = 0; i < 5; i++) {
        fwrite(ans[i], ansIndex[i], 1, fp);
    }
    fclose(fp);
}

int main()
{
    std::string testFile = "../std/stdin/primaryin/test_data.txt";
    std::string outputFile = "../std/testout/first3.txt";
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

