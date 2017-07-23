#include <stdio.h>
#include "defs_and_types.h"
int main() {
    float a[50][50] = {
        {0, 0, 0, 0},
        {0, 1, 2, 3},
        {0, 2, -1, 1},
        {0, 3, 0, -1}
    };
    float b[50] = {0, 9, 8, 3};
    float x[50], w[50], v[50][50];
    int i,j;
    for ( i = 0; i < 4; ++i)
    {
        x[i]=1;w[i]=1;
        for ( j = 0; j < 4; ++j)
        {
            v[i][j]=1;
        }
    }

    dsvd(a, 4, 4, w, v);
    fprintf(stderr, "oldu\n");
    perror("sorun yok");

    solveWithSvd(a, w, v, 4, 4, b, x);

    for (i = 0; i < 4; i++) {
        printf("%f\n", x[i]);
    }

    return 0;

}