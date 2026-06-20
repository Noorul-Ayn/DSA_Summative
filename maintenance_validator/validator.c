#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCEDURES 50
#define MAX_NAME_LEN 50
#define SIMILARITY_THRESHOLD 3

typedef struct Node {
    char name[MAX_NAME_LEN];
    struct Node *left;
    struct Node *right;
} Node;

Node *root = NULL;
int procedure_count = 0;

Node *create_node(const char *name) {
    Node *node = malloc(sizeof(Node));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    strncpy(node->name, name, MAX_NAME_LEN - 1);
    node->name[MAX_NAME_LEN - 1] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node *insert(Node *current, const char *name) {
    if (current == NULL) {
        return create_node(name);
    }
    int cmp = strcmp(name, current->name);
    if (cmp < 0) {
        current->left = insert(current->left, name);
    } else if (cmp > 0) {
        current->right = insert(current->right, name);
    }
    return current;
}

void load_procedures(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open %s\n", filename);
        return;
    }

    char line[MAX_NAME_LEN];
    while (fgets(line, sizeof(line), fp) != NULL && procedure_count < MAX_PROCEDURES) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) > 0) {
            root = insert(root, line);
            procedure_count++;
        }
    }

    fclose(fp);
    printf("Loaded %d procedures into the BST.\n", procedure_count);
}

Node *search_exact(Node *current, const char *name) {
    if (current == NULL) {
        return NULL;
    }
    int cmp = strcmp(name, current->name);
    if (cmp == 0) {
        return current;
    } else if (cmp < 0) {
        return search_exact(current->left, name);
    } else {
        return search_exact(current->right, name);
    }
}

int edit_distance(const char *a, const char *b) {
    int len_a = strlen(a);
    int len_b = strlen(b);

    int **dp = malloc((len_a + 1) * sizeof(int *));
    for (int i = 0; i <= len_a; i++) {
        dp[i] = malloc((len_b + 1) * sizeof(int));
    }

    for (int i = 0; i <= len_a; i++) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= len_b; j++) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= len_a; i++) {
        for (int j = 1; j <= len_b; j++) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                int delete_cost = dp[i - 1][j] + 1;
                int insert_cost = dp[i][j - 1] + 1;
                int replace_cost = dp[i - 1][j - 1] + 1;
                int min = delete_cost;
                if (insert_cost < min) min = insert_cost;
                if (replace_cost < min) min = replace_cost;
                dp[i][j] = min;
            }
        }
    }

    int result = dp[len_a][len_b];

    for (int i = 0; i <= len_a; i++) {
        free(dp[i]);
    }
    free(dp);

    return result;
}

void find_closest(Node *current, const char *input, char *best_match, int *best_distance) {
    if (current == NULL) {
        return;
    }
    int dist = edit_distance(input, current->name);
    if (dist < *best_distance) {
        *best_distance = dist;
        strncpy(best_match, current->name, MAX_NAME_LEN - 1);
        best_match[MAX_NAME_LEN - 1] = '\0';
    }
    find_closest(current->left, input, best_match, best_distance);
    find_closest(current->right, input, best_match, best_distance);
}

void log_invalid_attempt(const char *input) {
    FILE *fp = fopen("audit.log", "a");
    if (fp == NULL) {
        printf("Error: could not write to audit.log\n");
        return;
    }
    fprintf(fp, "REJECTED: %s\n", input);
    fclose(fp);
}

void verify_procedure(const char *input) {
    Node *exact = search_exact(root, input);
    if (exact != NULL) {
        printf("APPROVED: '%s' is a valid procedure. Execution authorized.\n", input);
        return;
    }

    char best_match[MAX_NAME_LEN] = "";
    int best_distance = 1000000;
    find_closest(root, input, best_match, &best_distance);

    if (best_distance <= SIMILARITY_THRESHOLD && best_distance < 1000000) {
        printf("SIMILAR ENTRY: '%s' is not approved. Did you mean '%s'?\n", input, best_match);
    } else {
        printf("UNKNOWN ENTRY: '%s' is not recognized. Attempt logged.\n", input);
        log_invalid_attempt(input);
    }
}

void free_tree(Node *current) {
    if (current == NULL) {
        return;
    }
    free_tree(current->left);
    free_tree(current->right);
    free(current);
}

int main(void) {
    load_procedures("procedures.txt");

    char input[MAX_NAME_LEN];
    printf("=== Secure Maintenance Procedure Validator ===\n");
    printf("Enter a procedure name (or QUIT to exit):\n");

    while (1) {
        printf("\n> ");
        if (scanf("%49s", input) != 1) {
            break;
        }
        if (strcmp(input, "QUIT") == 0) {
            break;
        }
        verify_procedure(input);
    }

    free_tree(root);
    printf("Memory freed. Goodbye.\n");
    return 0;
}
