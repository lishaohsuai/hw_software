/*
 * @Author: yanghh
 * @Date: 2020-05-11
 *
 */

//#define _LINUX
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
static const int MAX_NODE_COUNT = 600000;
static const int MAX_EDGE_COUNT = 2000000;

/* store graph */ 
struct ArcNode {
    unsigned int adjvex;
    unsigned int info;
};
std::vector<ArcNode> Gout[MAX_NODE_COUNT];
std::vector<ArcNode> Gin[MAX_NODE_COUNT];

/* for dfs && 5+2 && ºÙ÷¶ */ 
static bool vis[MAX_NODE_COUNT];
static unsigned int vis1[MAX_NODE_COUNT];
static unsigned int vis2[MAX_NODE_COUNT];
std::vector<unsigned int> rPath2[MAX_NODE_COUNT];

/* for inputs */
static char sortedCharIds[MAX_NODE_COUNT][11];
static unsigned int sortedIds[MAX_EDGE_COUNT << 1];
static unsigned int inputs[MAX_EDGE_COUNT << 1];
static unsigned int moneys[MAX_EDGE_COUNT];
static unsigned int inputsCnt;
static unsigned int moneysCnt;

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
        register unsigned int i, u, v, c;
        /* read file */
#ifdef _LINUX
        int fd = open(testFile.c_str(), O_RDONLY);
        int len = lseek(fd, 0, SEEK_END);
        char *mbuf = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        for(i = 0; i < len; ++i) {
            u = CharToInt(mbuf, ',' , i);
            v = CharToInt(mbuf, ',' , i);
            c = CharToInt(mbuf, '\r', i);
            inputs[inputsCnt++] = u;
            inputs[inputsCnt++] = v;
            moneys[moneysCnt++] = c;
        }
        munmap(mbuf, len);
#else
        FILE* fp = fopen(testFile.c_str(), "r");
        while (fscanf(fp, "%u,%u,%u", &u, &v, &c) != EOF) {
            inputs[inputsCnt++] = u;
            inputs[inputsCnt++] = v;
            moneys[moneysCnt++] = c;
        }
        fclose(fp);
#endif
        /* init sortedIds */
        memcpy(sortedIds, inputs, inputsCnt * sizeof(unsigned int));
        std::sort(sortedIds, sortedIds + inputsCnt);
        nodeCount = RemoveDup(sortedIds, inputsCnt);
        for(i = 0; i < nodeCount; ++i) {
            sprintf(sortedCharIds[i], "%d\0", sortedIds[i]);
        }
        /* init Graph */
        for (i = 0; i < inputsCnt; i += 2) {
            u = HalfSearch1(sortedIds, inputs[i]);
            v = HalfSearch1(sortedIds, inputs[i + 1]);
            c = moneys[i >> 1];
            Gout[u].push_back({v,c});
            Gin[v].push_back({u,c});
        }
        /* sort g.out[i] */
        for(i = 0; i < nodeCount; ++i) {
            sort(Gout[i].begin(), Gout[i].end(), cmp1);
            sort(Gin[i].begin(), Gin[i].end(), cmp1);
        }
    }
    
    unsigned int CharToInt(char buf[], char c, unsigned int &p)
    {
        register unsigned int res = 0;
        for(; buf[p] != c; ++p) {
            res = res * 10 + (buf[p] - '0');
        }
        ++p;
        return res;
    }
    
    int RemoveDup(unsigned int num[], int len)
    {
        register unsigned int slow = 0, fast = 0;
        while (fast < len)
        {
            if (num[fast] != num[slow])
            {
                ++slow;
                num[slow] = num[fast];
            }
            ++fast;
        }
        return slow + 1;
    }
    
    int HalfSearch1(unsigned int data[], unsigned int val) 
    {
        register unsigned int low = 0, high = nodeCount - 1;
        while(low <= high)
        {
            int mid = (high + low) / 2;
            if(data[mid] == val) return mid;
            else if(data[mid] < val) low = mid + 1;
            else high = mid - 1;
        }   
        return -1;
    } 
    
    int HalfSearch2(std::vector<ArcNode> &data, unsigned int val) 
    {
        register unsigned int low = 0, high = data.size() - 1;
        while(low <= high)
        {
            int mid = (high + low) / 2;
            if(data[mid].adjvex == val) return mid;
            else if(data[mid].adjvex < val) low = mid + 1;
            else high = mid - 1;
        }   
        return -1;
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
    
    void dfs3i(int head, int cur, int depth, unsigned int preMoney) 
    {
        unsigned int v, i;
        vis[cur] = true;
        for(i = 0; i < Gin[cur].size(); ++i) {
            v = Gin[cur][i].adjvex;
            if(v < head || vis[v]) continue;
            if(depth >= 2 && check(Gin[cur][i].info, preMoney) == false) {
                continue;
            }
            vis1[v] = tagValue;
            if (depth == 2) {
                if(vis2[v] != tagValue) {
                    rPath2[v].clear();
                }
                vis2[v] = tagValue;
                rPath2[v].push_back(cur);
            }
            if (depth < 3) {
                dfs3i(head, v, depth + 1, Gin[cur][i].info);
            }
        }
        vis[cur] = false;
    }
    
    void dfs6o(int head, int cur, int depth) 
    {
        unsigned int v, c, i, j, m1, m2;
        vis[cur] = true;
        tmpPath.push_back(cur);
        for(i = 0; i < Gout[cur].size(); ++i) {
            v = Gout[cur][i].adjvex;
            if(v < head || vis[v]) continue; 
            if (depth >= 4 && vis1[v] != tagValue) continue;
            if (depth >= 2) {
                if(check(tmpDist.back(), Gout[cur][i].info) == false) continue;
            }
            if(vis2[v] == tagValue) {
                tmpPath.push_back(v);
                tmpDist.push_back(Gout[cur][i].info);
                
                for(j = 0; j < rPath2[v].size(); ++j) {
                    c = rPath2[v][j];
                    if(vis[c]) continue;
                    m1 = Gout[v][HalfSearch2(Gout[v], c)].info;
                    m2 = Gout[c][HalfSearch2(Gout[c], head)].info;
                    if(check(tmpDist.back(), m1) && check(m2, tmpDist[0])) {
                        tmpPath.push_back(c);
                        AddResult();
                        tmpPath.pop_back(); 
                    }
                }
                
                tmpPath.pop_back(); 
                tmpDist.pop_back();   
            }
            if (depth < 5) {
                tmpDist.push_back(Gout[cur][i].info);
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
        if(5ll * y >= x && y <= 3ll * x) {
            return true;
        }
        return false;
    }
    
    void Solve() 
    {
        unsigned int i;
        for(i = 0; i < nodeCount; ++i) {
            if(Gout[i].size() && Gin[i].size()) {
                tagValue = i + 1;
                dfs3i(i, i, 1, 0);
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
    std::vector<unsigned int> tmpPath;
    std::vector<unsigned int> tmpDist;
};

int main01()
{
    std::string testFile = "/data/test_data.txt";
    std::string outputFile = "/projects/student/result.txt";
    
    Application s;
    s.ParseInput(testFile);
    s.Solve();
    s.Save(outputFile);
    
    return 0;
}



