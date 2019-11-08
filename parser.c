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
        char *p;
	list<list<element_t>> tuple_list;

        while (true) {
                int client;
                char act[5], buf[MAX_LEN];
                list<element_t> tuple;

                fgets(buf, sizeof(buf), stdin);
                if (!strncmp(buf, "exit", 4))    break;

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
                        else if (p[0]=='?') {    /* undefined variable */
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

        // print tuple list
        for (auto out_it = tuple_list.begin(); out_it != tuple_list.end(); out_it++) {
                for (auto it = out_it->begin(); it != out_it->end(); it++) {
                        cout << "type " << it->type << ", content " << it->content_int << endl;
                }
        }


}
