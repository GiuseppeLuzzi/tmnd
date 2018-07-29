#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	srand(time(NULL));
	int r;
	int dist[8];
	for (int i = 0; i < 8; i++) 
		dist[i] = 0;

	for (int i = 0; i < 1000; i++) {
		r = (rand() % 256) ;
		//printf("%d\n", i >> 5);
		dist[r >> 5]++;
	}
	for (int i = 0; i < 8; i++) 
		printf("%d: %d\n", i, dist[i]);
	return 0;
}