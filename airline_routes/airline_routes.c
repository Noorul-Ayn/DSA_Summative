#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_AIRPORTS 50
#define NAME_LEN 10

char airports[MAX_AIRPORTS][NAME_LEN];
int matrix[MAX_AIRPORTS][MAX_AIRPORTS];
int airport_count = 0;

int find_airport(const char *name) {
    for (int i = 0; i < airport_count; i++) {
        if (strcmp(airports[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_airport(const char *name) {
    if (airport_count >= MAX_AIRPORTS) {
        printf("Error: maximum number of airports reached.\n");
        return;
    }
    if (find_airport(name) != -1) {
        printf("Error: airport '%s' already exists.\n", name);
        return;
    }
    strncpy(airports[airport_count], name, NAME_LEN - 1);
    airports[airport_count][NAME_LEN - 1] = '\0';
    airport_count++;
    printf("Airport '%s' added.\n", name);
}

void remove_airport(const char *name) {
    int idx = find_airport(name);
    if (idx == -1) {
        printf("Error: airport '%s' not found.\n", name);
        return;
    }

    for (int i = idx; i < airport_count - 1; i++) {
        strcpy(airports[i], airports[i + 1]);
    }

    for (int i = idx; i < airport_count - 1; i++) {
        for (int j = 0; j < airport_count; j++) {
            matrix[i][j] = matrix[i + 1][j];
        }
    }
    for (int j = idx; j < airport_count - 1; j++) {
        for (int i = 0; i < airport_count; i++) {
            matrix[i][j] = matrix[i][j + 1];
        }
    }

    airport_count--;
    printf("Airport '%s' removed.\n", name);
}

void add_route(const char *from, const char *to) {
    int from_idx = find_airport(from);
    int to_idx = find_airport(to);
    if (from_idx == -1 || to_idx == -1) {
        printf("Error: one or both airports not found.\n");
        return;
    }
    matrix[from_idx][to_idx] = 1;
    printf("Route added: %s -> %s\n", from, to);
}

void remove_route(const char *from, const char *to) {
    int from_idx = find_airport(from);
    int to_idx = find_airport(to);
    if (from_idx == -1 || to_idx == -1) {
        printf("Error: one or both airports not found.\n");
        return;
    }
    matrix[from_idx][to_idx] = 0;
    printf("Route removed: %s -> %s\n", from, to);
}

void show_outgoing(const char *name) {
    int idx = find_airport(name);
    if (idx == -1) {
        printf("Error: airport '%s' not found.\n", name);
        return;
    }
    printf("Airports reachable directly from %s:\n", name);
    int found = 0;
    for (int j = 0; j < airport_count; j++) {
        if (matrix[idx][j] == 1) {
            printf("  -> %s\n", airports[j]);
            found = 1;
        }
    }
    if (!found) {
        printf("  (none)\n");
    }
}

void show_incoming(const char *name) {
    int idx = find_airport(name);
    if (idx == -1) {
        printf("Error: airport '%s' not found.\n", name);
        return;
    }
    printf("Airports with direct flights into %s:\n", name);
    int found = 0;
    for (int i = 0; i < airport_count; i++) {
        if (matrix[i][idx] == 1) {
            printf("  -> %s\n", airports[i]);
            found = 1;
        }
    }
    if (!found) {
        printf("  (none)\n");
    }
}

void display_matrix(void) {
    printf("\nAdjacency Matrix:\n");
    printf("%-8s", "");
    for (int j = 0; j < airport_count; j++) {
        printf("%-8s", airports[j]);
    }
    printf("\n");

    for (int i = 0; i < airport_count; i++) {
        printf("%-8s", airports[i]);
        for (int j = 0; j < airport_count; j++) {
            printf("%-8d", matrix[i][j]);
        }
        printf("\n");
    }
}

void load_sample_data(void) {
    add_airport("KGL");
    add_airport("NBO");
    add_airport("EBB");
    add_airport("JNB");
    add_airport("ADD");
    add_airport("CAI");
    add_airport("CPT");

    add_route("KGL", "NBO");
    add_route("KGL", "EBB");
    add_route("NBO", "JNB");
    add_route("EBB", "ADD");
    add_route("ADD", "CAI");
    add_route("JNB", "CPT");
}

int main(void) {
    memset(matrix, 0, sizeof(matrix));
    load_sample_data();

    char command[20];
    char arg1[NAME_LEN], arg2[NAME_LEN];

    printf("\n=== Airline Route Relationship Analyzer ===\n");
    printf("Commands:\n");
    printf("  out <airport>        - show outgoing routes\n");
    printf("  in <airport>         - show incoming routes\n");
    printf("  addairport <name>    - add a new airport\n");
    printf("  removeairport <name> - remove an airport\n");
    printf("  addroute <a> <b>     - add route a -> b\n");
    printf("  removeroute <a> <b>  - remove route a -> b\n");
    printf("  matrix               - display adjacency matrix\n");
    printf("  quit                 - exit\n");

    while (1) {
        printf("\n> ");
        if (scanf("%19s", command) != 1) {
            break;
        }

        if (strcmp(command, "quit") == 0) {
            break;
        } else if (strcmp(command, "out") == 0) {
            scanf("%9s", arg1);
            show_outgoing(arg1);
        } else if (strcmp(command, "in") == 0) {
            scanf("%9s", arg1);
            show_incoming(arg1);
        } else if (strcmp(command, "addairport") == 0) {
            scanf("%9s", arg1);
            add_airport(arg1);
        } else if (strcmp(command, "removeairport") == 0) {
            scanf("%9s", arg1);
            remove_airport(arg1);
        } else if (strcmp(command, "addroute") == 0) {
            scanf("%9s %9s", arg1, arg2);
            add_route(arg1, arg2);
        } else if (strcmp(command, "removeroute") == 0) {
            scanf("%9s %9s", arg1, arg2);
            remove_route(arg1, arg2);
        } else if (strcmp(command, "matrix") == 0) {
            display_matrix();
        } else {
            printf("Unknown command.\n");
        }
    }

    printf("Goodbye.\n");
    return 0;
}
