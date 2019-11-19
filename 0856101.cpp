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
	VAR,
	UN_VAR  // variable that hasn't defined yet
};

enum STATUS {
	NORMAL,
	SUS_IN,
	SUS_READ
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
vector<int> client_vec, suspend_id;
bool nextRnd = true;
string send2client;
map<string, element_t> variable_pair;

void output_tuple(list<list<element_t>> list) {
	FILE *fp = fopen("server.txt", "a");
	bool first_tuple = true;

	fprintf(fp, "(");
	for (auto out_it = list.begin(); out_it != list.end(); out_it++) {
		if (first_tuple)
			fprintf(fp, "(");
		else
			fprintf(fp, ",(");
		first_tuple = false;

		bool first_element = true;
		for (auto it = out_it->begin(); it != out_it->end(); it++) {
			if (!first_element) 
				fprintf(fp, ",");
			first_element = false;

			if (it->type == INT)
				fprintf(fp, "%d", it->content_int);
			else if (it->type == VAR)
				fprintf(fp, "%s", it->var_name);
			else if (it->type == STR)
				fprintf(fp, "%s", it->content_str);
			else if (it->type == UN_VAR)
				fprintf(fp, "%s", it->var_name);
		}
		fprintf(fp, ")");
	}
	fprintf(fp, ")\n");

	fclose(fp);
}

void master () {
	char *p;
	list<list<element_t>> tuple_list, wait_list;

	while (true) {
		while (!client_vec[0]);
		omp_set_lock(&simple_lock);
		int client;
		char act[5], buf[MAX_LEN];
		list<element_t> tuple;

		fgets(buf, sizeof(buf), stdin);
		strtok(buf, "\n");
		if (!strncmp(buf, "exit", 4)) {
			nextRnd = false;
			omp_unset_lock(&simple_lock);
			break;
		}    

		p = strtok(buf, " ");
		client = atoi(p);       // parse client ID
		if (suspend_id.at(client)) {
			omp_unset_lock(&simple_lock);
			continue;
		}
		p = strtok(NULL, " ");
		strcpy(act, p);         // parse action
		p = strtok(NULL, " ");
		do {
			element_t ele;
			ele.client_id = client;

			if (p[0]=='"') {                /* string */
				ele.type = STR;
				strcpy(ele.content_str, p);
			}
			else if (p[0]=='?') {    /* undefined variable */
				ele.type = UN_VAR;
				strcpy(ele.var_name, &p[1]);
				element_t tmp;
				memset(&tmp, 0, sizeof(element_t));
				tmp.type = UN_VAR;
				variable_pair.insert(pair<string, element_t>(string(ele.var_name), tmp));
			}
			else if (!isdigit(p[0])) {      /* defined variable */
				ele = (variable_pair.find(p))->second;
			}
			else {    /* int */
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
			list<list<element_t>>::iterator ret;    // record the instance that may need to pop out
			for (auto out_it = wait_list.begin(); (match == false)&&(out_it != wait_list.end()); out_it++) {
				auto tuple_it = tuple.begin();
				ret = out_it;
				for (auto it = out_it->begin(); it != out_it->end(); it++) {
					match = true;
					ret2client_id = it->client_id;

					// check length
					if (tuple.size()!=out_it->size()) {
						match = false;
						send2client.clear();
						break;
					}

					// compare elements one by one
					if ((it->type == tuple_it->type)&&(it->type==INT)) {
						if (it->content_int == tuple_it->content_int) {
							send2client += to_string(tuple_it->content_int);
							send2client += ",";
							tuple_it++;
						}
						else {
							match = false;
							send2client.clear();
							break;
						}
					}
					else if ((it->type == tuple_it->type)&&(it->type==STR)) {
						if (!strcmp(it->content_str, tuple_it->content_str)) {
							send2client += string(tuple_it->content_str);
							send2client += ",";
							tuple_it++;
						}
						else {
							match = false;
							send2client.clear();
							break;
						}
					}
					else if (it->type == VAR) {
						auto var_it = variable_pair.find(tuple_it->var_name);

						if ((it->type == (var_it->second).type)&&((var_it->second).type==INT)) {
							send2client += to_string((var_it->second).content_int);
							send2client += ",";
							tuple_it++;
						}
						else if ((it->type == (var_it->second).type)&&((var_it->second).type==STR)) {
							send2client += string((var_it->second).content_str);
							send2client += ",";
							tuple_it++;
						}
						else {
							match = false;
							send2client.clear();
							break;
						}                        
					}
					else if (it->type == UN_VAR) {
						auto var_it = variable_pair.find(it->var_name);
						(var_it->second) = *tuple_it;
						it->type = VAR;

						if ((var_it->second).type == INT) {
							send2client += to_string((var_it->second).content_int);
							send2client += ",";
						}
						else {
							send2client += string((var_it->second).content_str);
							send2client += ",";
						}
						tuple_it++;
					}
					else {
						match = false;
						send2client.clear();
						break;
					}
				}
			}
			if (match == false) {     // no previous request match
				tuple_list.push_back(tuple);
			}
			else {	// match -> rescue from suspend
				if (suspend_id.at(ret2client_id) == SUS_READ)
					tuple_list.push_back(tuple);
				client_vec.at(ret2client_id) = 1;
				suspend_id.at(ret2client_id) = NORMAL;
				tuple_list.erase(ret);
				client_vec.at(0) = 0;
			}
			output_tuple(tuple_list);
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
						send2client.clear();
						break;
					}

					// compare elements one by one
					if ((it->type == tuple_it->type)&&(it->type==INT)) {
						if (it->content_int == tuple_it->content_int) {
							send2client += to_string(tuple_it->content_int);
							send2client += ",";
							tuple_it++;
						}
						else {
							match = false;
							send2client.clear();
							break;
						}
					}
					else if ((it->type == tuple_it->type)&&(it->type==STR)) {
						if (!strcmp(it->content_str, tuple_it->content_str)) {
							send2client += string(tuple_it->content_str);
							send2client += ",";
							tuple_it++;
						}
						else {
							match = false;
							send2client.clear();
							break;
						}
					}
					else if (tuple_it->type == VAR) {
						auto var_it = variable_pair.find(tuple_it->var_name);
						if ((var_it->second).type == it->type) { 
							if ((it->type == (var_it->second).type)&&((var_it->second).type==INT)) {
								if (it->content_int == (var_it->second).content_int) {
									send2client += to_string((var_it->second).content_int);
									send2client += ",";
									tuple_it++;
								}
								else {
									match = false;
									send2client.clear();
									break;
								}
							}
							else if ((it->type == (var_it->second).type)&&((var_it->second).type==STR)) {
								if (!strcmp(it->content_str, (var_it->second).content_str)) {
									send2client += string((var_it->second).content_str);
									send2client += ",";
									tuple_it++;
								}
								else {
									match = false;
									send2client.clear();
									break;
								}
							}
						}
						else {  
							match = false;
							send2client.clear();
							break;
						}
					}
					else if (tuple_it->type == UN_VAR) {
						auto var_it = variable_pair.find(tuple_it->var_name);
						var_it->second = *it;

						if ((var_it->second).type==INT) {
							send2client += to_string((var_it->second).content_int);
							send2client += ",";
						}
						else if ((var_it->second).type==STR) {
							send2client += string((var_it->second).content_str);
							send2client += ",";
						}

						tuple_it->type = VAR;
						tuple_it++;
					}
					else {
						match = false;
						send2client.clear();
						break;
					}
				}
			}

			if (match == false) {     // no previous request match -> suspend
				wait_list.push_back(tuple);
				if (!strncmp(act, "in", 2))
					suspend_id.at(client) = SUS_IN;
				else
					suspend_id.at(client) = SUS_READ;
			}
			else {  // match 
				if (!strcmp(act, "in")) {
					tuple_list.erase(ret);
					output_tuple(tuple_list);
				}
				client_vec.at(client) = 1;
				client_vec.at(0) = 0;
			}
		}

		omp_unset_lock(&simple_lock);
	}
}

void client (int thread_id) {
	string file_name = to_string(thread_id);
	file_name += ".txt";
	FILE *fp = fopen(file_name.c_str(), "a");

	while (nextRnd || client_vec.at(thread_id)) {
		if (client_vec[thread_id]) {
			omp_set_lock(&simple_lock);

			send2client = send2client.substr(0, send2client.size()-1); 
			fprintf(fp, "(%s)\n", send2client.c_str());
			client_vec.at(thread_id) = 0; 
			client_vec.at(0) = 1;
			
			omp_unset_lock(&simple_lock);
		}
	}

	fclose(fp);
}

int main () {
	int numThread;
	cin >> numThread;
	getchar();
	client_vec.resize(numThread + 1, 0);
	suspend_id.resize(numThread + 1, NORMAL);

	omp_init_lock(&simple_lock);

#pragma omp parallel num_threads(numThread+1)
	{
		int tid = omp_get_thread_num();

		// master
		if (tid == 0) {
			client_vec.at(0) = 1;
			master();
		} 
		// client
		else {
			client(tid);
		}
	}

	omp_destroy_lock(&simple_lock);
}
