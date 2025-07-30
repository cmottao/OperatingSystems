#include "utils.h"

int main() {
    // Open input CSV file and output index files
    FILE *csv = fopen("dataset.csv", "r");
    FILE *f_index = fopen("hash_index.bin", "wb+");
    FILE *f_nodes = fopen("index_nodes.bin", "wb+");

    // Check if all files opened correctly
    if(!csv || !f_index || !f_nodes) {
        perror("Error opening files");
        return 1;
    }

    // Initialize hash table in the index file with -1 (empty buckets)
    long empty = -1;
    for(int i = 0; i < TABLE_SIZE; i++) {
        fwrite(&empty, sizeof(long), 1, f_index);
    }

    // Allocate a buffer to read lines from the CSV
    char *buffer = malloc(BUFFER_SIZE * sizeof(char));
    if(!buffer) {
        perror("Error allocating memory");
        return 1;
    }

    // Skip header line in the CSV
    if(fgets(buffer, BUFFER_SIZE, csv) == NULL) {
        perror("Error leyendo la cabecera del CSV");
        free(buffer);
        fclose(csv);
        fclose(f_index);
        fclose(f_nodes);
        return 1;
    }

    // Process each record in the CSV
    while(fgets(buffer, BUFFER_SIZE, csv)) {
        // Get the byte offset of the current record
        long offset = ftell(csv) - strlen(buffer);

        // Extract the license plate (second field)
        char *token = strtok(buffer, ",");
        token = strtok(NULL, ",");

        // Compute hash and locate the corresponding bucket
        unsigned long hash = djb2_hash(token) % TABLE_SIZE;
        long bucket_offset = hash * sizeof(long);
        long old_head;

        // Read the current head of the bucket's linked list
        fseek(f_index, bucket_offset, SEEK_SET);
        if(fread(&old_head, sizeof(long), 1, f_index) != 1) {
            perror("Error leyendo el head del bucket");
            continue;
        }

        // Create a new node pointing to the old head
        IndexNode node;
        strncpy(node.key, token, MAX_PLATE_SIZE - 1);
        node.key[MAX_PLATE_SIZE - 1] = '\0';
        node.data_offset = offset;
        node.next_offset = old_head;

        // Append the new node to the end of the nodes file
        fseek(f_nodes, 0, SEEK_END);
        long new_node_offset = ftell(f_nodes);
        fwrite(&node, sizeof(IndexNode), 1, f_nodes);

        // Update the bucket head with the new node's offset
        fseek(f_index, bucket_offset, SEEK_SET);
        fwrite(&new_node_offset, sizeof(long), 1, f_index);
    }

    // Clean up
    free(buffer);
    fclose(csv);
    fclose(f_index);
    fclose(f_nodes);

    return 0;
}