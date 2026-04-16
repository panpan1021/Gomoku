
int getCount(int board[SIZE][SIZE], int x, int y, int dir, int player):
参数:
board是棋盘,
[x,y]是坐标,
dir是四个方向,
补充：规定一下dir方向(0：横向；1：纵向，2：右斜向；3.左斜向)
player:
黑棋为1,白起为-1
功能:
获取在某一个方向的连珠个数

pair<bool, bool> getBlockStatus(int board[SIZE][SIZE], int x, int y, int dir, int player):
参数:
同上不多bb
功能:
获取在一个方向两端的阻塞情况
//-----上面这两个函数都是为了计算一个位置对于某一方的得分

int patternScore(int len, bool leftBlock, bool rightBlock):
参数:
len是连珠长度
剩下俩是一个方向两端阻塞情况
功能:
计算一个位置一个方向上的得分

int evaluatePlayer(int board[SIZE][SIZE], int player):
参数:

功能:
遍历每个位置,每个方向,将其相加作为某一方的总分

int evaluateBoard(int board[SIZE][SIZE], int myPlayer):
参数
myplayer代表你也就是ai
功能:
计算双方总分差值

vector<pair<int, int>> generateMoves(int board[SIZE][SIZE]):
功能:将所有可能得走法都存入move数组

void orderMoves(int board[SIZE][SIZE], vector<pair<int, int>>& moves, int player):
参数:
move是下一步可能得所有位置
功能:
把所有可能得位置遍历,计算总差分后回溯,并排序

int alphaBeta(int board[SIZE][SIZE], int depth, int alpha, int beta, bool isMax, int player, int& bestX, int& bestY)
核心函数,自己看的时候很蒙蔽,剪枝理解好一会
参数:
depth是咱们AI要博弈的层数,相当于下棋人说的"我能看5步",(ps:耀正只能看2步,[笑哭])

alpha对于AI来说是我在当前层能博弈出来的最优情况,在函数中:这个只有是在这一步是ai的回合的时候才会更新,beta在ai的回合不会更新,beta是上层递归调用传进来的,所以当alpha>=beta的时候就直接剪枝

beta是对于对手来说你能博弈出来的对ai最坏情况,在函数中:这个只有是这一步是对手的回合才会跟新,alpha不会跟新,当alpha>=beta剪枝

isMax是用于判断当前递归层是ai在博弈还是对手在博弈

best和besty是引用变量,作为输出参数返回给调用者

关于剪枝:假设我现在在ai的博弈层:beta的上层对手博弈出来的对我最坏情况,那么,如果说我继续往下一层博弈,得到的alpha>=beta那么就说明最好情况大于我的最坏情况,所以这个最好情况必须舍弃,完成剪枝

pair<int, int> getBestMove(int board[SIZE][SIZE], int player):
给main的最终接口,不多bb
