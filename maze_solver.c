#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#define BYTE_SIZE = 8
#define MAZE_SIZE 4//the size is 4*32 due to the use of bits
static int maze[MAZE_SIZE];
typedef struct
{
    uint16_t index_y;
    uint16_t index_X;
}possion
void main()
{
    printf("hello");
}
void crate_maze()
{
    srand(time(NULL));
    int i=0;
    for (i<MAZE_SIZE; i++;)
    {
        int random = rand();
        maze[i] = random;
    }//random maze
}
void crate_path()
{
    int i=0;
    int j=0;
    for(i<MAZE_SIZE;i++)
    {
        for(j<MAZE_SIZE;j++)
        {
            
        }
    }
}

int distance(int maze_size,possion index)
{
    return maze_size*sizeof(int)*BYTE_SIZE-index.index_x-max(maze_size*sizeof(int)*BYTE_SIZE>>1-index.index_y);
}
int path_find(int maze[],possion index)
{
    possion forward;
    forward.index_x = index.index_x+1;
    forward.index_y = index.index_y;

    possion backward;
    backward.index_x = index.index_x-1;
    backward.index_y = index.index_y;

    possion up;
    up.index_x = index.index_x;
    up.index_y = index.index_y+1;

    possion down;
    down.index_x = index.index_x;
    down.index_y = index.index_y-1;

    int dist_forward,dist_backward,dist_up,dist_down;
    dist_forward = distance(maze,forward);
    dist_backward = distance(maze,backward);
    dist_up = distance(maze,up);
    dist_down = distance(maze,down);
    int max_dist= max(max(max(dist_forward,dist_backward),dist_up),dist_down);
    if(dist_forward>=max_dist)
    {
        path_find(maze,forward);    
    }

}