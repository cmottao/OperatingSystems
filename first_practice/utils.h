#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// For named pipes
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>       
#include <unistd.h>     

// FIFO path
#define FIFO_C2S "/tmp/fifo_c2s"
#define FIFO_S2C "/tmp/fifo_s2c"

// Hash table and buffer configuration
#define TABLE_SIZE 20000003          
#define BUFFER_SIZE 256           
#define MAX_RESULTS 100           

// Field size limits
#define MAX_PLATE_SIZE 8         
#define MAX_CITY_SIZE 64         
#define MAX_DATE_SIZE 16      
#define MAX_PAID_SIZE 4

// Search criteria limits
#define MAX_PLATE_LENGTH 6
#define MAX_CITY_LENGTH 100
#define MIN_YEAR 2006
#define MAX_YEAR 2020

// Structure to hold search criteria
typedef struct {
    char plate[MAX_PLATE_SIZE];  
    char city[MAX_CITY_SIZE];      
    int year;                    
} SearchCriteria;

// Node stored in the hash index
typedef struct {
    char key[MAX_PLATE_SIZE];      
    long data_offset;              
    long next_offset;              
} IndexNode;

// Structure representing a full fine record
typedef struct {
    int year;                       
    char plate[MAX_PLATE_SIZE];  
    char date[MAX_DATE_SIZE];     
    int fine_amount;              
    char city[MAX_CITY_SIZE];      
    char paid[MAX_PAID_SIZE];      
} Fine;

// Structure for client internal state
typedef struct {
    int plate_set;
    int city_set;
    int year_set;
} SearchState;

// Combined structure for client internal use
typedef struct {
    SearchCriteria criteria;
    SearchState state;
} SearchData;

// Function to parse a CSV line into a Fine struct
Fine parse_fine(const char *line) {
    Fine fine;
    // Allocate temporary buffer to tokenize the line
    char *buffer = malloc(BUFFER_SIZE * sizeof(char));
    strncpy(buffer, line, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';

    // Extract each field from the CSV line
    char *token = strtok(buffer, ",");
    fine.year = atoi(token);

    token = strtok(NULL, ",");
    strncpy(fine.plate, token, sizeof(fine.plate) - 1);
    fine.plate[sizeof(fine.plate) - 1] = '\0';

    token = strtok(NULL, ",");
    strncpy(fine.date, token, sizeof(fine.date) - 1);
    fine.date[sizeof(fine.date) - 1] = '\0';

    token = strtok(NULL, ",");
    fine.fine_amount = atoi(token);                 

    token = strtok(NULL, ",");
    strncpy(fine.city, token, sizeof(fine.city) - 1);
    fine.city[sizeof(fine.city) - 1] = '\0';

    token = strtok(NULL, ",\n");
    strncpy(fine.paid, token, sizeof(fine.paid) - 1);
    fine.paid[sizeof(fine.paid) - 1] = '\0';

    return fine;
}

// DJB2 hash function for strings, used to compute bucket index in hash table
unsigned long djb2_hash(const char *str) {
    unsigned long hash = 5381, c;

    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash;
}