#include <iostream>
#include <string>
#include <list>
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
	union {
		char content_str[MAX_LEN];
		int content_int;
		char var_name[MAX_LEN];
	};
} element_t;

int main () {
        char buf[MAX_LEN];
        char *p;
	list<list<element_t>> tuple_list;

        for (int for_i = 0; for_i < 3; for_i++) {
                int client;
                char act[5];
                list<element_t> tuple;
                fgets(buf, sizeof(buf), stdin);
                p = strtok(buf, " ");
                client = atoi(p);       // parse client ID
                p = strtok(NULL, " ");
                strcpy(act, p);         // parse action
                p = strtok(NULL, " ");
                do {
                        element_t ele;

                        if (p[0]=='"') {                /* string */
                                printf("string\n");
                                ele.type = STR;
                                strcpy(ele.content_str, p);
                        }
                        else if (p[0]=='?') {    /* variable */
                                printf("variable\n");
                                ele.type = VAR;
                                // insert to variable list
                        }
                        else {    /* int */
                                printf("integer\n");
                                ele.type = INT;
                                ele.content_int = atoi(p);
                        }
                        tuple.push_back(ele);

                        p = strtok(NULL, " ");
                } while (p);    // til end of input buffer parsing
                tuple_list.push_back(tuple);
        }

        for (auto out_it = tuple_list.begin(); out_it != tuple_list.end(); out_it++) {
                for (auto it = out_it->begin(); it != out_it->end(); it++) {
                        cout << "type " << it->type << ", content " << it->content_int << endl;
                }
        }


}
