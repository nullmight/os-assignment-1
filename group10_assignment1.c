#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

#define ITERATIONS 24
#define COUNT 8
 
int size;
int start_x, start_y;
int iter;
 
pthread_mutex_t lock;
key_t shmkey;
int* data,shmid;
 
int movesX[] = {1,1,2,2,-1,-1,-2,-2};
int movesY[] = {2,-2,1,-1,2,-2,1,-1};
int perm[] = {0, 1, 2, 3};
 
typedef struct {
    int x, y;
} pair;

bool isValidNext(int board[], int x, int y)
{
    return ((x >= 0 && y >= 0) && (x < size && y < size)) && (board[y*size+x] < 0);
}
 
int getDegree(int board[], int x, int y)
{
    int cnt = 0;
    for (int i = 0; i < COUNT; ++i)
        if (isValidNext(board, (x + movesX[i]), (y + movesY[i]))) cnt++;
 
    return cnt;
}
 
bool next(int board[], int *x, int *y)
{
    int min_ind = -1, c, min_degree = (COUNT+1), nextX, nextY;
    int currX = *x, currY = *y;

    for (int cnt = 0; *data == 0 && cnt < COUNT; ++cnt)
    {
        int i = cnt >= 4 ? cnt : perm[cnt];
        nextX = currX + movesX[i];
        nextY = currY + movesY[i];
        if ((isValidNext(board, nextX, nextY)) &&
        (c = getDegree(board, nextX, nextY)) < min_degree)
        {
            min_ind = i;
            min_degree = c;
        }
    }

    if (*data == 1 || min_ind == -1)
        return false;

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

int tmp;

void tourBoard(int idx, int flg)
{
    if (*data == 1)
        exit(0);

    for (int j = idx + 1; *data == 0 && j < 4; ++j) {
        if (fork() == 0) {
            tmp = perm[idx];
            perm[idx] = perm[j];
            perm[j] = tmp;
            tourBoard(idx + 1, 1);
        }
    }

    if (idx + 1 < 3 && *data == 0 && fork() == 0) {
        tourBoard(idx + 1, 0);
    }

    if (*data == 1 || flg == 0) {
        exit(0);
    }

    // for (int i = 0; i < 4; ++i) {
    //     printf("%d ", perm[i]);
    // }
    // printf("at idx=%d, pid=%d\n", idx, getpid());

    int board[ size * size ];
    memset(board, -1, sizeof(board));

    int x = start_x, y = start_y;
    board[y * size + x] = 1; 
 
    for (int i = 0; i < size * size - 1; ++i) {
        if (*data == 1 || next(board, &x, &y) == 0) {
            while (wait(NULL) > 0);
            exit(0);
        }
    }
    pthread_mutex_lock(&lock);
    if(*data == 0)
    {
        *data = 1;
        print(board, size);
    }
    pthread_mutex_unlock(&lock);

    exit(0);
}
 
// Driver code
int main(int argc, char *argv[])
{
	size = atoi(argv[1]);
    start_x = atoi(argv[2]);
    start_y = atoi(argv[3]);
 
    if(size % 2 == 1 && (start_x + start_y) % 2 == 1) {
        printf("No Possible Tour");
        return 0;
    }
 
    if(shmkey = ftok("/", 3) == -1)
    {
        perror("Key generation failed\n");
        exit(1);
    }
    if((shmid = shmget(shmkey, sizeof(int), 0666 | IPC_CREAT))== -1)
    {
        perror("Failed to get shmid\n");
        exit(1);
    }
    data = (int *) shmat(shmid, NULL, 0x0);
    *data = 0;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
 
    if (fork() == 0) {
        tourBoard(0, 1);
    }
 
    while(wait(NULL)>0);
    if (*data == 0) {
        printf("No Possible Tour");
    }

    pthread_mutex_destroy(&lock);
    shmctl(shmid,IPC_RMID,NULL);
    return 0;
}

/*
0 1 2 3 at idx=0, pid=92236
0 1 3 2 at idx=3, pid=92259
0 2 1 3 at idx=2, pid=92248
0 2 3 1 at idx=3, pid=92263
0 3 1 2 at idx=3, pid=92258
0 3 2 1 at idx=2, pid=92254
1 0 2 3 at idx=1, pid=92237
1 0 3 2 at idx=3, pid=92264
1 2 0 3 at idx=2, pid=92240
1 2 3 0 at idx=3, pid=92245
1 3 0 2 at idx=3, pid=92250
1 3 2 0 at idx=2, pid=92243
2 0 1 3 at idx=2, pid=92242
2 0 3 1 at idx=3, pid=92249
2 1 0 3 at idx=1, pid=92238
2 1 3 0 at idx=3, pid=92261
2 3 0 1 at idx=2, pid=92246
2 3 1 0 at idx=3, pid=92255
3 0 1 2 at idx=3, pid=92262
3 0 2 1 at idx=2, pid=92251
3 1 0 2 at idx=3, pid=92260
3 1 2 0 at idx=1, pid=92239
3 2 1 0 at idx=2, pid=92244
3 2 0 1 at idx=3, pid=92252
*/