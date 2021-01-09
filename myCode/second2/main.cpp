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
// #include "header.hpp"
#include <cstddef>
#include <typeinfo>
#include <set>
#include <string>
#ifdef _LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif
// #define TEST

// static const int INF = 300000;
static std::vector<bool> vis;
static std::vector<int> vis1;             // dsf3i得到该数组, dfs6o遍历时与他做交集进行减枝
static std::vector<int> vis2;             // for 6 + 1, vis2[i] = head + 1 表示 i 是 head的入边节点
static std::vector<std::vector<int>> Gout;         // 记录出边id
static std::vector<std::vector<int>> Gin;          // 记录入边id
static std::vector<int> pOut;             // Gout[i] 数组索引
static std::vector<int> pIn;              // Gin[i]数组索引
static unsigned int maxNodeOut;      // 有出度的节点的最大idid
std::vector<unsigned int> inputs;
std::set<unsigned int> mySet;
std::vector<unsigned int> ids; //0...n to sorted id
std::unordered_map<unsigned int, int> idHash; // sorted id to 0 ... n
std::unordered_map<unsigned long long, unsigned int>money;
static std::string ans[5];
static int ansIndex[5];              // ans[i]
static int totalCircles = 0;
static unsigned int tmpPath[10];
static int tmpPathIdx = 0;
static int tmpLen = 0;
static int tmpFlag = 0;
static unsigned int nodeCnt = 0;

inline std::string int2Str(unsigned int num)
{
    if(num == 0)
    {
        return "0";
    }
    std::string str = "", sign = "";
    // if(num < 0) //负数
    // {
    //     num = -1 * num;
    //     sign = "-";
    // }
    int i = 0;
    while (num != 0)
    {
        str += (char)((num % 10) + '0');
        num /= 10;
        i++;
    }
    int semiLen = i / 2;
    int offset = i % 2 - 1;
    char ch;
    for (int j = 0; j < semiLen; ++j)
    {
        ch = str[j];
        str[j] = str[2 * semiLen + offset - j];
        str[2 * semiLen + offset - j] = ch;
    }
    return str;
}


void parseInput(std::string &testFile) 
{
    FILE* file = fopen(testFile.c_str(), "r");
    register unsigned int u, v, c;
    maxNodeOut = 0;
    while (fscanf(file, "%u,%u,%u", &u, &v, &c) != EOF) {
        inputs.push_back(u);
        inputs.push_back(v);
        inputs.push_back(c);
        mySet.insert(u);
        mySet.insert(v);
        // money[(unsigned long long)u << 32 | v] = c;
    }
    fclose(file);
}

void constructGraph(){
    auto tmp = inputs;
    nodeCnt = mySet.size();
    for(auto it=mySet.begin(); it!=mySet.end(); it++){
        ids.push_back(*it);
        // std::cout << *it << std::endl;
    }
    // 压缩矩阵
    nodeCnt = 0;
    for(unsigned int &x : ids){
        idHash[x] = nodeCnt++;
    }
    int sz = inputs.size();
    Gout = std::vector<std::vector<int>>(nodeCnt);
    Gin = std::vector<std::vector<int>>(nodeCnt);
    pOut = std::vector<int>(nodeCnt, 0);// 对于每一个节点的出度
    pIn = std::vector<int>(nodeCnt, 0);// 对于每一个节点的入度
    for(int i=0; i<sz; i+=3){
        int u = idHash[inputs[i]], v = idHash[inputs[i+1]];
        pOut[u]++;
        Gout[u].push_back(v);
        pIn[v]++;
        Gin[v].push_back(u);
        money[(unsigned long long)u << 32 | v] = inputs[i+2];
    }
    vis = std::vector<bool>(nodeCnt, false);
    vis1 = std::vector<int>(nodeCnt, 0);
    vis2 = std::vector<int>(nodeCnt, 0);
}

void addans(int idx)
{
    register int j;

    for(j = 0; j < tmpPathIdx; ++j) {
        ans[idx] += int2Str(ids[tmpPath[j]]);
        ans[idx] += ',';
    }
    ans[idx][ans[idx].size() - 1] = '\n';
    ++totalCircles;
}

// 正向遍历6层
void dfs6o(int head, int cur, int depth) 
{
    register int v, i, j;
    vis[cur] = true;
    tmpPath[tmpPathIdx++] = cur;

    for(i = 0; i < pOut[cur]; ++i) {
        v = Gout[cur][i];
        if(v < head || vis[v]) continue;                 // id < head 继续访问
        if (depth >= 4 && vis1[v] != tmpFlag) continue;  // 第四层以后有交集才访问
        bool flagRecv = true;
        // bool flagLastError = false;
        if(vis2[v] == tmpFlag && depth >= 2) {   // 存在路径得到一条结果
            tmpPath[tmpPathIdx++] = v;
            // 先判断答案是否合法
            // bool flagRecv = true;
            for(j = 0; j < tmpPathIdx; j++){ // tmpPathIdx 表示个数
                if(j == 0){
                    unsigned int leftMoney = money[(unsigned long long)(tmpPath[tmpPathIdx-1]) << 32 | tmpPath[0]];
                    unsigned int rightMoney = money[(unsigned long long)(tmpPath[0]) << 32 | tmpPath[1]];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        flagRecv = false;
                        break;
                    }
                }else if (j != (tmpPathIdx - 1)){
                    unsigned int leftMoney = money[(unsigned long long)(tmpPath[j-1]) << 32 | tmpPath[j]];
                    unsigned int rightMoney = money[(unsigned long long)(tmpPath[j]) << 32 | tmpPath[j+1]];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        // flagLastError = true;
                        flagRecv = false;
                        break;
                    }
                } else { // 最后
                    unsigned int leftMoney = money[(unsigned long long)(tmpPath[j-1]) << 32 | tmpPath[j]];
                    unsigned int rightMoney = money[(unsigned long long)(tmpPath[j]) << 32 | tmpPath[0]];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        // flagLastError = true;
                        flagRecv = false;
                        break;
                    }
                }
            }
            if(flagRecv){
                addans(depth - 2);  // depth-2ӳ��Ϊ0~4֮��
            }

            tmpPathIdx--;    
        }
        if (depth < 6 ) { // 减少迭代  && (flagRecv || flagLastError )
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
    maxNodeOut = ids[ids.size() - 1];
    for(i = 0; i < nodeCnt; ++i) {
        if (pOut[i]) {
            // 对出度所链接的节点进行排序，这样查找时id都是从小到大，对结果就不用排序了
            std::sort(Gout[i].begin(), Gout[i].end());
        }
    }
    for(i = 0; i < nodeCnt; ++i) {
        // 只有出入度不为0的节点才会构成环
        if(pOut[i] && pIn[i]) {
            tmpFlag = i + 1;  // for 6 + 1, 标记入度节点，head + 1 的目的是因为有id为0的节点
            // std::cout << "tmpFlag" << tmpFlag ;
            dfs3i(i, i, 1);
            dfs6o(i, i, 1);
        }
    }
    // std::cout << "[debug]" << pOut[10] << " " << ids[10] << " " << pIn[10];
}

void save(std::string &outputFile)
{
    std::ostringstream os;
    os << totalCircles << std::endl;
    FILE * fp = fopen(outputFile.c_str(), "w" );
    fwrite(os.str().c_str(), os.str().length(), 1, fp);
    register int i;
    for(i = 0; i < 5; i++) {
        fwrite(ans[i].c_str(), ans[i].size(), 1, fp);
    }
    fclose(fp);
}

int main()
{
    std::string testFile = "../std/stdin/2861665/test_data.txt";
    // std::string testFile = "/data/test_data.txt";
    std::string outputFile = "../std/testout/first7.txt";
    // std::string outputFile = "/projects/student/result.txt";
#ifdef TEST
    auto t0 = clock();
#endif
    parseInput(testFile); // 读取数据并构建图
    constructGraph();
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