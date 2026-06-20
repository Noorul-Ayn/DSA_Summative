#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOCATIONS 20
#define NAME_LEN 30
#define INF 999999

char locations[MAX_LOCATIONS][NAME_LEN];
int matrix[MAX_LOCATIONS][MAX_LOCATIONS];
int location_count = 0;

int find_location(const char *name) {
    for (int i = 0; i < location_count; i++) {
        if (strcmp(locations[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_location(const char *name) {
    if (find_location(name) == -1) {
        strncpy(locations[location_count], name, NAME_LEN - 1);
        locations[location_count][NAME_LEN - 1] = '\0';
        location_count++;
    }
}

void add_path(const char *a, const char *b, int weight) {
    add_location(a);
    add_location(b);
    int i = find_location(a);
    int j = find_location(b);
    matrix[i][j] = weight;
    matrix[j][i] = weight;
}

void load_network(void) {
    add_path("Library", "Cafeteria", 6);
    add_path("Library", "Engineering", 15);
    add_path("Cafeteria", "ScienceBlock", 4);
    add_path("ScienceBlock", "Dormitory", 8);
    add_path("Engineering", "Administration", 5);
    add_path("Administration", "Dormitory", 3);
    add_path("Cafeteria", "ChargingStation", 2);
    add_path("ChargingStation", "Administration", 4);
}

void dijkstra(int start, int end) {
    int dist[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];
    int previous[MAX_LOCATIONS];

    for (int i = 0; i < location_count; i++) {
        dist[i] = INF;
        visited[i] = 0;
        previous[i] = -1;
    }
    dist[start] = 0;

    for (int count = 0; count < location_count - 1; count++) {
        int min_dist = INF;
        int u = -1;

        for (int i = 0; i < location_count; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        if (u == -1) {
            break;
        }
        visited[u] = 1;

        for (int v = 0; v < location_count; v++) {
            if (matrix[u][v] != 0 && !visited[v]) {
                int new_dist = dist[u] + matrix[u][v];
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    previous[v] = u;
                }
            }
        }
    }

    if (dist[end] == INF) {
        printf("No path exists from %s to %s.\n", locations[start], locations[end]);
        return;
    }

    int path[MAX_LOCATIONS];
    int path_len = 0;
    int current = end;
    while (current != -1) {
        path[path_len++] = current;
        current = previous[current];
    }

    printf("Path: ");
    for (int i = path_len - 1; i >= 0; i--) {
        printf("%s", locations[path[i]]);
        if (i > 0) {
            printf(" -> ");
        }
    }
    printf("\n");
    printf("Total travel distance: %d\n", dist[end]);
}

int main(void) {
    memset(matrix, 0, sizeof(matrix));
    load_network();

    int dormitory_idx = find_location("Dormitory");

    char input[NAME_LEN];
    printf("=== Campus Delivery Robot Navigation ===\n");
    printf("Locations available:\n");
    for (int i = 0; i < location_count; i++) {
        printf("  - %s\n", locations[i]);
    }
    printf("\nEnter starting building (or QUIT to exit): ");

    while (1) {
        if (scanf("%29s", input) != 1) {
            break;
        }
        if (strcmp(input, "QUIT") == 0) {
            break;
        }

        int start_idx = find_location(input);
        if (start_idx == -1) {
            printf("Error: '%s' is not a valid building name. Please try again.\n", input);
        } else if (start_idx == dormitory_idx) {
            printf("You are already at Dormitory. Distance: 0\n");
        } else {
            dijkstra(start_idx, dormitory_idx);
        }

        printf("\nEnter starting building (or QUIT to exit): ");
    }

    printf("Goodbye.\n");
    return 0;
}
