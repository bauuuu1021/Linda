#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <omp.h>
#include <map>
extern "C" {
#include <string.h>
#include <unistd.h>
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
vector<int> client_vec;
bool nextRnd = true;
string send2client;

void output_tuple(list<list<element_t>> list) {
	FILE *fp = fopen("server.txt", "a");

    fprintf(fp, "(");
	for (auto out_it = list.begin(); out_it != list.end(); out_it++) {
        fprintf(fp, "(");
		for (auto it = out_it->begin(); it != out_it->end(); it++) {
			if (it->type == INT)
				fprintf(fp, " %d ", it->content_int);
			else if (it->type == VAR)
				fprintf(fp, " %s ", it->var_name);
			else 
				fprintf(fp, " %s ", it->content_str);
		}
		fprintf(fp, ") ");
	}
    fprintf(fp, ")\n");

    fclose(fp);
}

void master () {
	//while (!omp_test_lock(&simple_lock));
	char *p;
	list<list<element_t>> tuple_list, wait_list;

	while (true) {
		sleep(1);
		int client;
		char act[5], buf[MAX_LEN];
		list<element_t> tuple;

		fgets(buf, sizeof(buf), stdin);
		strtok(buf, "\n");
		if (!strncmp(buf, "exit", 4)) {
			nextRnd = false;
			send2client.clear();
			fill(client_vec.begin(), client_vec.end(), 1);
			break;
		}    

		p = strtok(buf, " ");
		client = atoi(p);       // parse client ID
		p = strtok(NULL, " ");
		strcpy(act, p);         // parse action
		p = strtok(NULL, " ");
		do {
			element_t ele;
			ele.client_id = client;
			printf("input with id %d\n", ele.client_id);

			if (p[0]=='"') {                /* string */
				printf("string\n");
				ele.type = STR;
				strcpy(ele.content_str, p);
			}
			else if (p[0]=='?') {    /* undefined variable */
				printf("undefined variable\n");
				ele.type = VAR;
				strcpy(ele.var_name, &p[1]);
				// insert to variable list
			}
			else if (!isdigit(p[0])) {      /* defined variable */
				printf("defined variable %s\n", p);
			}
			else {    /* int */
				printf("integer\n");
				ele.type = INT;
				ele.content_int = atoi(p);
			}
			tuple.push_back(ele);

			p = strtok(NULL, " ");
		} while (p);    // til end of input buffer parsing

		// responsing logic
		send2client.clear();
		if (!strcmp(act, "out")) {      // out
			bool match = false;
			int ret2client_id;
			for (auto out_it = wait_list.begin(); (match == false)&&(out_it != wait_list.end()); out_it++) {
				auto tuple_it = tuple.begin();
				for (auto it = out_it->begin(); it != out_it->end(); it++) {
					match = true;
					ret2client_id = it->client_id;

					// check length
					if (tuple.size()!=out_it->size()) {
						match = false;
						break;
					}

					// compare elements one by one
					if ((it->type == tuple_it->type)&&(it->type==INT)) {
						if (it->content_int == tuple_it->content_int) {
							send2client += to_string(tuple_it->content_int);
							send2client += " ";
							tuple_it++;
						}
						else {
							match = false;
							break;
						}
					}
					else if ((it->type == tuple_it->type)&&(it->type==STR)) {
						if (!strcmp(it->content_str, tuple_it->content_str)) {
							send2client += string(tuple_it->content_str);
							send2client += " ";
							tuple_it++;
						}
						else {
							match = false;
							break;
						}
					}
					else {
						// TODO variable
					}
				}
			}
			cout << "send2client:" << send2client <<  "match " << match << "client " << ret2client_id << endl;
			if (match == false) {     // no previous request match
				tuple_list.push_back(tuple);
				output_tuple(tuple_list);
			}
			else 
				client_vec.at(ret2client_id) = 1;
		}
		else {  // read or in
			bool match = false;
			list<list<element_t>>::iterator ret;    // record the instance that may need to pop out
			for (auto out_it = tuple_list.begin(); (match == false)&&(out_it != tuple_list.end()); out_it++) {
				auto tuple_it = tuple.begin();
				ret = out_it;
				for (auto it = out_it->begin(); it != out_it->end(); it++) {
					match = true;

					// check length
					if (tuple.size()!=out_it->size()) {
						match = false;
						break;
					}

					// compare elements one by one
					if ((it->type == tuple_it->type)&&(it->type==INT)) {
						if (it->content_int == tuple_it->content_int) {
							send2client += to_string(tuple_it->content_int);
							send2client += " ";
							tuple_it++;
						}
						else {
							match = false;
							break;
						}
					}
					else if ((it->type == tuple_it->type)&&(it->type==STR)) {
						if (!strcmp(it->content_str, tuple_it->content_str)) {
							send2client += string(tuple_it->content_str);
							send2client += " ";
							tuple_it++;
						}
						else {
							match = false;
							break;
						}
					}
					else {
						// TODO variable
					}
				}
			}
			cout << "send2client:" << send2client <<  "match " << match << endl;
			if (match == false)      // no previous request match
				wait_list.push_back(tuple);
			else {  // match 
				if (!strcmp(act, "in")) {
					tuple_list.erase(ret);
					output_tuple(tuple_list);
				}
				client_vec.at(client) = 1;
			}
		}

		// print tuple list
		cout << "----------------------------\n";
		for (auto out_it = tuple_list.begin(); out_it != tuple_list.end(); out_it++) {
			for (auto it = out_it->begin(); it != out_it->end(); it++) {
				if (it->type == INT)
					cout << " " << it->content_int;
				else if (it->type == VAR)
					cout << " " << it->var_name;
				else 
					cout << " " << it->content_str;
			}
			cout << "\n";
		}
		cout << "----------------------------\n";
		// print waiting list
		cout << "****************************\n";
		for (auto out_it = wait_list.begin(); out_it != wait_list.end(); out_it++) {
			cout << "ask by client " << (out_it->begin())->client_id << endl;
			for (auto it = out_it->begin(); it != out_it->end(); it++) {
				if (it->type == INT)
					cout << " " << it->content_int;
				else if (it->type == VAR)
					cout << " " << it->var_name;
				else 
					cout << " " << it->content_str;
			}
			cout << "\n";
		}
		cout << "****************************\n";
	}
	//omp_unset_lock(&simple_lock);

}

void client (int thread_id) {
	while (nextRnd) {
		while (!client_vec[thread_id]);
		string file_name = to_string(thread_id);
		file_name += ".txt";
		FILE *fp = fopen(file_name.c_str(), "a");
		printf("Thread %d - acquired simple_lock\n", thread_id);
		fprintf(fp, "(%s)\n", send2client.c_str());
		fclose(fp);
		client_vec[thread_id] = 0; 
	}
}

int main () {
	int numThread;
	cin >> numThread;
	getchar();
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
