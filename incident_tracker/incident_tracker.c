#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INCIDENTS 25

typedef struct Incident {
    int id;
    char source[20];
    char description[100];
    struct Incident *next;
    struct Incident *prev;
} Incident;

Incident *head = NULL;
Incident *tail = NULL;
Incident *active = NULL;
int count = 0;
int next_id = 1;
int live_mode = 0;

void add_incident(const char *source, const char *description) {
    Incident *node = malloc(sizeof(Incident));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    node->id = next_id++;
    strncpy(node->source, source, sizeof(node->source) - 1);
    node->source[sizeof(node->source) - 1] = '\0';
    strncpy(node->description, description, sizeof(node->description) - 1);
    node->description[sizeof(node->description) - 1] = '\0';
    node->next = NULL;
    node->prev = NULL;

    if (count == MAX_INCIDENTS) {
        Incident *old_head = head;
        head = head->next;
        if (head != NULL) {
            head->prev = NULL;
        }
        if (active == old_head) {
            active = head;
        }
        free(old_head);
        count--;
    }

    if (tail == NULL) {
        head = node;
        tail = node;
    } else {
        node->prev = tail;
        tail->next = node;
        tail = node;
    }
    count++;

    if (active == NULL) {
        active = head;
    }
}

void view_active(void) {
    if (active == NULL) {
        printf("No incidents recorded.\n");
        return;
    }
    printf("---------------------------------\n");
    printf("Incident ID : %d\n", active->id);
    printf("Source      : %s\n", active->source);
    printf("Description : %s\n", active->description);
    printf("---------------------------------\n");
}

void go_newer(void) {
    if (active == NULL) {
        printf("No incidents recorded.\n");
        return;
    }
    if (active->next == NULL) {
        printf("Already at the newest incident.\n");
        return;
    }
    active = active->next;
    view_active();
}

void go_older(void) {
    if (active == NULL) {
        printf("No incidents recorded.\n");
        return;
    }
    if (active->prev == NULL) {
        printf("Already at the oldest incident.\n");
        return;
    }
    active = active->prev;
    view_active();
}

void start_live(void) {
    live_mode = 1;
    printf("Live monitoring enabled.\n");
}

void stop_live(void) {
    live_mode = 0;
    printf("Live monitoring stopped.\n");
}

void delete_all(void) {
    Incident *current = head;
    while (current != NULL) {
        Incident *temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
    tail = NULL;
    active = NULL;
    count = 0;
    printf("All incidents deleted.\n");
}

void save_and_quit(void) {
    FILE *fp = fopen("session_log.txt", "w");
    if (fp == NULL) {
        printf("Error: could not save session.\n");
        return;
    }
    Incident *current = head;
    while (current != NULL) {
        fprintf(fp, "%d|%s|%s\n", current->id, current->source, current->description);
        current = current->next;
    }
    fclose(fp);
    printf("Session saved to session_log.txt.\n");

    delete_all();
}

void simulate_incoming_incident(void) {
    static const char *sources[] = {"Ambulance", "Police", "Fire"};
    int idx = rand() % 3;
    char desc[100];
    snprintf(desc, sizeof(desc), "Auto-generated incident report #%d", next_id);
    add_incident(sources[idx], desc);
    printf("[LIVE] New incident added from %s (ID %d).\n", sources[idx], next_id - 1);
}

int main(void) {
    char command;

    add_incident("Police", "Initial incident at startup");
    active = head;

    printf("=== Emergency Dispatch Incident Tracker ===\n");
    printf("Commands: f=newer, b=older, l=live on, s=live off, d=delete all, q=quit\n");

    view_active();

    while (1) {
        printf("\nEnter command: ");
        if (scanf(" %c", &command) != 1) {
            break;
        }

        switch (command) {
            case 'f':
                go_newer();
                break;
            case 'b':
                go_older();
                break;
            case 'l':
                start_live();
                break;
            case 's':
                stop_live();
                break;
            case 'd':
                delete_all();
                break;
            case 'q':
                save_and_quit();
                printf("Session ended.\n");
                return 0;
            default:
                printf("Unknown command. Use f, b, l, s, d, or q.\n");
        }

        if (live_mode == 1) {
            simulate_incoming_incident();
        }
    }

    return 0;
}
