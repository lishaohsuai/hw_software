#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <thread>
#include <cstring>
#include <time.h>
#include <cstddef>
#include <typeinfo>
#include <set>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
static const int threadCount = 8;

// #define TEST
/* System parameter */
static const int MAX_NODE_COUNT = 800000;
static const int MAX_EDGE_COUNT = 2000000;

/* store graph */ 
struct ArcNode {
    unsigned int adjvex; // 存储输入节点或者输出节点的信息
    unsigned int info; // 存储金额
};
std::vector<ArcNode> Gout[MAX_NODE_COUNT];
std::vector<ArcNode> Gin[MAX_NODE_COUNT];

std::vector<unsigned int> inputs;
std::vector<unsigned int> inputss;
std::set<unsigned int> mySet;
static int tmpLen = 0;
// static int tmpFlag = 0;
static unsigned int nodeCnt = 0;
// std::vector<unsigned int> tmpDist;
std::vector<unsigned int> ids; //0...n to sorted id
std::unordered_map<unsigned int, int> idHash; // sorted id to 0 ... n

// 以下变量与线程有关，每个线程需要单独享有一份
static std::vector<bool> vis[threadCount];
static std::vector<int> vis1[threadCount];
static std::vector<int> vis2[threadCount];
static int totalCircles[threadCount];
static int tmpPath[threadCount][10];
static short int tmpPathIdx[threadCount];
static int tmpFlag[threadCount];
// for multi-thread
std::vector<unsigned int> tmpDist[threadCount];
static std::string ans[threadCount][5]; // 每个线程写入对应的数组


bool cmp(ArcNode x,ArcNode y)
{
    return x.adjvex < y.adjvex;
}

inline std::string int2Str(unsigned int num)
{
    if(num == 0){
        return "0";
    }
    std::string str = "", sign = "";
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
    while (fscanf(file, "%u,%u,%u", &u, &v, &c) != EOF) {
        inputs.push_back(u);
        inputs.push_back(v);
        inputs.push_back(c);
        mySet.insert(u);
        mySet.insert(v);
    }
    fclose(file);
}

void constructGraph(){
    auto tmp = inputs;
    nodeCnt = mySet.size();
    for(auto it=mySet.begin(); it!=mySet.end(); it++){
        ids.push_back(*it);
    }
    // 压缩矩阵
    nodeCnt = 0;
    for(unsigned int &x : ids){
        idHash[x] = nodeCnt++;
    }
    int sz = inputs.size();
    for(int i=0; i<sz; i+=3){
        unsigned int u = idHash[inputs[i]], v = idHash[inputs[i+1]],c = inputs[i+2];
        Gout[u].push_back({v,c});
        Gin[v].push_back({u,c});
    }
    for(int i=0; i<threadCount; i++){
        vis[i] = std::vector<bool>(nodeCnt, false);
        vis1[i] = std::vector<int>(nodeCnt, 0);
        vis2[i] = std::vector<int>(nodeCnt, 0);
    }
}


void addans(int idx, int threadNo)
{
    register int j;
    for(j = 0; j < tmpPathIdx[threadNo]; ++j) {
        ans[threadNo][idx] += int2Str(ids[tmpPath[threadNo][j]]);
        ans[threadNo][idx] += ',';
    }
    ans[threadNo][idx][ans[threadNo][idx].size() - 1] = '\n';
    ++totalCircles[threadNo];
}

// 正向遍历6层
void dfs6o(int head, int cur, int depth, int threadNo) 
{
    register int v, i, j;
    vis[threadNo][cur] = true;
    tmpPath[threadNo][tmpPathIdx[threadNo]++] = cur;
    // tmpDist.push_back(Gout[cur][]) 已经在外面增加了
    for(i = 0; i < Gout[cur].size(); ++i) {
        v = Gout[cur][i].adjvex;
        tmpDist[threadNo].push_back(Gout[cur][i].info);
        if(v < head || vis[threadNo][v]) 
        {
            tmpDist[threadNo].pop_back();
            continue;                 // id < head 继续访问
        }
        if (depth >= 4 && vis1[threadNo][v] != tmpFlag[threadNo]) 
        {
            tmpDist[threadNo].pop_back();
            continue;  // 第四层以后有交集才访问
        }
        bool flagRecv = true;
        // bool flagLastError = false;
        if(vis2[threadNo][v] == tmpFlag[threadNo] && depth >= 2) {   // 存在路径得到一条结果
            // tmpDist.push_back(Gout[cur][i].info);// 增加最后一条路径的值就是最后一个节点到第一个节点的路径
            for(int k = 0; k < Gout[v].size(); k++){
                if(Gout[v][k].adjvex == head){
                    tmpDist[threadNo].push_back(Gout[v][k].info);
                    break;
                }   
            }
            tmpPath[threadNo][tmpPathIdx[threadNo]++] = v;
            // 先判断答案是否合法
            for(j = 0; j< tmpDist[threadNo].size(); j++){
                if(j == 0){
                    unsigned int leftMoney = tmpDist[threadNo][tmpDist[threadNo].size() - 1];
                    unsigned int rightMoney = tmpDist[threadNo][0];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        flagRecv = false;
                        break;
                    }
                }else {
                    unsigned int leftMoney = tmpDist[threadNo][j - 1];
                    unsigned int rightMoney = tmpDist[threadNo][j];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        flagRecv = false;
                        break;
                    }
                }

            }
            if(flagRecv){
                addans(depth - 2, threadNo);  // depth-2ӳ��Ϊ0~4֮��
            }
            // tmpDist.pop_back();
            tmpDist[threadNo].pop_back();
            tmpPathIdx[threadNo]--;    
        }
        if (depth < 6 ) { // 减少迭代  && (flagRecv || flagLastError )
            // tmpDist.push_back(Gout[cur][i].info);
            dfs6o(head, v, depth + 1, threadNo);
            // tmpDist.pop_back();
        }
        tmpDist[threadNo].pop_back();
    }
    vis[threadNo][cur] = false;
    tmpPathIdx[threadNo]--;
}

// 反向遍历3层：如果将图看做是无向图，一个点数为7的换中，距离起点最远的点距离不超做3
void dfs3i(int head, int cur, int depth, int threadNo) 
{
    register int v, i;
    vis[threadNo][cur] = true;
    for(i = 0; i < Gin[cur].size(); ++i) {
        v = Gin[cur][i].adjvex;
        if(v < head || vis[threadNo][v]) continue;
        vis1[threadNo][v] = tmpFlag[threadNo];          // 记录距离起点反向距离不超过3的所有点，为了后面做交集
        if(depth == 1) {            // for 6 + 1, 记录head的直接入度节点，即最后一层
            vis2[threadNo][v] = tmpFlag[threadNo] ;     // 标记
        }
        if (depth < 3) {
            dfs3i(head, v, depth + 1, threadNo);
        }
    }
    vis[threadNo][cur] = false;
}

void process(int start, int end, int threadNo){
    register int i;
	for(i = start; i < end; ++i) {
		tmpFlag[threadNo] = i + 1;  // for 6 + 1， 标记入度节点， head + 1的目的是因为有id为0的节点 
		dfs3i(i, i, 1, threadNo);
		dfs6o(i, i, 1, threadNo); // 先不管压不压缩看看结果对不对把
    }
}

void solve() 
{
    register int i, idx = 0;
    for(i = 0; i < nodeCnt; ++i) {
        if (Gout[i].size()) {
            // 对出度所链接的节点进行排序，这样查找时id都是从小到大，对结果就不用排序了
            std::sort(Gout[i].begin(), Gout[i].end(), cmp);
            // if(Gin[i].size()){
            //     inputss.push_back(i);
            // }
        }
    }
    // 划分区间
    idx = nodeCnt;
    int d = idx >> 3;
    int s1 = 0, e1 = s1 + d;
    int s2 = e1, e2 = s2 + d;
    int s3 = e2, e3 = s3 + d;
    int s4 = e3, e4 = s4 + d;
    int s5 = e4, e5 = s5 + d;
    int s6 = e5, e6 = s6 + d;
    int s7 = e6, e7 = s7 + d;
    int s8 = e7, e8 = s8 + d;
    std::thread t1 = std::thread(&process, s1, e1, 0);
    std::thread t2 = std::thread(&process, s2, e2, 1);
    std::thread t3 = std::thread(&process, s3, e3, 2);
    std::thread t4 = std::thread(&process, s4, e4, 3);
    std::thread t5 = std::thread(&process, s5, e5, 4);
    std::thread t6 = std::thread(&process, s6, e6, 5);
    std::thread t7 = std::thread(&process, s7, e7, 6);
    // std::thread t8 = std::thread(&process, s8, e8, 7);
    process(s8, idx, 7);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
}

void save(std::string &outputFile)
{
    // std::ostringstream os;
    int sum = 0;
    for(int i=0; i < threadCount; i++){
        sum += totalCircles[i];
    }
    std::string total = int2Str(sum);
    total += '\n';
    // os << totalCircles[0] + totalCircles[1] + totalCircles[2] + totalCircles[3]  << std::endl;
    FILE * fp = fopen(outputFile.c_str(), "w" );
    fwrite(total.c_str(), total.length(), 1, fp);
    register int i, j;
    for(i = 0; i < 5; i++) {
    	for(j = 0; j < threadCount; j++) {
    		fwrite(ans[j][i].c_str(), ans[j][i].size(), 1, fp);
		} 
    }
    fclose(fp);
}

int main()
{
    // std::string testFile = "../std/stdin/2861665/test_data.txt";
    std::string testFile = "/data/test_data.txt";
    // std::string outputFile = "../std/testout/first5.txt";
    std::string outputFile = "/projects/student/result.txt";
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