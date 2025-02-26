#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

#define COUNT 8

// Input
int size;
int start_x, start_y;

// Shared memory
pthread_mutex_t lock;
key_t shmkey;
int* found,shmid;
 
// For moving Knight
int movesX[] = {1,1,2,2,-1,-1,-2,-2};
int movesY[] = {2,-2,1,-1,2,-2,1,-1};
int perm[] = {0, 1, 2, 3, 4, 5, 6, 7};
 
//  For printing
typedef struct {
    int x, y;
} pair;

/**
 * @brief Returns whether the given position is in the board and unvisited
 *
 * @param board 1d array containing current state of the board
 * @param x x position
 * @param y y position
 * @return true
 * @return false
 */
bool isValidNext(int board[], int x, int y)
{
    return ((x >= 0 && y >= 0) && (x < size && y < size)) && (board[y*size+x] < 0);
}

/**
 * @brief Returns number of possible moves from given position
 *
 * @param board 1d array containing current state of the board
 * @param x x position
 * @param y y position
 * @return int number of possible moves from (x, y)
 */
int getDegree(int board[], int x, int y)
{
    int cnt = 0;
    for (int i = 0; i < COUNT; ++i)
        if (isValidNext(board, (x + movesX[i]), (y + movesY[i]))) cnt++;
 
    return cnt;
}

/**
 * @brief Moves the Knight using Warnsdorff's Rule, if found 
 * flag is unset and a move is possible
 * 
 * @param board 1d array containing current state of the board
 * @param x Pointer to current x position of Knight
 * @param y Pointer to current y position of Knight
 * @return true Transition successful
 * @return false No possible transition/found flag already set
 */
bool next(int board[], int *x, int *y)
{
    int min_ind = -1, c, min_degree = (COUNT+1), nextX, nextY;
    int currX = *x, currY = *y;

    for (int cnt = 0; *found == 0 && cnt < COUNT; ++cnt)
    {
        int i = perm[cnt];
        nextX = currX + movesX[i];
        nextY = currY + movesY[i];
        if ((isValidNext(board, nextX, nextY)) &&
        (c = getDegree(board, nextX, nextY)) < min_degree)
        {
            min_ind = i;
            min_degree = c;
        }
    }

    if (*found == 1 || min_ind == -1)
        return false;

    nextX = currX + movesX[min_ind];
    nextY = currY + movesY[min_ind];
 
    board[nextY * size + nextX] = board[currY * size + currX] + 1;
 
    *x = nextX;
    *y = nextY;
 
    return true;
}

/**
 * @brief Prints the path of a successful Knight's Tour
 *
 * @param board 1d array containing current state of the board
 * @param n Size of the board
 */
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

// Length of permutations
int maxp;

/**
 * @brief Searches for possible Knight's Tour using Warnsdorff's Rule and 
 * permutations of fixed orderings to break ties
 * 
 * @param idx Last swapped index of perm
 * @param flg Whether current perm is yet to tour
 */
void tourBoard(int idx, int flg) {

    if (*found == 1) {
        while (wait(NULL) > 0);
        exit(0);
    }

    // Generate permutations
    for (int j = idx + 1; *found == 0 && j < maxp; ++j) {
        if (fork() == 0) {
            int tmp = perm[idx];
            perm[idx] = perm[j];
            perm[j] = tmp;
            tourBoard(idx + 1, 1);
        }
    }

    if (idx + 2 < maxp && *found == 0 && fork() == 0) {
        tourBoard(idx + 1, 0);
    }

    if (*found == 1 || flg == 0) {
        while (wait(NULL) > 0);
        exit(0);
    }

    // Initialize board, starting position of Knight
    int board[ size * size ];
    memset(board, -1, sizeof(board));

    int x = start_x, y = start_y;
    board[y * size + x] = 1; 
 
    // Make moves
    for (int i = 0; i < size * size - 1; ++i) {
        if (*found == 1 || next(board, &x, &y) == 0) {
            while (wait(NULL) > 0);
            exit(0);
        }
    }

    // Obtain mutex lock, set found flag and print the path, unlock mutex
    pthread_mutex_lock(&lock);
    if(*found == 0) {
        *found = 1;
        print(board, size);
    }
    pthread_mutex_unlock(&lock);

    while (wait(NULL) > 0);
    exit(0);
}

// Driver code
int main(int argc, char *argv[])
{
    size = atoi(argv[1]);
    start_x = atoi(argv[2]);
    start_y = atoi(argv[3]);

    // Tour cannot exist in this case
    if (size % 2 == 1 && (start_x + start_y) % 2 == 1) {
        printf("No Possible Tour");
        return 0;
    }

    // Shared memory
    if (shmkey = ftok("/", 3) == -1) {
        perror("Key generation failed\n");
        exit(1);
    }
    if ((shmid = shmget(shmkey, sizeof(int), 0666 | IPC_CREAT)) == -1) {
        perror("Failed to get shmid\n");
        exit(1);
    }

    // Initialize found flag to 0
    found = (int *)shmat(shmid, NULL, 0x0);
    *found = 0;

    // Initialize mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    // Edge case handling
    if (size == 5){
        maxp = 7;
    } else {
        maxp = 4;
    }

    // Try finding Knight's Tour with a child process
    if (fork() == 0) {
        tourBoard(0, 1);
    }

    // If found flag is still 0, then tour does not exist
    while (wait(NULL) > 0);
    if (*found == 0) {
        printf("No Possible Tour");
    }

    // Clean up
    pthread_mutex_destroy(&lock);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}