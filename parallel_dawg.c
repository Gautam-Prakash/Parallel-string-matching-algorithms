#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_WORD_LENGTH 50
#define ALPHABET_SIZE 26

/* Node in the DAWG */
typedef struct Node {
    int index;                /* Unique identifier for the node */
    int is_end_of_word;       /* Flag to indicate end of word */
    struct Node *next[ALPHABET_SIZE];  /* Pointers to next nodes */
} Node;

/* Initialize a new node */
Node* new_node(int index) {
    Node *n = (Node*) malloc(sizeof(Node));
    n->index = index;
    n->is_end_of_word = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        n->next[i] = NULL;
    }
    return n;
}

/* Build a DAWG from a set of words */
Node* build_dawg(char **words, int n) {
    Node *root = new_node(0);
    Node *prev = root;
    int i, j;
    for (i = 0; i < n; i++) {
        Node *curr = root;
        for (j = 0; j < strlen(words[i]); j++) {
            int c = words[i][j] - 'a';
            if (curr->next[c] == NULL) {
                curr->next[c] = new_node(++(prev->index));
            }
            curr = curr->next[c];
        }
        curr->is_end_of_word = 1;
        curr->index = i+1;
    }
    return root;
}

/* Search for patterns in a text using the DAWG */
void forward_dawg_match(char *text, Node *root, char **patterns, int k) {
    int n = strlen(text);
    int i, j, pos;
    Node *curr_node;
    #pragma omp parallel for private(i, j, pos, curr_node)
    for (pos = 0; pos < n; pos++) {
        curr_node = root;
        for (i = pos; i < n; i++) {
            int c = text[i] - 'a';
            if (curr_node->next[c] == NULL) {
                break;
            }
            curr_node = curr_node->next[c];
            if (curr_node->is_end_of_word) {
                #pragma omp critical
                {
                    printf("Pattern found at position %d: %s\n", pos, patterns[curr_node->index - 1]);
                }
            }
        }
    }
}

/* Test the implementation */
int main() {
    char *patterns[] = {"hello", "world", "he", "llo", "orl", "d"};
    char *text = "helloworld";
    int k = 6;
    int n = strlen(text);
    Node *root = build_dawg(patterns, k);
    forward_dawg_match(text, root, patterns, k);
    return 0;
}

