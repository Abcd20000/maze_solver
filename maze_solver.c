#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/resource.h>

#define BYTE_SIZE       8
#define MAZE_SIZE       50
#define BITS_PER_WORD   (BYTE_SIZE * (int)sizeof(int))
#define GRID_W          (MAZE_SIZE * BYTE_SIZE * (int)sizeof(int))
#define GRID_H          GRID_W
#define MAZE_WORDS      ((GRID_W * GRID_H + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define END_INDEX_X     (GRID_W - 1)
#define END_INDEX_Y     (GRID_H - 1)
#define MAX(a,b)        (((a)>(b))?(a):(b))
#define LW              (GRID_W / 2)
#define LH              (GRID_H / 2)

static int     maze[MAZE_WORDS];
static uint8_t visited[GRID_W][GRID_H];
static uint8_t gen_visited[LW][LH];

typedef struct { int index_x; int index_y; } position;

static void cell_set_wall(int x, int y, int wall)
{
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    int b = y * GRID_W + x;
    if (wall) maze[b/BITS_PER_WORD] |=  (1 << (b%BITS_PER_WORD));
    else      maze[b/BITS_PER_WORD] &= ~(1 << (b%BITS_PER_WORD));
}

static int cell_is_wall(int x, int y)
{
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return 1;
    int b = y * GRID_W + x;
    return (maze[b/BITS_PER_WORD] >> (b%BITS_PER_WORD)) & 1;
}

static const int DX[4] = { 1,-1, 0, 0 };
static const int DY[4] = { 0, 0, 1,-1 };

static void shuffle(int *a, int n)
{
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1), t = a[i]; a[i]=a[j]; a[j]=t;
    }
}

static void carve(int lx, int ly)
{
    gen_visited[lx][ly] = 1;
    cell_set_wall(lx*2, ly*2, 0);

    int dirs[4] = {0,1,2,3};
    shuffle(dirs, 4);

    for (int d = 0; d < 4; d++) {
        int nx = lx + DX[dirs[d]];
        int ny = ly + DY[dirs[d]];
        if (nx<0||nx>=LW||ny<0||ny>=LH) continue;
        if (gen_visited[nx][ny]) continue;
        cell_set_wall(lx*2 + DX[dirs[d]], ly*2 + DY[dirs[d]], 0);
        cell_set_wall(nx*2, ny*2, 0);
        carve(nx, ny);
    }
}

void create_maze(void)
{
    srand((unsigned)time(NULL));
    memset(maze,        0xFF, sizeof(maze));
    memset(gen_visited, 0,    sizeof(gen_visited));

    carve(0, 0);

    cell_set_wall(END_INDEX_X - 1, END_INDEX_Y,     0);
    cell_set_wall(END_INDEX_X,     END_INDEX_Y,     0);
    cell_set_wall(END_INDEX_X,     END_INDEX_Y - 1, 0);
    cell_set_wall(END_INDEX_X - 1, END_INDEX_Y - 1, 0);
}

void print_maze(void)
{
    printf("Maze (%d x %d):\n", GRID_W, GRID_H);
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++)
            putchar(cell_is_wall(x,y) ? '#' : '.');
        putchar('\n');
    }
}

static int heuristic(position p)
{
    return abs(END_INDEX_X - p.index_x) + abs(END_INDEX_Y - p.index_y);
}

int path_find(position cur, int g)
{
    if (cur.index_x<0||cur.index_x>=GRID_W||cur.index_y<0||cur.index_y>=GRID_H)
        return -1;
    if (cell_is_wall(cur.index_x, cur.index_y)) return -1;
    if (visited[cur.index_x][cur.index_y])      return -1;

    if (cur.index_x==END_INDEX_X && cur.index_y==END_INDEX_Y)
        return g;  // return actual distance instead of 1

    visited[cur.index_x][cur.index_y] = 1;

    position nb[4] = 
    {
        {cur.index_x+1, cur.index_y  },
        {cur.index_x-1, cur.index_y  },
        {cur.index_x,   cur.index_y+1},
        {cur.index_x,   cur.index_y-1}
    };
    int f[4];
    for (int i=0;i<4;i++)
        f[i] = (g+1) + heuristic(nb[i]);

    for (int i=1;i<4;i++) {
        position tp=nb[i]; 
        int tf=f[i],j=i-1;
        while(j>=0&&f[j]>tf)
        {
            nb[j+1]=nb[j];
            f[j+1]=f[j];
            j--;
        }
        nb[j+1]=tp;
        f[j+1]=tf;
    }

    for (int i=0;i<4;i++) {
        int res = path_find(nb[i], g+1);
        if (res != -1) {
            visited[cur.index_x][cur.index_y] = 0;
            return res;  // bubble up the distance
        }
    }

    visited[cur.index_x][cur.index_y] = 0;
    return -1;  // no path
}
int bfs(position start)
{
    if (cell_is_wall(start.index_x, start.index_y)) return 0;

    // Queue implemented as a circular buffer
    int capacity = GRID_W * GRID_H;
    position *queue = malloc(capacity * sizeof(position));
    int *dist      = malloc(capacity * sizeof(int));
    if (!queue || !dist) { free(queue); free(dist); return -1; }

    int head = 0, tail = 0;

    // Enqueue start
    queue[tail] = start;
    dist[tail]  = 0;
    tail++;
    visited[start.index_x][start.index_y] = 1;

    int result = -1; // -1 = no path found

    while (head < tail) {
        position cur   = queue[head];
        int      cur_d = dist[head];
        head++;

        if (cur.index_x == END_INDEX_X && cur.index_y == END_INDEX_Y) {
            result = cur_d;
            break;
        }

        position nb[4] = {
            {cur.index_x+1, cur.index_y  },
            {cur.index_x-1, cur.index_y  },
            {cur.index_x,   cur.index_y+1},
            {cur.index_x,   cur.index_y-1}
        };

        for (int i = 0; i < 4; i++) {
            int nx = nb[i].index_x, ny = nb[i].index_y;
            if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) 
                continue;
            if (cell_is_wall(nx, ny))   
                continue;
            if (visited[nx][ny])        
                continue;
            visited[nx][ny]  = 1;
            queue[tail]      = nb[i];
            dist[tail]       = cur_d + 1;
            tail++;
        }
    }

    free(queue);
    free(dist);
    return result;
}
void restart_visited()
{
    int i,j;
    for(i=0;i<GRID_H;i++)
    {
        for (j=0;j<GRID_W;j++)
        {
            visited[i][j]=0;
        }
    }
}
int main(void)
{
    long long counter = 0;
    struct rlimit rl;
    memset(visited, 0, sizeof(visited));
    rl.rlim_cur = 64 * 1024 * 1024;  /* 64 MB soft limit */
    rl.rlim_max = RLIM_INFINITY;      /* no hard cap      */
    int count=0;
    setrlimit(RLIMIT_STACK, &rl);
    while(1)
    {
        count++;
        create_maze();
        position start = {0, 0};
        clock_t start_time = clock();
        //printf("Searching from (0,0) to (%d,%d)...\n", END_INDEX_X, END_INDEX_Y);
        int best_move = bfs(start);
        //printf("the best solution %d \n",best_move);
        //print_maze();
        restart_visited();
        clock_t end_time = clock();
        double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        //printf("it took %f seconds \n",time_taken);
        start.index_x =0;
        start.index_y =0;
        //printf("start index_x %d \n",start.index_x);
        //printf("start index_y %d \n",start.index_y);
        start_time = clock();
        int best_move2 = path_find(start,0);
        /*if (best_move2!=-1) 
            printf("Path found!\n");
        else
            printf("No path found.\n");
        */
        end_time = clock();
        time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        //printf("it took %f seconds ",time_taken);
        if (best_move2!=best_move)
        {
            printf("best move = %d best_move2 = %d \n",best_move,best_move2);
            printf("it tool %d",count);
            break;
        }
        restart_visited();
    }
    return 0;
}