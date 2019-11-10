#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <omp.h>
#include <map>
extern "C" {
#include <string.h>
#include <stdio.h>
}

using namespace std;
#define MAX_LEN 1024

enum TYPE {
	STR,
	INT,
	VAR
};

typedef struct element {
	int type;
        int client_id;
	union {
		char content_str[MAX_LEN];
		int content_int;
		char var_name[MAX_LEN];
	};
} element_t;

omp_lock_t simple_lock;
std::vector<int> client_vec;
bool nextRnd = true;

void master () {
    while (1) {
        int id;
        //while (!omp_test_lock(&simple_lock));
        printf("master request\n");
        scanf("%d", &id);
        client_vec.at(id) = 1;
        //omp_unset_lock(&simple_lock);
    }
}

void client (int thread_id) {
    while (nextRnd) {
        while (!client_vec[thread_id]) ;
        printf("Thread %d - acquired simple_lock\n", thread_id);
        client_vec[thread_id] = 0; 
    }
}

int main () {
	int numThread;
	std::cin >> numThread;
    client_vec.resize(numThread + 1, 0);

    omp_init_lock(&simple_lock);

    #pragma omp parallel num_threads(numThread+1)
    {
        int tid = omp_get_thread_num();

        // master
        if (tid == 0) {
            master();
        } 
        // client
        else {
            client(tid);
        }
    }
    
    omp_destroy_lock(&simple_lock);
}

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