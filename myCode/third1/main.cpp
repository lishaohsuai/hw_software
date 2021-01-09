/*
 * @Author: your name
 * @Date: 2020-04-24
 *
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <cstring>
#include <vector>

#ifdef _LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif

//-------------------------------------------------------------------------------------------------

/* System parameter */
static const int MAX_NODE_COUNT = 1000000;

struct ArcNode {
    unsigned int adjvex;
    double info;
};

struct Graph {
    std::vector<ArcNode> out[MAX_NODE_COUNT];
    std::vector<unsigned int> in[MAX_NODE_COUNT];
};

static Graph g;
static bool vis[MAX_NODE_COUNT];
static unsigned int vis1[MAX_NODE_COUNT];
static unsigned int vis2[MAX_NODE_COUNT];

//-------------------------------------------------------------------------------------------------

int cmp1(struct ArcNode a, struct ArcNode b)
{
    return a.adjvex < b.adjvex;
}

class Application{
public:
    Application() : circleCount(0), nodeCount(0)
    {
    }
    ~Application()
    {
    }
    
public:
    void ParseInput(std::string &testFile)
    {
        unsigned int i;
        unsigned int  u, v;
        double c;
        std::vector<unsigned int> inputs;
        std::unordered_map<unsigned int, unsigned int> inputIdHash;
        std::unordered_map<unsigned long long, double> money;
        std::
        /* read file */
        FILE* fp = fopen(testFile.c_str(), "r");
        while (fscanf(fp, "%u,%u,%lf", &u, &v, &c) != EOF) {
            inputs.push_back(u);
            inputs.push_back(v);
            money.emplace((unsigned long long)u << 32 | v, c);
        }
        fclose(fp);
        /* init sortedIds */
        sortedIds = inputs;
        sort(sortedIds.begin(), sortedIds.end());
        sortedIds.erase(unique(sortedIds.begin(), sortedIds.end()), sortedIds.end());
        nodeCount = sortedIds.size();
        for(i = 0; i < nodeCount; ++i) {
            inputIdHash[sortedIds[i]] = i;
        }
        /* init Graph */
        ArcNode arc;
        unsigned int sz = inputs.size();
        for (i = 0; i < sz; i += 2) {
            u = inputIdHash[inputs[i]];
            v = inputIdHash[inputs[i + 1]];
            c = money[(unsigned long long)inputs[i] << 32 | inputs[i + 1]];
            arc.adjvex = v;
            arc.info = c;
            g.out[u].push_back(arc);
            g.in[v].push_back(u);
        }
        /* sort g.out[i] */
        for(i = 0; i < nodeCount; ++i) {
            if (g.out[i].size()) {
                sort(g.out[i].begin(), g.out[i].end(), cmp1);
            }
        }
    }
    
    void AddResult()
    {
        unsigned int length = tmpPath.size();
        unsigned int index = length - 3;
        unsigned i;
        for(i = 0; i < length - 1; ++i) {
            oss[index] << sortedIds[tmpPath[i]] << ",";
        }
        oss[index] << sortedIds[tmpPath.back()] << "\n";
        ++circleCount;
    }
    
    void dfs3i(int head, int cur, int depth) 
    {
        unsigned int v, i;
        vis[cur] = true;
        for(i = 0; i < g.in[cur].size(); ++i) {
            v = g.in[cur][i];
            if(v < head || vis[v]) continue;
            vis1[v] = tagValue;
            if (depth == 1) {
                vis2[v] = tagValue;
            }
            if (depth < 3) {
                dfs3i(head, v, depth + 1);
            }
        }
        vis[cur] = false;
    }
    
    void dfs7o(int head, int cur, int depth) 
    {
        unsigned int v, i, j;
        double d;
        vis[cur] = true;
        tmpPath.push_back(cur);
        for(i = 0; i < g.out[cur].size(); ++i) {
            v = g.out[cur][i].adjvex;
            if(v < head || vis[v]) continue; 
            if (depth >= 5 && vis1[v] != tagValue) continue;
            if (depth >= 2) {
                double rate = g.out[cur][i].info * 1.0 / tmpDist.back();
                if(rate < 0.2 || rate > 3) continue;
            }
            if(vis2[v] == tagValue && depth >= 2) {
                tmpPath.push_back(v);
                tmpDist.push_back(g.out[cur][i].info);
                
                for(j = 0; j < g.out[v].size(); j++) {
                    if(g.out[v][j].adjvex == head) {
                        d = g.out[v][j].info;
                        break;
                    }
                }
                double rate1 = d * 1.0 / tmpDist.back();
                double rate2 = tmpDist[0] * 1.0 / d;
                if(rate1 >= 0.2 && rate1 <= 3 && rate2 >= 0.2 && rate2 <= 3) {
                    AddResult();
                }
                tmpPath.pop_back(); 
                tmpDist.pop_back();   
            }
            if (depth < 7) {
                tmpDist.push_back(g.out[cur][i].info);
                dfs7o(head, v, depth + 1);
                tmpDist.pop_back();
            }
        }
        vis[cur] = false;
        tmpPath.pop_back();
    }
    
    void Solve() 
    {
        unsigned int i;
        for(i = 0; i < nodeCount; ++i) {
            if(g.out[i].size() && g.in[i].size()) {
                tagValue = i + 1;
                dfs3i(i, i, 1);
                dfs7o(i, i, 1); 
            }
        }
    }
    
    void Save(std::string &outputFile)
    {
        std::ostringstream os;
        os << circleCount << std::endl;
        FILE * fp = fopen(outputFile.c_str(), "w" );
        fwrite(os.str().c_str(), os.str().length(), 1, fp);
        unsigned int i;
        for(i = 0; i < 6; i++) {
            fwrite(oss[i].str().c_str(), oss[i].str().length(), 1, fp);
        }
        fclose(fp);
    }
    
private:
    unsigned int tagValue;
    unsigned int circleCount;
    unsigned int nodeCount;
    std::ostringstream oss[6];
    std::vector<unsigned int> sortedIds;
    std::vector<unsigned int> tmpPath;
    std::vector<double> tmpDist;
};

// #define TEST
int main()
{
    std::string testFile = "/data/test_data.txt";
    // std::string testFile = "../std/stdin/third/test_data.txt";
    std::string outputFile = "/projects/student/result.txt";
    // std::string outputFile = "../std/testout/third1.txt";
    


    Application s;
    #ifdef TEST
    auto t0 = clock();
    #endif
    s.ParseInput(testFile);
    #ifdef TEST
    auto t1 = clock();
    std::cout << "[DEBUG] 读取文件并构建图所消耗时间" <<  t1 - t0 << std::endl;
    #endif
    s.Solve();
    #ifdef TEST
    auto t2 = clock();
    std::cout << "[DEBUG] 处理时间" << t2 - t1 << std::endl;
    #endif
    s.Save(outputFile);
#ifdef TEST
    auto t3 = clock();
    std::cout << "[DEBUG] 存储文件时间" << t3 - t2 << std::endl;
#endif  
    return 0;
}


