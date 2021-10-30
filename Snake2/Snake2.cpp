#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include<ctime>
#include<cmath>
#include"jsoncpp/json.h"
const int N = 1000000;
const int INF = 0x3f3f3f3f;

using namespace std;
int n, m;
const int MAXN = 25;
const int dx[4] = { -1,0,1,0 };
const int dy[4] = { 0,1,0,-1 };
int invalid[MAXN][MAXN];
int num;
int snake[2][N];
int front[2];
int rear[2];

double c = 0.818;//信心上限函数权值

clock_t start;

struct node
{
	int dir[2];
	int validSize[2];
	int validDir[2][3];
	int count;//子节点个数
	int total;
	int score[2];
	node* parent;
	node* children[9];
	node()
	{
		count = 0;
		total = 0;
		parent = NULL;
		memset(dir, 0, sizeof(dir));
		memset(validSize, 0, sizeof(validSize));
		memset(validDir, 0, sizeof(validDir));
		memset(score, 0, sizeof(score));
		memset(children, 0, sizeof(children));
	}
};
typedef node* pn;

pn root;

bool whetherGrow()  //本回合是否生长
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}

void deleteEnd(int id)     //删除蛇尾
{
	int pos = snake[id][rear[id]++];
	invalid[pos / MAXN][pos % MAXN] = 0;

}

void move(int id, int dir)  //编号为id的蛇朝向dir方向移动一步
{
	int p = snake[id][front[id] - 1];
	int x = p / MAXN + dx[dir];
	int y = p % MAXN + dy[dir];
	snake[id][front[id]++] = x * MAXN + y;
	invalid[x][y] = id + 2;
}

bool validDirection(int id, int k)  //判断当前移动方向的下一格是否合法
{
	int p = snake[id][front[id] - 1];
	int oth = id == 0 ? 1 : 0;
	int pp = snake[oth][front[oth] - 1];
	if (p == pp)
		return 0;
	int x = p / MAXN + dx[k];
	int y = p % MAXN + dy[k];
	if (x > n || y > m || x < 1 || y < 1)
		return false;
	if (invalid[x][y] <= 3 && invalid[x][y] >= 1)
		return false;
	return true;
}


int getValidDir(int* dir, int id)
{
	int ret = 0;
	int flag = 0;
	if (!whetherGrow())
	{
		flag = 1;
		deleteEnd(0);
		deleteEnd(1);
	}

	for (int i = 0; i < 4; i++)
		if (validDirection(id, i))
			dir[ret++] = i;

	if (flag)
	{
		int pos = snake[0][--rear[0]];
		invalid[pos / MAXN][pos % MAXN] = 2;
		pos = snake[1][--rear[1]];
		invalid[pos / MAXN][pos % MAXN] = 3;
	}

	return ret;
}

void backMove()
{
	int pos = snake[0][--front[0]];
	invalid[pos / MAXN][pos % MAXN] = 0;
	pos = snake[1][--front[1]];
	invalid[pos / MAXN][pos % MAXN] = 0;
	if (num > 9 && (num - 9) % 3 != 0)
	{
		pos = snake[0][--rear[0]];
		invalid[pos / MAXN][pos % MAXN] = 2;
	}
	if (num > 9 && (num - 9) % 3 != 0)
	{
		pos = snake[1][--rear[1]];
		invalid[pos / MAXN][pos % MAXN] = 3;
	}

}

int randMove()
{
	if (snake[0][front[0] - 1] == snake[1][front[1] - 1])
		return -1;

	int possibleDir[3], con = 0;
	con = getValidDir(possibleDir, 0);
	if (!con)
		return 1;
	int dir = possibleDir[rand() % con];

	con = 0;
	con = getValidDir(possibleDir, 1);
	if (!con)
		return 0;

	if (!whetherGrow())
	{
		deleteEnd(0);
		deleteEnd(1);
	}

	::move(0, dir);
	dir = possibleDir[rand() % con];
	::move(1, dir);

	num++;
	int key = randMove();
	num--;
	backMove();
	return key;
}

pn bestchild(pn p)
{
	if (p->count == 1)
		return p->children[0];
	pn t[4][4];
	double ucb1[4];
	double ucb2[4];
	memset(ucb1, 0, sizeof(ucb1));
	memset(ucb2, 0, sizeof(ucb2));
	memset(t, 0, sizeof(t));
	for (int i = 0; i < p->count; i++)
	{
		t[p->children[i]->dir[0]][p->children[i]->dir[1]] = p->children[i];
		ucb1[p->children[i]->dir[0]] += 1.0 * p->children[i]->score[0] / p->children[i]->total + c * sqrt(log(p->total) / p->children[i]->total);
		ucb2[p->children[i]->dir[1]] += 1.0 * p->children[i]->score[1] / p->children[i]->total + c * sqrt(log(p->total) / p->children[i]->total);
	}
	int max1 = 0;
	int max2 = 0;
	for (int i = 0; i < 4; i++)
	{
		if (ucb1[i] > ucb1[max1])
			max1 = i;
		if (ucb2[i] > ucb2[max2])
			max2 = i;
	}

	return t[max1][max2];
}

void backup(pn p, int res)
{
	while (p != root)
	{
		p->total++;
		if (res > 0)
			p->score[res] += 2;
		else
		{
			p->score[0] += 1;
			p->score[1] += 1;
		}
		num--;
		backMove();
		p = p->parent;
	}
	p->total++;
}

pn MTCS()
{
	if (root->validSize[0] == 1)
	{
		pn ret = new node;
		ret->dir[0] = root->validDir[0][0];
		return ret;
	}

	pn tail = root;
	while ((double)(clock() - start) / CLOCKS_PER_SEC < 0.99)
	{
		tail = root;
		while (tail->validSize[0] && tail->validSize[1])
		{
			if (tail->count < (tail->validSize[0] * tail->validSize[1]))
			{
				pn q = new node();
				tail->children[tail->count] = q;
				q->parent = tail;
				q->dir[0] = tail->validDir[0][tail->count % tail->validSize[0]];
				q->dir[1] = tail->validDir[1][tail->count / tail->validSize[0]];
				tail->count++;
				if (!whetherGrow())
				{
					deleteEnd(0);
					deleteEnd(1);
				}
				::move(0, q->dir[0]);
				::move(1, q->dir[1]);
				num++;

				if (snake[0][front[0] - 1] != snake[1][front[1] - 1])
				{
					q->validSize[0] = getValidDir(q->validDir[0], 0);
					q->validSize[1] = getValidDir(q->validDir[1], 1);
				}

				tail = q;
				break;
			}
			else
			{
				tail = bestchild(tail);
				if (!whetherGrow())
				{
					deleteEnd(0);
					deleteEnd(1);
				}
				::move(0, tail->dir[0]);
				::move(1, tail->dir[1]);
				num++;
			}
		}

		int res = randMove();
		backup(tail, res);
	}

	pn t[4][4];
	double ucb[4];
	memset(ucb, 0, sizeof(ucb));
	memset(t, 0, sizeof(t));
	for (int i = 0; i < root->count; i++)
	{
		t[root->children[i]->dir[0]][root->children[i]->dir[1]] = root->children[i];
		ucb[root->children[i]->dir[0]] += 1.0 * root->children[i]->score[0] / root->children[i]->total;
	}

	int ma = 0;
	for (int i = 0; i < 4; i++)
		if (ucb[i] > ucb[ma])
			ma = i;

	for (int i = 0; i < 4; i++)
		if (t[ma][i])
			return t[ma][i];
	return NULL;
}

int main()
{
	string str;
	string temp;
	getline(cin, str);
	start = clock();
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);

	n = input["requests"][(Json::Value::UInt)0]["height"].asInt();  //棋盘高度
	m = input["requests"][(Json::Value::UInt)0]["width"].asInt();   //棋盘宽度

	int x = input["requests"][(Json::Value::UInt)0]["x"].asInt();  //读蛇初始化的信息
	if (x == 1)
	{
		snake[0][front[0]++] = 1 * MAXN + 1;
		snake[1][front[1]++] = n * MAXN + m;
		invalid[1][1] = 2;
		invalid[n][m] = 3;
	}
	else
	{
		snake[1][front[1]++] = 1 * MAXN + 1;
		snake[0][front[0]++] = n * MAXN + m;
		invalid[1][1] = 3;
		invalid[n][m] = 2;
	}
	//处理地图中的障碍物
	int obsCount = input["requests"][(Json::Value::UInt)0]["obstacle"].size();

	for (int i = 0; i < obsCount; i++)
	{
		int ox = input["requests"][(Json::Value::UInt)0]["obstacle"][(Json::Value::UInt)i]["x"].asInt();
		int oy = input["requests"][(Json::Value::UInt)0]["obstacle"][(Json::Value::UInt)i]["y"].asInt();
		invalid[ox][oy] = 1;
	}

	//根据历史信息恢复现场
	int total = input["responses"].size();

	int dir;
	for (int i = 0; i < total; i++)
	{
		num = i;

		if (!whetherGrow())
		{
			deleteEnd(0);
			deleteEnd(1);
		}

		dir = input["responses"][i]["direction"].asInt();
		::move(0, dir);
		dir = input["requests"][i + 1]["direction"].asInt();
		::move(1, dir);
	}

	num = total;
	root = new node();

	root->validSize[0] = getValidDir(root->validDir[0], 0);
	root->validSize[1] = getValidDir(root->validDir[1], 1);

	srand((unsigned)time(0) + total);

	pn response = MTCS();
	int ans;
	if (response == NULL)
		ans = root->validDir[0][rand() % root->validSize[0]];
	else
		ans = response->dir[0];

	Json::Value ret;
	ret["response"]["direction"] = ans;
	Json::FastWriter writer;
	cout << writer.write(ret) << endl;
	return 0;
}