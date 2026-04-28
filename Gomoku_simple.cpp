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
#include <unordered_map>
using namespace std;

const int SIZE = 15;

// 评估函数所用常数（增大差距，提高区分度）
const int WIN_SCORE = 10000000;
const int LIVE4_SCORE = 1000000;
const int SLEEP4_SCORE = 100000;
const int LIVE3_SCORE = 10000;
const int SLEEP3_SCORE = 1000;
const int LIVE2_SCORE = 500;
const int SLEEP2_SCORE = 50;

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

// 优化的评估函数：避免重复计算，只计算有效棋子位置
int evaluatePlayer(int board[SIZE][SIZE], int player) {
    int total = 0;
    bool visited[SIZE][SIZE][4] = { false }; // 记录每个位置在每个方向是否已计算
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] != player) continue;
            
            for (int d = 0; d < 4; d++) {
                if (visited[i][j][d]) continue; // 跳过已计算的方向
                
                int cnt = getCount(board, i, j, d, player);
                auto [leftBlock, rightBlock] = getBlockStatus(board, i, j, d, player);
                total += patternScore(cnt, leftBlock, rightBlock);
                
                // 标记该方向上的所有棋子为已计算
                int nx = i, ny = j;
                while (inBoard(nx, ny) && board[nx][ny] == player) {
                    visited[nx][ny][d] = true;
                    nx += dx[d];
                    ny += dy[d];
                }
                nx = i - dx[d];
                ny = j - dy[d];
                while (inBoard(nx, ny) && board[nx][ny] == player) {
                    visited[nx][ny][d] = true;
                    nx -= dx[d];
                    ny -= dy[d];
                }
            }
        }
    }
    return total;
}

// 总评估函数：本方得分减去对方得分（防守系数1.2）
int evaluateBoard(int board[SIZE][SIZE], int myPlayer) {
    int myScore = evaluatePlayer(board, myPlayer);
    int oppScore = evaluatePlayer(board, -myPlayer);
    return myScore - oppScore * 1.2;
}

// 生成所有可能的走法：只考虑已有棋子周围2格内的空位
vector<pair<int, int>> generateMoves(int board[SIZE][SIZE]) {
    vector<pair<int, int>> moves;
    bool hasPiece = false;
    bool added[SIZE][SIZE] = { false }; // 避免重复
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] != 0) {
                hasPiece = true;
                for (int dx = -2; dx <= 2; dx++) {
                    for (int dy = -2; dy <= 2; dy++) {
                        int nx = i + dx, ny = j + dy;
                        if (inBoard(nx, ny) && board[nx][ny] == 0 && !added[nx][ny]) {
                            moves.emplace_back(nx, ny);
                            added[nx][ny] = true;
                        }
                    }
                }
            }
        }
    }
    if (!hasPiece) {
        moves.emplace_back(7, 7);
    }
    return moves;
}

// 历史启发表
int historyHeuristic[SIZE][SIZE] = { 0 };

// 给每个走法一个初始评分（基于评估值和历史启发）
void orderMoves(int board[SIZE][SIZE], vector<pair<int, int>>& moves, int player) {
    vector<pair<int, pair<int, int>>> scored;
    for (auto& mv : moves) {
        int x = mv.first, y = mv.second;
        board[x][y] = player;
        int eval = evaluateBoard(board, player);
        board[x][y] = 0;
        // 结合历史启发
        int score = eval + historyHeuristic[x][y] * 100;
        scored.emplace_back(-score, mv); // 降序排列
    }
    sort(scored.begin(), scored.end());
    moves.clear();
    for (auto& p : scored) {
        moves.push_back(p.second);
    }
}

// 置换表条目
struct TTEntry {
    int depth;
    int score;
    int flag; // 0: exact, 1: lower bound, 2: upper bound
};

// 简单的置换表实现
unordered_map<long long, TTEntry> transpositionTable;

// 棋盘状态哈希
long long hashBoard(int board[SIZE][SIZE]) {
    long long hash = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            hash = hash * 3 + (board[i][j] + 1); // -1, 0, 1 -> 0, 1, 2
        }
    }
    return hash;
}

int searchStartTime;
int maxDepth;

// Alpha-Beta 搜索（带置换表和历史启发）
int alphaBeta(int board[SIZE][SIZE], int depth, int alpha, int beta, bool isMax, int player, int& bestX, int& bestY, int currentDepth) {
    // 超时检查
    if ((clock() - searchStartTime) / (double)CLOCKS_PER_SEC > 0.95) {
        return evaluateBoard(board, player);
    }
    
    if (depth == 0) {
        return evaluateBoard(board, player);
    }
    
    long long hash = hashBoard(board);
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end()) {
        TTEntry& entry = it->second;
        if (entry.depth >= depth) {
            if (entry.flag == 0) return entry.score;
            else if (entry.flag == 1 && entry.score >= beta) return entry.score;
            else if (entry.flag == 2 && entry.score <= alpha) return entry.score;
        }
    }
    
    vector<pair<int, int>> moves = generateMoves(board);
    if (moves.empty()) return evaluateBoard(board, player);
    
    int bestScore;
    int flag = 2; // 默认上界
    
    if (isMax) {
        bestScore = INT_MIN;
        orderMoves(board, moves, player);
        
        for (auto& mv : moves) {
            int x = mv.first, y = mv.second;
            board[x][y] = player;
            
            int eval = alphaBeta(board, depth - 1, alpha, beta, false, player, bestX, bestY, currentDepth + 1);
            
            board[x][y] = 0;
            
            if (eval > bestScore) {
                bestScore = eval;
                if (currentDepth == 0) {
                    bestX = x;
                    bestY = y;
                }
            }
            
            alpha = max(alpha, eval);
            
            if (beta <= alpha) {
                historyHeuristic[x][y] += depth * depth; // 历史启发加分
                break;
            }
        }
        if (bestScore >= beta) flag = 1; // 下界
        else if (bestScore <= alpha) flag = 2; // 上界
        else flag = 0; // 精确值
    } else {
        bestScore = INT_MAX;
        orderMoves(board, moves, -player);
        
        for (auto& mv : moves) {
            int x = mv.first, y = mv.second;
            board[x][y] = -player;
            
            int eval = alphaBeta(board, depth - 1, alpha, beta, true, player, bestX, bestY, currentDepth + 1);
            
            board[x][y] = 0;
            
            if (eval < bestScore) {
                bestScore = eval;
            }
            
            beta = min(beta, eval);
            
            if (beta <= alpha) {
                historyHeuristic[x][y] += depth * depth;
                break;
            }
        }
        if (bestScore >= beta) flag = 1;
        else if (bestScore <= alpha) flag = 2;
        else flag = 0;
    }
    
    // 存入置换表
    transpositionTable[hash] = {depth, bestScore, flag};
    
    return bestScore;
}

// 获取最佳落子（使用迭代加深）
pair<int, int> getBestMove(int board[SIZE][SIZE], int player) {
    searchStartTime = clock();
    int bestX = -1, bestY = -1;
    int currentDepth = 1;
    
    // 迭代加深搜索
    while ((clock() - searchStartTime) / (double)CLOCKS_PER_SEC < 0.9) {
        int tempX = -1, tempY = -1;
        alphaBeta(board, currentDepth, INT_MIN, INT_MAX, true, player, tempX, tempY, 0);
        
        if ((clock() - searchStartTime) / (double)CLOCKS_PER_SEC < 0.9) {
            bestX = tempX;
            bestY = tempY;
            maxDepth = currentDepth;
            currentDepth++;
        } else {
            break;
        }
    }
    
    if (bestX == -1) {
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
    
    int board[SIZE][SIZE] = { 0 };
    
    // 读取 n 个完整回合（每个回合：对方落子 + 本方落子）
    for (int i = 0; i < n; i++)
    {
        cin >> x >> y;
        if (x != -1)
            board[x][y] = -1; // 对方棋子
        cin >> x >> y;
        if (x != -1)
            board[x][y] = 1; // 本方棋子
    }
    
    // 读取对方最新一步
    cin >> x >> y;
    if (x != -1)
        board[x][y] = -1;
    
    int new_x = -1, new_y = -1;
    
    // 计算总棋子数
    int totalPieces = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] != 0) totalPieces++;
    
    // 判断是否处于"一手交换"决策时刻
    if (totalPieces == 1 && n == 0) {
        // 找到黑棋第一手的位置
        int firstX = -1, firstY = -1;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (board[i][j] != 0) { firstX = i; firstY = j; break; }
        
        // 改进的交换策略：距离中心小于4格则交换，否则不交换
        double dist = sqrt((firstX - 7) * (firstX - 7) + (firstY - 7) * (firstY - 7));
        if (dist <= 4.0) {
            new_x = -1;
            new_y = -1;
        } else {
            auto move = getBestMove(board, 1);
            new_x = move.first;
            new_y = move.second;
        }
    } else {
        auto move = getBestMove(board, 1);
        new_x = move.first;
        new_y = move.second;
    }
    
    cout << new_x << " " << new_y << endl;
    return 0;
}