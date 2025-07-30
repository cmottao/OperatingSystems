#include "utils.h"

// Perform a search based on the given criteria and stores the matching Fine records in 'results'
void search(SearchCriteria criteria, Fine *results, int *out_found) {
    // Open the necessary files for reading the hash index and original dataset
    FILE *f_index = fopen("hash_index.bin", "rb");
    FILE *f_nodes = fopen("index_nodes.bin", "rb");
    FILE *csv = fopen("dataset.csv", "r");

    if(!f_index || !f_nodes || !csv) {
        perror("Error opening files");
        return;
    }

    // Compute the hash of the license plate to find the correct bucket
    unsigned long hash = djb2_hash(criteria.plate) % TABLE_SIZE;
    long bucket_offset = hash * sizeof(long);
    long current_offset;

    // Get the starting node of the linked list in the bucket
    fseek(f_index, bucket_offset, SEEK_SET);

    if(fread(&current_offset, sizeof(long), 1, f_index) != 1) {
        perror("Error reading index file");
        fclose(f_index);
        fclose(f_nodes);
        fclose(csv);
        return;
    }

    // Buffer to read lines from the CSV file
    char *buffer = malloc(BUFFER_SIZE * sizeof(char));
    if(!buffer) {
        perror("Error allocating memory");
        return;
    }

    int found = 0;

    // Traverse the linked list in the bucket
    while(current_offset != -1) {
        IndexNode node;

        // Read the index node from the nodes file
        fseek(f_nodes, current_offset, SEEK_SET);
        if(fread(&node, sizeof(IndexNode), 1, f_nodes) != 1) {
            perror("Error reading index node");
            break;
        }

        // Check if the key matches the plate we're searching for
        if(strncmp(node.key, criteria.plate, MAX_PLATE_SIZE) == 0) {
            // Go to the corresponding offset in the CSV and read the record
            fseek(csv, node.data_offset, SEEK_SET);

            if(fgets(buffer, BUFFER_SIZE, csv)) {
                Fine fine = parse_fine(buffer);

                // Filter based on year and city (optional)
                int matches = 1;  // Start assuming it matches
                
                // If a city is specified (non-empty), check for match
                if(strlen(criteria.city) > 0) {
                    matches = matches && (strcmp(fine.city, criteria.city) == 0);
                }
                
                // If a year is specified (greater than 0), check for match  
                if(criteria.year > 0) {
                    matches = matches && (fine.year == criteria.year);
                }
                
                if(matches) {
                    results[found++] = fine;
                }
            }
        }
        // Move to the next node in the bucket chain
        current_offset = node.next_offset;
    }
    // Store the number of records found
    *out_found = found;

    // Clean up
    free(buffer);
    fclose(f_index);
    fclose(f_nodes);
    fclose(csv);
}

int main() {
    printf("========================================\n");
    printf("         SEARCH ENGINE ACTIVE\n");
    printf("========================================\n");
    
    while(1) {
        Fine *results = malloc(MAX_RESULTS * sizeof(Fine));
        if(!results) {
            perror("Error allocating memory");
            return 1;
        }
        int found;

        // Open the FIFO from client to server to read search criteria
        int rfd = open(FIFO_C2S, O_RDONLY);
        if(rfd == -1) {
            perror("Error opening FIFO_C2S");
            exit(EXIT_FAILURE);
        }

        SearchCriteria criteria;
        if(read(rfd, &criteria, sizeof(SearchCriteria)) == -1) {
            perror("Error reading FIFO_C2S");
            close(rfd);
            exit(EXIT_FAILURE);
        }
        close(rfd);

        // Perform the search
        search(criteria, results, &found);

        // Open the FIFO from server to client to write results
        int wfd = open(FIFO_S2C, O_WRONLY);
        if(wfd == -1) {
            perror("Error opening FIFO_S2C");
            exit(EXIT_FAILURE);
        }

        if(write(wfd, &found, sizeof(int)) == -1) {
            perror("Error writing FIFO_S2C");
            close(wfd);
            exit(EXIT_FAILURE);
        }

        if(found == 0) {
            printf("========================================\n");
            printf("      No records were found\n");
            printf("========================================\n");
            close(wfd);
        } 
        else {
            printf("========================================\n");
            printf("     %d records were found\n", found);
            printf("========================================\n");
            for(int i = 0; i < found; i++) {
                Fine *f = &results[i];

                if(write(wfd, f, sizeof(Fine)) == -1) {
                    perror("Error writing FIFO_S2C");
                    close(wfd);
                    exit(EXIT_FAILURE);
                }
            }
            close(wfd);
        }
        free(results);
    }
    return 0;
}