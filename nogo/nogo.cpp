// 3 * bfs() + 新scatter

// 不围棋（NoGo）样例程序
// 随机策略
// 作者：fffasttime
// 游戏信息：http://www.botzone.org/games#NoGo
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <ctime>
#include "jsoncpp/json.h"
const int N = 600000;
const int INF = 0x3f3f3f3f;
using namespace std;

int board[9][9];
int rounds;
clock_t start, finish;

bool dfs_air_visit[9][9];
const int cx[] = { -1,0,1,0 };
const int cy[] = { 0,-1,0,1 };

int q[N + 100];
int first, last;

bool inBorder(int x, int y) { return x >= 0 && y >= 0 && x < 9 && y < 9; }

//true: has air
bool dfs_air(int fx, int fy)
{
	dfs_air_visit[fx][fy] = true;
	bool flag = false;
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] == 0)
				flag = true;
			if (board[dx][dy] == board[fx][fy] && !dfs_air_visit[dx][dy])
				if (dfs_air(dx, dy))
					flag = true;
		}
	}
	return flag;
}

//true: available
bool judgeAvailable(int fx, int fy, int col)
{
	if (board[fx][fy]) return false;
	board[fx][fy] = col;
	memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
	if (!dfs_air(fx, fy))
	{
		board[fx][fy] = 0;
		return false;
	}
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] && !dfs_air_visit[dx][dy])
				if (!dfs_air(dx, dy))
				{
					board[fx][fy] = 0;
					return false;
				}
		}
	}
	board[fx][fy] = 0;
	return true;
}

int valuePoint(int fx, int fy, int col)
{
	board[fx][fy] = col;
	int p = 0;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
		{
			if (judgeAvailable(i, j, col) && !judgeAvailable(i, j, col * -1))
				p++;
			if (!judgeAvailable(i, j, col) && judgeAvailable(i, j, col * -1))
				p--;
		}
	board[fx][fy] = 0;
	return p;
}

int d(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

int scatter()
{
	int m = 0;
	vector<int> res;
	for (int i = first; i != last; i = (i + 1) % N)
	{
		int temp = 0;
		while (q[i])
		{
			temp = q[i] % 81;
			q[i] /= 81;
		}

		int n = INF;
		for (int x = 0; x < 9; x++)
			for (int y = 0; y < 9; y++)
				if (board[x][y] == -1 && (temp / 9 != x || temp / 9 != y) && d(temp / 9, temp % 9, x, y) < n)
					n = d(temp / 9, temp % 9, x, y);
		if (n > m)
		{
			m = n;
			res.clear();
			res.push_back(temp);
		}
		else if (n == m)
			res.push_back(temp);
	}

	if (res.size() != 1)
		return res[rand() % res.size()];
	else
		return res[0];
}

int findBest()
{
	if (!board[0][0] && judgeAvailable(0, 0, -1))
		return 0;

	first = last = 0;
	int m = -INF;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			if (judgeAvailable(i, j, -1))
			{
				int v = valuePoint(i, j, -1);
				if (v > m)
				{
					m = v;
					last = first;
					q[last++] = i * 9 + j;
				}
				else if (v == m)
					q[last++] = i * 9 + j;
			}

	int depth = 0;
	while (last != (first + 1) % N && depth++ < 2)
	{
		int l = last;
		m = INF * (depth % 2 ? 1 : -1);
		for (int k = first; k < l; k = (k + 1) % N)
		{
			if ((double)(clock() - start) / CLOCKS_PER_SEC > 0.99)
			{
				last = l;
				return scatter();
			}

			int temp = q[k];
			for (int i = 0; i < depth; i++)
			{
				board[(temp / 9) % 9][temp % 9] = ((depth - i) % 2) ? -1 : 1;
				rounds++;
				temp /= 81;
			}

			int tm = -INF;
			vector<int> tpoint;
			for (int i = 0; i < 9; i++)
				for (int j = 0; j < 9; j++)
					if (judgeAvailable(i, j, (depth % 2) ? 1 : -1))
					{
						int v = valuePoint(i, j, (depth % 2) ? 1 : -1);
						if (v > tm)
						{
							tm = v;
							tpoint.clear();
							tpoint.push_back(i * 9 + j + q[k] * 81);
						}
						else if (v == m)
							tpoint.push_back(i * 9 + j + q[k] * 81);
					}

			if (depth % 2)
			{
				if (tm <= m)
				{
					if (tm < m)
					{
						m = tm;
						last = l;
					}
					for (int i : tpoint)
					{
						q[last] = i;
						last = (last + 1) % N;
					}
				}
			}
			else
			{
				if (tm >= m)
				{
					if (tm > m)
					{
						m = tm;
						last = l;
					}
					for (int i : tpoint)
					{
						q[last] = i;
						last = (last + 1) % N;
					}
				}
			}

			temp = q[k];
			for (int i = 0; i < depth; i++)
			{
				board[(temp / 9) % 9][temp % 9] = 0;
				rounds--;
				temp /= 81;
			}

			if (l == last && depth % 2)
			{
				int ret = 0;
				while (q[k])
				{
					ret = q[k] % 81;
					q[k] /= 81;
				}
				return ret;
			}
		}
		if (l == last)
			break;
		first = l;
	}

	if (last != (first + 1) % N)
		return scatter();
	int ret = 0;
	while (q[first])
	{
		ret = q[first] % 81;
		q[first] /= 81;
	}
	return ret;
}

int main()
{
	srand((unsigned)time(0));
	string str;
	int x, y;
	// 读入JSON
	getline(cin, str);
	//getline(cin, str);

	start = clock();
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);
	// 分析自己收到的输入和自己过往的输出，并恢复状态
	int turnID = input["responses"].size();
	rounds = turnID + input["requests"].size();
	for (int i = 0; i < turnID; i++)
	{
		x = input["requests"][i]["x"].asInt(), y = input["requests"][i]["y"].asInt();
		if (x != -1)
			board[x][y] = 1;
		else
			rounds--;
		x = input["responses"][i]["x"].asInt(), y = input["responses"][i]["y"].asInt();
		if (x != -1)
			board[x][y] = -1;
		else
			rounds--;
	}
	x = input["requests"][turnID]["x"].asInt(), y = input["requests"][turnID]["y"].asInt();
	if (x != -1)
		board[x][y] = 1;
	else
		rounds--;
	// 输出决策JSON
	Json::Value ret;
	Json::Value action;

	int res = findBest();

	action["x"] = res / 9; action["y"] = res % 9;
	ret["response"] = action;
	Json::FastWriter writer;

	cout << writer.write(ret) << endl;
	return 0;
}