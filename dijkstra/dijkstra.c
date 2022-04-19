/*
 * 61908697 RyosukeSasaki
 */

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

#define NNODE 6
#define INF 100
int cost[NNODE][NNODE] = {
    {0,     2,      5,      1,      INF,        INF},
    {2,     0,      3,      2,      INF,        INF},
    {5,     3,      0,      3,      1,          5},
    {1,     2,      3,      0,      1,          INF},
    {INF,   INF,    1,      1,      0,          2},
    {INF,   INF,    5,      INF,    2,          0},
};
const char node_name[NNODE] = "ABCDEF";


typedef enum{
    dijkstra_SUCCEEDED = 0,
    dijkstra_VALUE_ERR = -1,
    dijkstra_NAME_NOT_ASSIGNED = -2,
} dijkstra_error_t;


dijkstra_error_t value_check()
{
    int i,j;
    for(i=0; i<NNODE; i++) {
        for(j=0; j<NNODE; j++) {
            if(cost[i][j]<0 || INF<cost[i][j]) return dijkstra_VALUE_ERR;
        }
    }
    for(i=0; i<NNODE; i++) {
        if(node_name[i] == 0) return dijkstra_NAME_NOT_ASSIGNED;
    }
    return dijkstra_SUCCEEDED;
}


bool exist(int* array, int len, int target)
{
    int i;
    for(i=0; i<len; i++) {
        if (array[i] == target) return true;
    }
    return false;
}


int iNprime=0;
int Nprime[NNODE];
int D[NNODE];
int p[NNODE];
void add_Nprime(int num)
{
    Nprime[iNprime++] = num;
    iNprime %= NNODE;
}
void clear_Nprime()
{
    int i;
    for(i=0; i<NNODE; i++) Nprime[i] = -1;
    iNprime = 0;
}


dijkstra_error_t dijkstra(int root)
{
    // Initialize
    int k;
    int i;
    clear_Nprime();
    add_Nprime(root);
    for(i=0; i<NNODE; i++) {D[i] = cost[root][i]; p[i] = root;}
    
    // Loop
    for (k=0; k<NNODE; k++)
    {
        bool fin = true;
        int min[2] = {-1, INF}; // index, value
        for(i=0; i<NNODE; i++) {
            if(exist(Nprime, NNODE, i)) continue;
            if(D[i] < min[1]) {min[0]=i; min[1]=D[i];}
            fin = false;
        }
        if(fin) break;
        add_Nprime(min[0]);
        for(i=0; i<NNODE; i++){
            if(exist(Nprime, NNODE, i)) continue;
            if(D[i] > D[min[0]] + cost[min[0]][i]) {
                D[i] = D[min[0]] + cost[min[0]][i];
                p[i] = min[0];
            }
        }
    }
    return dijkstra_SUCCEEDED;
}


void show_graph(int root)
{
    printf("root node %c:\r\n\t", node_name[root]);
    int i;
    for(i=0; i<NNODE; i++) {
        printf("[%c, %c, %d] ", node_name[i], node_name[p[i]], D[i]);
    }
    printf("\r\n");
}


int ret=0;
int main()
{
    ret = value_check();
    switch (ret) {
    case dijkstra_VALUE_ERR:
        printf("value error in given matrix\r\n");
        return ret;
        break;
    case dijkstra_NAME_NOT_ASSIGNED:
        printf("name not assigned to the node\r\n");
        return ret;
        break;
    default:
        break;
    }

    int i;
    for(i=0; i<NNODE; i++) {
        dijkstra(i);
        show_graph(i);
    }
    return dijkstra_SUCCEEDED;
}