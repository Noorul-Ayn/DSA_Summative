#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TREE_NODES 512

typedef struct Node {
    unsigned char character;
    int frequency;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *nodes[MAX_TREE_NODES];
    int size;
} MinHeap;

Node *create_node(unsigned char character, int frequency, Node *left, Node *right) {
    Node *node = malloc(sizeof(Node));
    node->character = character;
    node->frequency = frequency;
    node->left = left;
    node->right = right;
    return node;
}

void heap_swap(Node **a, Node **b) {
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(MinHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->nodes[index]->frequency < heap->nodes[parent]->frequency) {
            heap_swap(&heap->nodes[index], &heap->nodes[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

void heapify_down(MinHeap *heap, int index) {
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < heap->size && heap->nodes[left]->frequency < heap->nodes[smallest]->frequency) {
            smallest = left;
        }
        if (right < heap->size && heap->nodes[right]->frequency < heap->nodes[smallest]->frequency) {
            smallest = right;
        }
        if (smallest == index) {
            break;
        }
        heap_swap(&heap->nodes[index], &heap->nodes[smallest]);
        index = smallest;
    }
}

void heap_insert(MinHeap *heap, Node *node) {
    heap->nodes[heap->size] = node;
    heapify_up(heap, heap->size);
    heap->size++;
}

Node *heap_extract_min(MinHeap *heap) {
    Node *min = heap->nodes[0];
    heap->size--;
    heap->nodes[0] = heap->nodes[heap->size];
    heapify_down(heap, 0);
    return min;
}

Node *build_huffman_tree(int frequencies[256]) {
    MinHeap heap;
    heap.size = 0;

    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            heap_insert(&heap, create_node((unsigned char)i, frequencies[i], NULL, NULL));
        }
    }

    if (heap.size == 1) {
        Node *only = heap_extract_min(&heap);
        return create_node('\0', only->frequency, only, NULL);
    }

    while (heap.size > 1) {
        Node *left = heap_extract_min(&heap);
        Node *right = heap_extract_min(&heap);
        Node *merged = create_node('\0', left->frequency + right->frequency, left, right);
        heap_insert(&heap, merged);
    }

    return heap_extract_min(&heap);
}

void generate_codes(Node *root, char *code, int depth, char codes[256][256]) {
    if (root == NULL) {
        return;
    }
    if (root->left == NULL && root->right == NULL) {
        code[depth] = '\0';
        if (depth == 0) {
            strcpy(codes[root->character], "0");
        } else {
            strcpy(codes[root->character], code);
        }
        return;
    }
    code[depth] = '0';
    generate_codes(root->left, code, depth + 1, codes);
    code[depth] = '1';
    generate_codes(root->right, code, depth + 1, codes);
}

void free_tree(Node *root) {
    if (root == NULL) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

long get_file_size(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

void compress(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "rb");
    if (in == NULL) {
        printf("Error: could not open %s\n", input_filename);
        return;
    }

    int frequencies[256] = {0};
    int ch;
    long total_chars = 0;
    while ((ch = fgetc(in)) != EOF) {
        frequencies[(unsigned char)ch]++;
        total_chars++;
    }

    if (total_chars == 0) {
        printf("Error: input file is empty.\n");
        fclose(in);
        return;
    }

    Node *root = build_huffman_tree(frequencies);

    char codes[256][256];
    for (int i = 0; i < 256; i++) {
        codes[i][0] = '\0';
    }
    char temp_code[256];
    generate_codes(root, temp_code, 0, codes);

    FILE *out = fopen(output_filename, "wb");
    if (out == NULL) {
        printf("Error: could not create %s\n", output_filename);
        fclose(in);
        free_tree(root);
        return;
    }

    int distinct_count = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            distinct_count++;
        }
    }

    fwrite(&total_chars, sizeof(long), 1, out);
    fwrite(&distinct_count, sizeof(int), 1, out);
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            unsigned char character = (unsigned char)i;
            fwrite(&character, sizeof(unsigned char), 1, out);
            fwrite(&frequencies[i], sizeof(int), 1, out);
        }
    }

    rewind(in);
    unsigned char buffer = 0;
    int bit_count = 0;

    while ((ch = fgetc(in)) != EOF) {
        char *code = codes[(unsigned char)ch];
        for (int i = 0; code[i] != '\0'; i++) {
            buffer = buffer << 1;
            if (code[i] == '1') {
                buffer |= 1;
            }
            bit_count++;
            if (bit_count == 8) {
                fwrite(&buffer, sizeof(unsigned char), 1, out);
                buffer = 0;
                bit_count = 0;
            }
        }
    }

    if (bit_count > 0) {
        buffer = buffer << (8 - bit_count);
        fwrite(&buffer, sizeof(unsigned char), 1, out);
    }

    fclose(in);
    fclose(out);
    free_tree(root);

    long original_size = get_file_size(input_filename);
    long compressed_size = get_file_size(output_filename);
    double ratio = (double)compressed_size / (double)original_size * 100.0;

    printf("Compression complete.\n");
    printf("Original size    : %ld bytes\n", original_size);
    printf("Compressed size   : %ld bytes\n", compressed_size);
    printf("Compression ratio : %.2f%% of original size\n", ratio);
}

void decompress(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "rb");
    if (in == NULL) {
        printf("Error: could not open %s\n", input_filename);
        return;
    }

    long total_chars;
    fread(&total_chars, sizeof(long), 1, in);

    int distinct_count;
    fread(&distinct_count, sizeof(int), 1, in);

    int frequencies[256] = {0};
    for (int i = 0; i < distinct_count; i++) {
        unsigned char character;
        int frequency;
        fread(&character, sizeof(unsigned char), 1, in);
        fread(&frequency, sizeof(int), 1, in);
        frequencies[character] = frequency;
    }

    Node *root = build_huffman_tree(frequencies);

    FILE *out = fopen(output_filename, "wb");
    if (out == NULL) {
        printf("Error: could not create %s\n", output_filename);
        fclose(in);
        free_tree(root);
        return;
    }

    Node *current = root;
    long chars_written = 0;
    int byte;

    while (chars_written < total_chars && (byte = fgetc(in)) != EOF) {
        for (int bit_pos = 7; bit_pos >= 0 && chars_written < total_chars; bit_pos--) {
            int bit = (byte >> bit_pos) & 1;

            if (current->left == NULL && current->right == NULL) {
                fputc(current->character, out);
                chars_written++;
                current = root;
                if (chars_written >= total_chars) {
                    break;
                }
            }

            if (bit == 0) {
                current = current->left;
            } else {
                current = current->right;
            }

            if (current->left == NULL && current->right == NULL && chars_written < total_chars) {
                fputc(current->character, out);
                chars_written++;
                current = root;
            }
        }
    }

    fclose(in);
    fclose(out);
    free_tree(root);

    printf("Decompression complete. Output written to %s\n", output_filename);
}

int verify_match(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");

    if (f1 == NULL || f2 == NULL) {
        printf("Error: could not open files for comparison.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }

    int c1, c2;
    int identical = 1;

    while (1) {
        c1 = fgetc(f1);
        c2 = fgetc(f2);
        if (c1 != c2) {
            identical = 0;
            break;
        }
        if (c1 == EOF) {
            break;
        }
    }

    fclose(f1);
    fclose(f2);
    return identical;
}

int main(void) {
    const char *original_file = "telemetry.txt";
    const char *compressed_file = "telemetry.huff";
    const char *restored_file = "telemetry_restored.txt";

    printf("=== Telemetry Data Compression Utility ===\n\n");

    printf("Step 1: Compressing %s...\n", original_file);
    compress(original_file, compressed_file);

    printf("\nStep 2: Decompressing %s...\n", compressed_file);
    decompress(compressed_file, restored_file);

    printf("\nStep 3: Verifying restored file matches original...\n");
    if (verify_match(original_file, restored_file)) {
        printf("SUCCESS: '%s' is identical to '%s'.\n", restored_file, original_file);
    } else {
        printf("FAILURE: '%s' does NOT match '%s'.\n", restored_file, original_file);
    }

    return 0;
}
