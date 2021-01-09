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

#ifdef _LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define _CRT_SECURE_NO_WARNINGS      // ��ֹ fopen �� fscanf ����



static const int threadCount = 4;
static const int MAXID = 220000;
/* ��������洢ͼ */
static int Gout[MAXID][50];       // ��¼����id
static int Gin[MAXID][50];        // ��¼���id
static int pOut[MAXID];           // Gout[i]��������
static int pIn[MAXID];            // Gin[i]��������
static int inputs[MAXID];
static unsigned int maxNodeOut;   // �г��ȵĽ������id 

/* ���±������߳���أ�ÿ���߳���Ҫ��������һ�� */ 
static bool vis[threadCount][MAXID];
static int vis1[threadCount][MAXID];       // dsf3i�õ�������, dfs6o����ʱ�������������м�֦
static int vis2[threadCount][MAXID];       // for 6 + 1, vis2[i] = head + 1 ��ʾi��head����߽��  
static int totalCircles[threadCount];
static int tmpPath[threadCount][10];
static short int tmpPathIdx[threadCount];
static int tmpFlag[threadCount];

/* for multi-thread */
static char ans[threadCount][5][90000000]; // ÿ���߳̽����д���Ӧ������
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

// �������6�� 
void dfs6o(int head, int cur, int depth, int threadNo) 
{
    register int v, i;
    vis[threadNo][cur] = true;
    tmpPath[threadNo][tmpPathIdx[threadNo]] = cur;
    ++tmpPathIdx[threadNo];
    for(i = 0; i < pOut[cur]; ++i) {
        v = Gout[cur][i];
        if(v < head || vis[threadNo][v]) continue;                 // id < head�ż������� 
        if (depth >= 4 && vis1[threadNo][v] != tmpFlag[threadNo]) continue;  // ���Ĳ��Ժ��н����ŷ���(��֦) 
        
        if(vis2[threadNo][v] == tmpFlag[threadNo] && depth >= 2) {   // ����·�����õ�һ����� 
            tmpPath[threadNo][tmpPathIdx[threadNo]] = v;
            ++tmpPathIdx[threadNo];
            addans(depth - 2, threadNo);  // depth-2ӳ��Ϊ0~4֮��
            --tmpPathIdx[threadNo];    
        }
        if (depth < 6) {
            dfs6o(head, v, depth + 1, threadNo);
        }
    }
    vis[threadNo][cur] = false;
    --tmpPathIdx[threadNo];
}

// �������3��: �����ͼ����������ͼ��һ������Ϊ7�Ļ��У����������Զ�ĵ���벻����3
// ����Ѱ�ҿ��Ե�������Ϊ4���ڵ�����·����������˵BFS���Щ�����������£��о������� 
void dfs3i(int head, int cur, int depth, int threadNo) 
{
    register int v, i;
    vis[threadNo][cur] = true;
    for(i = 0; i < pIn[cur]; ++i) {
        v = Gin[cur][i];
        if(v < head || vis[threadNo][v]) continue;
        vis1[threadNo][v] = tmpFlag[threadNo];          // ��¼������㷴����벻����3�����н�㣬Ϊ�˺��������� 
        if(depth == 1) {                                // for 6 + 1, ��¼head��ֱ����Ƚ�㣬�����һ��
            vis2[threadNo][v] = tmpFlag[threadNo] ;     // ���
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
		tmpFlag[threadNo] = inputs[i] + 1;  // for 6 + 1�������Ƚ�㣬head+1��Ŀ������Ϊ��idΪ0�Ľ�� 
		dfs3i(inputs[i], inputs[i], 1, threadNo);
		dfs6o(inputs[i], inputs[i], 1, threadNo);
    }
}

void solve() 
{
    register int i, idx = 0;
    for(i = 0; i <= maxNodeOut; ++i) {
        if (pOut[i]) {
            // �Գ��������ӵĽ�����������������ʱid���Ǵ�С���󣬶Խ���Ͳ������� 
            std::sort(Gout[i], Gout[i] + pOut[i]);
            if(pIn[i]) {
            	inputs[idx++] = i;
			}
        }
    }
    
    // ��������
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
    std::string testFile = "/data/test_data.txt";
    std::string outputFile = "/projects/student/result.txt";
    parseInput(testFile); // ��ȡ���ݲ�����ͼ 
    solve();
    save(outputFile);
    return 0;
}

