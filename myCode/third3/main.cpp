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
// #define TEST
/* System parameter */
static const int MAX_NODE_COUNT = 1000000;
static const int MAX_EDGE_COUNT = 2000000;

/* store graph */ 
struct ArcNode {
    unsigned int adjvex; // 存储输入节点或者输出节点的信息
    double info; // 存储金额
};
std::vector<ArcNode> Gout[MAX_NODE_COUNT];
std::vector<ArcNode> Gin[MAX_NODE_COUNT];

std::vector<unsigned int> inputs;
std::vector<double> moneys;
std::set<unsigned int> mySet;
static std::vector<bool> vis;
static std::vector<int> vis1;             // dsf3i得到该数组, dfs6o遍历时与他做交集进行减枝
static std::vector<int> vis2;             // for 6 + 1, vis2[i] = head + 1 表示 i 是 head的入边节点
static std::string ans[6];
static int ansIndex[6];              // ans[i]
static int totalCircles = 0;
static unsigned int tmpPath[10];
static int tmpPathIdx = 0;
static int tmpLen = 0;
static int tmpFlag = 0;
static unsigned int nodeCnt = 0;
std::vector<double> tmpDist;
std::vector<unsigned int> ids; //0...n to sorted id
std::unordered_map<unsigned int, int> idHash; // sorted id to 0 ... n

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
    register unsigned int u, v;
    double c;
    while (fscanf(file, "%u,%u,%lf", &u, &v, &c) != EOF) {
        inputs.push_back(u);
        inputs.push_back(v);
        moneys.push_back(c);
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
    for(int i=0; i<sz; i+=2){
        unsigned int u = idHash[inputs[i]], v = idHash[inputs[i+1]];
        double c = moneys[i >> 1];
        Gout[u].push_back({v,c});
        Gin[v].push_back({u,c});
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
void dfs7o(int head, int cur, int depth) 
{
    register int v, i, j;
    vis[cur] = true;
    tmpPath[tmpPathIdx++] = cur;
    // tmpDist.push_back(Gout[cur][]) 已经在外面增加了
    for(i = 0; i < Gout[cur].size(); ++i) {
        v = Gout[cur][i].adjvex;
        tmpDist.push_back(Gout[cur][i].info);
        if(v < head || vis[v]) 
        {
            tmpDist.pop_back();
            continue;                 // id < head 继续访问
        }
        if (depth >= 5 && vis1[v] != tmpFlag) // 如果 vis1 没有遍历到那么结果就弹出 就是正向遍历
        {
            tmpDist.pop_back();
            continue;  // 第四层以后有交集才访问
        }
        bool flagRecv = true;
        // bool flagLastError = false;
        if(vis2[v] == tmpFlag && depth >= 2) {   // 存在路径得到一条结果 减少最后一层遍历
              register unsigned int low = 0, high = Gout[v].size() - 1;
                int tmp = -1;

                while(low <= high)
                {
                    int mid = low + (high - low) / 2;
                    if(Gout[v][mid].adjvex == head) {
                        tmp = mid;
                        break;
                    }
                    else if(Gout[v][mid].adjvex < head) low = mid + 1;
                    else high = mid - 1;
                }   

            tmpDist.push_back(Gout[v][tmp].info);
            // for(int k = 0; k < Gout[v].size(); k++){
            //     if(Gout[v][k].adjvex == head){
            //         // std::cout << "head" << Gout[v][k].info << std::endl;
            //         tmpDist.push_back(Gout[v][k].info);
            //     }   
            // }
            tmpPath[tmpPathIdx++] = v;
            // 先判断答案是否合法
            for(j = 0; j< tmpDist.size(); j++){
                if(j == 0){
                    double leftMoney = tmpDist[tmpDist.size() - 1];
                    double rightMoney = tmpDist[0];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        flagRecv = false;
                        break;
                    }
                }else {
                    double leftMoney = tmpDist[j - 1];
                    double rightMoney = tmpDist[j];
                    double scale = rightMoney * 1.0 / leftMoney;
                    if(!(scale >= 0.2 && scale <= 3)){
                        flagRecv = false;
                        break;
                    }
                }

            }
            if(flagRecv){
                addans(depth - 2);  // depth-2ӳ��Ϊ0~4֮��
            }
            // tmpDist.pop_back();
            tmpDist.pop_back();
            tmpPathIdx--;    
        }
        if (depth < 7 ) { // 减少迭代  && (flagRecv || flagLastError )
            // tmpDist.push_back(Gout[cur][i].info);
            dfs7o(head, v, depth + 1);
            // tmpDist.pop_back();
        }
        tmpDist.pop_back();
    }
    vis[cur] = false;
    tmpPathIdx--;
}

// 反向遍历3层：如果将图看做是无向图，一个点数为7的换中，距离起点最远的点距离不超做3
void dfs3i(int head, int cur, int depth) 
{
    register int v, i;
    vis[cur] = true;
    for(i = 0; i < Gin[cur].size(); ++i) {
        v = Gin[cur][i].adjvex;
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
    for(i = 0; i < nodeCnt; ++i) {
        if (Gout[i].size()) {
            // 对出度所链接的节点进行排序，这样查找时id都是从小到大，对结果就不用排序了
            std::sort(Gout[i].begin(), Gout[i].end(), cmp);
        }
    }
    for(i = 0; i < nodeCnt; ++i) {
        // 只有出入度不为0的节点才会构成环
        if(Gout[i].size() && Gin[i].size()) {
            tmpFlag = i + 1;  // for 6 + 1, 标记入度节点，head + 1 的目的是因为有id为0的节点
            dfs3i(i, i, 1);
            dfs7o(i, i, 1);
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
    for(i = 0; i < 6; i++) {
        fwrite(ans[i].c_str(), ans[i].size(), 1, fp);
    }
    fclose(fp);
}

int main()
{
    // std::string testFile = "../std/stdin/third/test_data.txt";// 2861665
    std::string testFile = "/data/test_data.txt";
    // std::string outputFile = "../std/testout/third3.txt";
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