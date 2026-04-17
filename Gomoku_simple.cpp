#include<iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
using namespace std;
const int SIZE = 15;
int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0
// 评估函数所用常数
const int WIN_SCORE = 1000000;
const int LIVE4_SCORE = 100000;
const int SLEEP4_SCORE = 10000;
const int LIVE3_SCORE = 5000;
const int SLEEP3_SCORE = 800;
const int LIVE2_SCORE = 200;
const int SLEEP2_SCORE = 20;
// 方向向量：水平、垂直、主对角线、次对角线
const int dx[4] = { 1, 0, 1, 1 };
const int dy[4] = { 0, 1, 1, -1 };
// 判断坐标是否在棋盘内
inline bool inBoard(int x, int y) {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}
// 获取某个方向从 (x,y) 开始连续相同棋子的个数（包括自身）
int getCount(int board[SIZE][SIZE], int x, int y, int dir, int player) {
    int cnt = 1;
    int nx = x + dx[dir], ny = y + dy[dir];
    while (inBoard(nx, ny) && board[nx][ny] == player) {
        cnt++;
        nx += dx[dir];
        ny += dy[dir];
    }
    nx = x - dx[dir], ny = y - dy[dir];
    while (inBoard(nx, ny) && board[nx][ny] == player) {
        cnt++;
        nx -= dx[dir];
        ny -= dy[dir];
    }
    return cnt;
}
// 判断某个方向的两端是否被阻塞（返回左阻塞, 右阻塞）
pair<bool, bool> getBlockStatus(int board[SIZE][SIZE], int x, int y, int dir, int player) {
    bool leftBlock = true, rightBlock = true;
    int nx = x + dx[dir], ny = y + dy[dir];
    while (inBoard(nx, ny) && board[nx][ny] == player) {
        nx += dx[dir];
        ny += dy[dir];
    }
    if (inBoard(nx, ny) && board[nx][ny] == 0) rightBlock = false;
    nx = x - dx[dir], ny = y - dy[dir];
    while (inBoard(nx, ny) && board[nx][ny] == player) {
        nx -= dx[dir];
        ny -= dy[dir];
    }
    if (inBoard(nx, ny) && board[nx][ny] == 0) leftBlock = false;
    return { leftBlock, rightBlock };
}
// 根据长度和阻塞情况打分
int patternScore(int len, bool leftBlock, bool rightBlock) {
    if (len >= 5) return WIN_SCORE;
    if (len == 4) {
        if (!leftBlock && !rightBlock) return LIVE4_SCORE;      // 活四
        else return SLEEP4_SCORE;                               // 冲四（眠四）
    }
    if (len == 3) {
        if (!leftBlock && !rightBlock) return LIVE3_SCORE;      // 活三
        else return SLEEP3_SCORE;                               // 眠三
    }
    if (len == 2) {
        if (!leftBlock && !rightBlock) return LIVE2_SCORE;      // 活二
        else return SLEEP2_SCORE;                               // 眠二
    }
    return 0;
}
// 评估整个棋盘上某一方（player）的总分
int evaluatePlayer(int board[SIZE][SIZE], int player) {
    int total = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] != player) continue;
            // 对四个方向分别评估
            for (int d = 0; d < 4; d++) {
                // 为了避免重复计算，只计算以当前点为起点的方向（通过方向限制）
                // 简单起见，每个方向都算，会有重复但影响不大（效率可接受）
                int cnt = getCount(board, i, j, d, player);
                auto [leftBlock, rightBlock] = getBlockStatus(board, i, j, d, player);
                total += patternScore(cnt, leftBlock, rightBlock);
            }
        }
    }
    return total;
}
// 总评估函数：本方得分减去对方得分（防守系数0.9）
int evaluateBoard(int board[SIZE][SIZE], int myPlayer) {
    int myScore = evaluatePlayer(board, myPlayer);
    int oppScore = evaluatePlayer(board, -myPlayer);
    return myScore - oppScore * 0.9;
}
// 生成所有可能的走法：只考虑已有棋子周围2格内的空位
vector<pair<int, int>> generateMoves(int board[SIZE][SIZE]) {
    vector<pair<int, int>> moves;
    bool hasPiece = false;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] != 0) {
                hasPiece = true;
                for (int dx = -2; dx <= 2; dx++) {
                    for (int dy = -2; dy <= 2; dy++) {
                        int nx = i + dx, ny = j + dy;
                        if (inBoard(nx, ny) && board[nx][ny] == 0) {
                            moves.emplace_back(nx, ny);
                        }
                    }
                }
            }
        }
    }
    if (!hasPiece) {
        // 空棋盘，默认下天元
        moves.emplace_back(7, 7);
    }
    // 去重
    sort(moves.begin(), moves.end());
    moves.erase(unique(moves.begin(), moves.end()), moves.end());
    return moves;
}

// 给每个走法一个初始评分（基于下子后的估值变化）
void orderMoves(int board[SIZE][SIZE], vector<pair<int, int>>& moves, int player) {
    vector<pair<int, pair<int, int>>> scored;
    for (auto& mv : moves) {
        int x = mv.first, y = mv.second;
        board[x][y] = player;
        int score = evaluateBoard(board, player);
        board[x][y] = 0;
        scored.emplace_back(-score, mv); // 降序排列，负号实现从大到小
    }
    sort(scored.begin(), scored.end());
    moves.clear();
    for (auto& p : scored) {
        moves.push_back(p.second);
    }
}

// Alpha-Beta 搜索
int alphaBeta(int board[SIZE][SIZE], int depth, int alpha, int beta, bool isMax, int player, int& bestX, int& bestY) {
    if (depth == 0) {
        return evaluateBoard(board, player);
    }
    vector<pair<int, int>> moves = generateMoves(board);
    if (moves.empty()) return evaluateBoard(board, player);

    if (isMax) {
        int maxEval = INT_MIN;
        orderMoves(board, moves, player);
        for (auto& mv : moves) {
            int x = mv.first, y = mv.second;
            board[x][y] = player;
            int eval = alphaBeta(board, depth - 1, alpha, beta, false, player, bestX, bestY);
            board[x][y] = 0;
            if (eval > maxEval) {
                maxEval = eval;
                if (depth == 3) { // 根节点记录最佳走法
                    bestX = x;
                    bestY = y;
                }
            }
            alpha = max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    }
    else {
        int minEval = INT_MAX;
        orderMoves(board, moves, -player);
        for (auto& mv : moves) {
            int x = mv.first, y = mv.second;
            board[x][y] = -player;
            int eval = alphaBeta(board, depth - 1, alpha, beta, true, player, bestX, bestY);
            board[x][y] = 0;
            if (eval < minEval) {
                minEval = eval;
            }
            beta = min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

// 获取最佳落子
pair<int, int> getBestMove(int board[SIZE][SIZE], int player) {
    int bestX = -1, bestY = -1;
    alphaBeta(board, 3, INT_MIN, INT_MAX, true, player, bestX, bestY);
    if (bestX == -1) {
        // 回退：简单遍历第一个合法空位
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (board[i][j] == 0) return { i, j };
    }
    return { bestX, bestY };
}

int main()
{
    int x, y, n;
    cin >> n;

    // 读取 n 个完整回合（每个回合：对方落子 + 本方落子）
    for (int i = 0; i < n; i++)
    {
        cin >> x >> y;
        if (x != -1)
            board[x][y] = -1; // 对方棋子
        cin >> x >> y;
        if (x != -1)
            board[x][y] = 1; // 本方棋子（暂定黑色，交换后会互换）
    }
    // 读取对方最新一步（第 n+1 回合的对方落子）
    cin >> x >> y;
    if (x != -1)
        board[x][y] = -1;
    //---------------------------------------------------------------------------------
    int new_x = -1, new_y = -1;

    // 判断是否处于“一手交换”决策时刻：
    // 条件：当前是第一个回合（n==0），并且对方刚刚下了一子（不是(-1,-1)），且棋盘上总棋子数为1
    // 注意：n已经读取，最后一个对方落子已读入board，但本方尚未行动
    // 我们还需要知道本方原本是黑还是白？根据输入规则，如果n==0且对方落子不是(-1,-1)，则本方为后手（白方）
    // 此时需要决定是否交换。
    int totalPieces = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] != 0) totalPieces++;

    // 判断：是否本方还没有下过任何棋子（即总棋子数==1，且刚读入的对方落子有效）
    if (totalPieces == 1 && n == 0) {
        // 找到黑棋第一手的位置
        int firstX = -1, firstY = -1;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (board[i][j] != 0) { firstX = i; firstY = j; break; }
        // 简单启发式：如果第一手离中心太远（欧氏距离 > 5），则交换；否则不交换
        double dist = sqrt((firstX - 7) * (firstX - 7) + (firstY - 7) * (firstY - 7));
        if (dist > 5.0) {
            // 交换：输出 (-1, -1)
            new_x = -1;
            new_y = -1;
        }
        else {
            // 不交换：正常下棋，调用搜索
            auto move = getBestMove(board, 1); // 当前本方为白方，但board中1代表本方，所以player=1
            new_x = move.first;
            new_y = move.second;
        }
    }
    else {
        // 正常对局，直接调用搜索
        auto move = getBestMove(board, 1); // 本方始终为1
        new_x = move.first;
        new_y = move.second;
    }

    //---------------------------------------------------------------------------------
    cout << new_x << " " << new_y << endl;
    return 0;
}