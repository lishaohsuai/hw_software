#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <unordered_map>
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
    unsigned int info;
};

struct Graph {
    std::vector<ArcNode> out[MAX_NODE_COUNT];
    std::vector<unsigned int> in[MAX_NODE_COUNT];
};

static Graph g;
static bool vis[MAX_NODE_COUNT];
static unsigned int vis1[MAX_NODE_COUNT];
static unsigned int vis2[MAX_NODE_COUNT];

/* for int to char */
static char sortedCharIds[MAX_NODE_COUNT][12];

/* for output */
static char ans1[100000000];
static char ans2[100000000];
static char ans3[200000000];
static char ans4[300000000];
static char ans5[400000000];
static char* ans[5] = {ans1, ans2, ans3, ans4, ans5};
static int ansCnt[5];

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
        unsigned int i, u, v, c;
        std::vector<unsigned int> inputs;
        std::unordered_map<unsigned int, unsigned int> inputIdHash;
        std::unordered_map<unsigned long long, unsigned int> money;
        /* read file */
        FILE* fp = fopen(testFile.c_str(), "r");
        while (fscanf(fp, "%u,%u,%u", &u, &v, &c) != EOF) {
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
            sprintf(sortedCharIds[i], "%d\0", sortedIds[i]);
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
        unsigned int i, tmpLen;
        for(i = 0; i < length; ++i) {
            tmpLen = strlen(sortedCharIds[tmpPath[i]]);
            memcpy(ans[index] + ansCnt[index], sortedCharIds[tmpPath[i]], tmpLen);
            ansCnt[index] += tmpLen;
            ans[index][ansCnt[index]++] = ',';
        }
        ans[index][ansCnt[index] - 1] = '\n';
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
    
    void dfs6o(int head, int cur, int depth) 
    {
        unsigned int v, i, j, d;
        vis[cur] = true;
        tmpPath.push_back(cur);
        for(i = 0; i < g.out[cur].size(); ++i) {
            v = g.out[cur][i].adjvex;
            if(v < head || vis[v]) continue; 
            if (depth >= 4 && vis1[v] != tagValue) continue;
            if (depth >= 2) {
                if(check(tmpDist.back(), g.out[cur][i].info) == false) continue;
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
                if(check(tmpDist.back(), d) && check(d, tmpDist[0])) {
                    AddResult();
                }
                tmpPath.pop_back(); 
                tmpDist.pop_back();   
            }
            if (depth < 6) {
                tmpDist.push_back(g.out[cur][i].info);
                dfs6o(head, v, depth + 1);
                tmpDist.pop_back();
            }
        }
        vis[cur] = false;
        tmpPath.pop_back();
    }
    
    //  0.2 <= y/x <= 3
    bool check(unsigned int x, unsigned int y)
    {
        if(5 * (unsigned long long)y >= (unsigned long long)x && (unsigned long long)y <= 3 * (unsigned long long)x) {
            return true;
        }
        return false;
    }
    
    void Solve() 
    {
        unsigned int i;
        for(i = 0; i < nodeCount; ++i) {
            if(g.out[i].size() && g.in[i].size()) {
                tagValue = i + 1;
                dfs3i(i, i, 1);
                dfs6o(i, i, 1); 
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
        for(i = 0; i < 5; i++) {
            fwrite(ans[i], ansCnt[i], 1, fp);
        }
        fclose(fp);
    }
    
private:
    unsigned int tagValue;
    unsigned int circleCount;
    unsigned int nodeCount;
    std::vector<unsigned int> sortedIds;
    std::vector<unsigned int> tmpPath;
    std::vector<unsigned int> tmpDist;
};

int main()
{
    std::string testFile = "/data/test_data.txt";
    std::string outputFile = "/projects/student/result.txt";
    
    Application s;
    s.ParseInput(testFile);
    s.Solve();
    s.Save(outputFile);
    
    return 0;
}


