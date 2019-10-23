#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>
#include <unistd.h>

void func (int thread_id) {
	printf("[%d]in function\n", thread_id);
	int it = thread_id;
	while (1) {
		it++;
		sleep(1);
		if (it>5) {
			printf("[%d] %d\n", thread_id, 10-thread_id);
			break;
		}
	}
}

int main () {
#if 0
	char c[128], sd[128];
	char *p;
	scanf("%[^\n]", c);

	p = strtok(c, " ");
	sprintf(sd, "%s", p);
	p = strtok(NULL, " ");
	while (p) {
		sprintf(sd, "%s,%s", sd, p);
		p = strtok(NULL, " ");
	}
	printf("%s\n", sd);
#endif

	int i = 0;
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			while (1) {
				printf("hi %d\n", i);
				i++;
				sleep(1);
			}
		}
		#pragma omp section  
		{
			int j;			
			#pragma omp parallel for
				for (j=0; j<10; j = j+2) {
					func(j);
				}
		}
	}
}

