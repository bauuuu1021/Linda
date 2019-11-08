#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

omp_lock_t simple_lock;

int main () {
	int numThread;
	std::cin >> numThread;
#if 0
 	omp_init_lock(&simple_lock);

        #pragma omp parallel num_threads(numThread+1)
        {
            int tid = omp_get_thread_num();

            // master
            if (tid == 0) {
                while (1) {
                    while (!omp_test_lock(&simple_lock));
                    printf("master\n");
                    sleep(2);
                    omp_unset_lock(&simple_lock);
                }
            } 
            // client
            else {
            while (!omp_test_lock(&simple_lock))
                ;//printf("Thread %d - failed to acquire simple_lock\n", tid);

            printf("Thread %d - acquired simple_lock\n", tid);
sleep(1);
            printf("Thread %d - released simple_lock\n", tid);
            omp_unset_lock(&simple_lock);
            }
        }

    omp_destroy_lock(&simple_lock);
#endif
    #pragma omp parallel num_threads(numThread+1)
        {
            int tid = omp_get_thread_num();

            // master
            if (tid == 0) {
                while (1) {
                    printf("master\n");
                    sleep(2);
                }
            } 
            // client
            else {
            while (1) {
                printf("Thread %d - acquired simple_lock\n", tid);
                sleep(1);
            }
            }
        }
}