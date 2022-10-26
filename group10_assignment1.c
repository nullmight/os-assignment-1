#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/shm.h>
#include <limits.h>

const int COUNT = 8;
int size;
#define ITERATIONS 10000

int movesX[] = {1,1,2,2,-1,-1,-2,-2};
int movesY[] = {2,-2,1,-1,2,-2,1,-1};
int perm[19][4] = {
    {1, 2, 3, 4},
    {2, 1, 3, 4},
    {3, 1, 2, 4},
    {1, 3, 2, 4},
    {2, 3, 1, 4},
    {3, 2, 1, 4},
    {4, 2, 1, 3},
    {2, 4, 1, 3},
    {1, 4, 2, 3},
    {4, 1, 2, 3},
    {2, 1, 4, 3},
    {1, 2, 4, 3},
    {1, 3, 4, 2},
    {3, 1, 4, 2},
    {4, 1, 3, 2},
    {1, 4, 3, 2},
    {3, 4, 1, 2},
    {4, 3, 1, 2},
    {4, 3, 2, 1}
};
typedef struct {
    int x, y;
} pair;
 
bool isValidNext(int board[], int x, int y)
{
    return ((x >= 0 && y >= 0) && (x < size && y < size)) && (board[y*size+x] < 0);
}
 

int getDegree(int board[], int x, int y)
{
    int count = 0;
    for (int i = 0; i < COUNT; ++i)
        if (isValidNext(board, (x + movesX[i]), (y + movesY[i]))) count++;
 
    return count;
}
 

bool next(int board[], int *x, int *y, int index)
{
    int min_ind = -1, c, min_degree = (COUNT+1), nextX, nextY;
    int currX = *x, currY = *y;

    if (index == -1) {
        int start = rand()%COUNT;
        for (int count = 0; count < COUNT; ++count)
        {
            int i = (start + count)%COUNT;
            nextX = currX + movesX[i];
            nextY = currY + movesY[i];
            if ((isValidNext(board, nextX, nextY)) &&
            (c = getDegree(board, nextX, nextY)) < min_degree)
            {
                min_ind = i;
                min_degree = c;
            }
        }
    } else {
        for (int count = 0; count < COUNT; ++count)
        {
            int i = count >= 4 ? count : perm[index][count] - 1;
            nextX = currX + movesX[i];
            nextY = currY + movesY[i];
            if ((isValidNext(board, nextX, nextY)) &&
            (c = getDegree(board, nextX, nextY)) < min_degree)
            {
                min_ind = i;
                min_degree = c;
            }
        }
    }
 
    if (min_ind == -1) return false;
 
    nextX = currX + movesX[min_ind];
    nextY = currY + movesY[min_ind];
 
    board[nextY * size + nextX] = board[currY * size + currX] + 1;
 
    *x = nextX;
    *y = nextY;
 
    return true;
}

void print(int board[], int n)
{
    pair order[n * n + 1];
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j) {
            pair p = { .x = i, .y = j};
            order[board[n * j + i]] = p;
        } 
    }

    for(int i = 1; i <= n * n; i++) printf("%d,%d|", order[i].x, order[i].y);
}
int index1;
bool tourBoard(int start_x, int start_y)
{
    
    int board[ size * size ];
    for (int i = 0; i < size*size; ++i) board[i] = -1;

    int x = start_x, y = start_y;
    board[y * size + x] = 1; 
 
    if (index1 >= 19) {
        for (int i = 0; i < size * size - 1; ++i)
            if (next(board, &x, &y, -1) == 0) return false;
    } else {
        for (int i = 0; i < size * size - 1; ++i)
            if (next(board, &x, &y, index1) == 0) return false;
    }
    print(board, size);

    return true;
}
 
// Driver code
int main(int argc, char *argv[])
{

	size = atoi(argv[1]);
    int startX = atoi(argv[2]);
    int startY = atoi(argv[3]);

    if(size % 2 == 1 && (startX + startY) % 2 == 1) {
        printf("No Possible Tour");
        return 0;
    }
    
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        index1 = iter;
        if (iter < 19) {
            if (tourBoard(startX, startY)) {
                return 0;
            }
        } else {
            srand(time(NULL) + iter);
            if (tourBoard(startX, startY)) {
                return 0;
            }
        }            
        
    }

    printf("No Possible Tour");

    return 0;
}


