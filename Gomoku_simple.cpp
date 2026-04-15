#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

const int SIZE = 15;
int board[SIZE][SIZE] = {0};

int main() {
    int x, y, n;
    cin >> n;

    // 读取 n 个完整回合（每个回合：对方落子 + 本方落子）
    for (int i = 0; i < n; i++) {
        cin >> x >> y; if (x != -1) board[x][y] = -1; // 对方棋子
        cin >> x >> y; if (x != -1) board[x][y] = 1;  // 本方棋子（暂定黑色，交换后会互换）
    }
    // 读取对方最新一步（第 n+1 回合的对方落子）
    cin >> x >> y;
    if (x != -1) board[x][y] = -1;

    int new_x, new_y;

    // 一手交换：如果是白棋的第一回合（n==0 且对方下的是有效棋子），则交换
    if (x != -1 && n == 0) {
        new_x = -1; new_y = -1;   // 输出 -1 -1 表示交换
    } else {
        // 收集所有空位
        int avail_x[SIZE*SIZE], avail_y[SIZE*SIZE];
        int cnt = 0;
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                if (board[i][j] == 0) {
                    avail_x[cnt] = i;
                    avail_y[cnt] = j;
                    cnt++;
                }
        if (cnt == 0) {
            new_x = -1; new_y = -1; // 棋盘满了，随便输出（实际游戏已结束）
        } else {
            srand(time(0));
            int rand_pos = rand() % cnt;
            new_x = avail_x[rand_pos];
            new_y = avail_y[rand_pos];
        }
    }
    cout << new_x << " " << new_y << endl;
    return 0;
}