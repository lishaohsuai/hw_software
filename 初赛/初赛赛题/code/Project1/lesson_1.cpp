/*
 * @Author: your name
 * @Date: 2020-04-17 10:43:11
 * @LastEditTime: 2020-04-17 10:43:42
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \code\lesson_1.cpp
 */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <time.h>
#define TEST
#define _CRT_SECURE_NO_WARNINGS 
// 防止fopen 和 fscanf 报错

struct Path {
	//ID最小的第一个输出；
	//总体按照循环转账路径长度升序排序；
	//同一级别的路径长度下循环转账账号ID序列，按照字典序（ID转为无符号整数后）升序排序
	int length;
	std::vector<unsigned int> path;

	Path(int length, const std::vector<unsigned int> &path) : length(length), path(path) {}

	bool operator<(const Path&rhs)const {//在排序的时候有用
		if (length != rhs.length) return length < rhs.length; // length 从小到大
		for (int i = 0; i < length; i++) {
			if (path[i] != rhs.path[i])// 字典序
				return path[i] < rhs.path[i];
		}
	}
};

class Solution {
public:
	//maxN=560000
	//maxE=280000 ~avgN=26000
	//vector<int> *G;
	std::vector<std::vector<int>> G;// 定义二维图
	std::unordered_map<unsigned int, int> idHash; //sorted id to 0...n
	std::vector<unsigned int> ids; //0...n to sorted id
	std::vector<unsigned int> inputs; //u-v pairs  先存储账号A 再存储账号B
	std::vector<int> inDegrees;
	std::vector<bool> vis;
	std::vector<Path> ans;
	int nodeCnt;

	void parseInput(std::string &testFile) {
		FILE* file = fopen(testFile.c_str(), "r");
		unsigned int u, v, c;
		int cnt = 0;
		while (fscanf(file, "%u,%u,%u", &u, &v, &c) != EOF) {
			inputs.push_back(u);
			inputs.push_back(v);
			++cnt;
		}
#ifdef TEST
		printf("%d Records in Total\n", cnt);// 输出总共存储了多少条数据
#endif
	}

	void constructGraph() {
		auto tmp = inputs;
		sort(tmp.begin(), tmp.end());// 对所有id 进行排序是什么意思？？
#ifdef TEST
		std::cout << "开始输出数据" << tmp.size() << std::endl;
		for (int i = 0; i < 10; i++) {
			std::cout << tmp[i] << std::endl;
		}
		std::cout << "结束输出数据" << std::endl;
#endif
		tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());//独一无二的tmp了
#ifdef TEST
		std::cout << "开始输出数据" << tmp.size() << std::endl;
		for (int i = 0; i < 10; i++) {
			std::cout << tmp[i] << std::endl;
		}
		std::cout << "结束输出数据" << std::endl;
#endif
		nodeCnt = tmp.size();
		ids = tmp;
		nodeCnt = 0;
		for (unsigned int &x : tmp) {
			idHash[x] = nodeCnt++;//map 中存储对应的账号id 所映射的位置？？？
		}
#ifdef TEST
		printf("%d Nodes in Total\n", nodeCnt);
#endif
		int sz = inputs.size();
		//G=new vector<int>[nodeCnt];
		G = std::vector<std::vector<int>>(nodeCnt);
		inDegrees = std::vector<int>(nodeCnt, 0);// 对于每一个节点的入度
		for (int i = 0; i < sz; i += 2) {
			int u = idHash[inputs[i]], v = idHash[inputs[i + 1]];
			G[u].push_back(v);//u节点指向v节点
			++inDegrees[v];// v节点的入度增加
		}
	}

	void dfs(int head, int cur, int depth, std::vector<int> &path) {
		vis[cur] = true;//当前节点已经访问过
		path.push_back(cur);//路径中存入当前节点
		for (int &v : G[cur]) {
			if (v == head && depth >= 3 && depth <= 7) { // 如果当前节点的一个出度是头结点，那么就是一个答案
				std::vector<unsigned int> tmp;
				for (int &x : path)
					tmp.push_back(ids[x]);
				ans.emplace_back(Path(depth, tmp));//等于push_back 不会触发 不需要触发拷贝构造和转移构造
			}
			if (depth<7 && !vis[v] && v>head) {
				dfs(head, v, depth + 1, path);
			}
		}
		vis[cur] = false;
		path.pop_back();
	}

	//search from 0...n
	//由于要求id最小的在前，因此搜索的全过程中不考虑比起点id更小的节点
	void solve() {
		vis = std::vector<bool>(nodeCnt, false);
		std::vector<int> path;
		for (int i = 0; i < nodeCnt; i++) {
			if (i % 100 == 0)
				std::cout << i << "/" << nodeCnt << std::endl;//每100个节点输出一个字符
			if (!G[i].empty()) {// 如果节点i对应的节点 有出度的话开启这个节点的dfs
				dfs(i, i, 1, path);
			}
		}
		sort(ans.begin(), ans.end());
	}

	void save(std::string &outputFile) {
		printf("Total Loops %d\n", (int)ans.size());
		std::ofstream out(outputFile);
		out << ans.size() << std::endl;
		for (auto &x : ans) {
			auto path = x.path;
			int sz = path.size();
			out << path[0];
			for (int i = 1; i < sz; i++)
				out << "," << path[i];
			out << std::endl;
		}
	}
};

int main()
{
	std::string testFile = "C:/Users/lee/Desktop/code/华为软件精英挑战赛/初赛/初赛赛题/test_data.txt";
	std::string outputFile = "C:/Users/lee/Desktop/code/华为软件精英挑战赛/初赛/初赛赛题/result_data.txt";
#ifdef TEST
	std::string answerFile = "C:/Users/lee/Desktop/code/华为软件精英挑战赛/初赛/初赛赛题/result.txt";
#endif
	auto t = clock();
	//    for(int i=0;i<100;i++){
	Solution solution;// 构建一个解决方案类
	solution.parseInput(testFile); // 读取数据将 id1,id2,money  id1和id2 存储在 std::vector<unsigned int> inputs; 
	solution.constructGraph();// 构建图
	//solution.topoSort();
	solution.solve();//解决它
	solution.save(outputFile);
	std::cout << clock() - t << std::endl;
	//    }
	system("pause");
	return 0;
}