#include "utils.h"

// Function to pause execution
void pause_execution(void) {
    printf("\nPress Enter to continue...");
    getchar();
}

// Function to clear the screen portably
void clear_screen(void) {
#ifdef _WIN32
    if(system("cls") != 0) {
        perror("Error clearing screen");
    }
#else
    if(system("clear") != 0) {
        perror("Error clearing screen");
    }
#endif
}

// Function to validate year
int validate_year(int year) {
    return (year >= MIN_YEAR && year <= MAX_YEAR);
}

// Function to get safe string input
int get_string_input(char *buffer, size_t buffer_size, const char *prompt) {
    printf("%s", prompt);
    
    if(fgets(buffer, buffer_size, stdin) == NULL) {
        return 0;
    }
    
    // Remove newline
    size_t len = strlen(buffer);
    if(len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    
    return 1;
}

// Function to get safe integer input
int get_int_input(int *value, const char *prompt) {
    char input[20];
    char *endptr;
    
    printf("%s", prompt);
    
    if(fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }
    
    *value = strtol(input, &endptr, 10);
    
    // Check if conversion was successful
    if(*endptr != '\n' && *endptr != '\0') {
        return 0;
    }
    
    return 1;
}

// Function to initialize search criteria
void init_search_data(SearchData *data) {
    memset(&data->criteria, 0, sizeof(SearchCriteria));
    data->state.plate_set = 0;
    data->state.city_set = 0;
    data->state.year_set = 0;
}

// Function to print current criteria
void print_current_criteria(const SearchData *data) {
    printf("\n=== CURRENT CRITERIA ===\n");
    
    if(data->state.plate_set) {
        printf("✓ Plate: %s\n", data->criteria.plate);
    } else {
        printf("✗ Plate: Not selected (Mandatory)\n");
    }

    if(data->state.city_set) {
        printf("✓ City: %s\n", data->criteria.city);
    } else {
        printf("✗ City: Not selected (Optional)\n");
    }
    
    if(data->state.year_set) {
        printf("✓ Year: %d\n", data->criteria.year);
    } else {
        printf("✗ Year: Not selected (Optional)\n");
    }
    
    printf("==========================\n\n");
}

// Function to print the main menu
void print_menu(const SearchData *data) {
    clear_screen();
    
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                 TRAFFIC FINE FINDER                      ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Available options:\n");
    printf("1. Configure Plate (Mandatory)\n");
    printf("2. Configure City (Optional)\n");
    printf("3. Configure Year of Validity (Optional)\n");
    printf("4. Perform Search\n");
    printf("5. Exit\n\n");
    
    print_current_criteria(data);
    printf("Select an option: ");
}

// Function to handle plate input
void handle_plate_input(SearchData *data) {
    clear_screen();
    printf("=== PLATE CONFIGURATION ===\n\n");
    
    char plate_input[MAX_PLATE_SIZE + 1];
    
    do {
        if(!get_string_input(plate_input, sizeof(plate_input), 
                             "Enter plate (5-6 characters): ")) {
            printf("Error reading input.\n");
            continue;
        }
        
        // Validate plate length
        size_t plate_len = strlen(plate_input);
        if(plate_len < 5 || plate_len > 6) {
            printf("Plate must be between 5 and 6 characters.\n");
            continue;
        }
        
        // Convert to uppercase
        for(int i = 0; plate_input[i]; i++) {
            plate_input[i] = toupper(plate_input[i]);
        }
        
        strncpy(data->criteria.plate, plate_input, MAX_PLATE_SIZE - 1);
        data->criteria.plate[MAX_PLATE_SIZE - 1] = '\0';
        data->state.plate_set = 1;
        
        printf("✓ Plate configured successfully: %s\n", data->criteria.plate);
        break;
        
    } while (1);
}

// Function to handle city input
void handle_city_input(SearchData *data) {
    clear_screen();
    printf("=== CITY CONFIGURATION ===\n\n");
    
    char city_input[MAX_CITY_SIZE + 1];
    
    do {
        if(!get_string_input(city_input, sizeof(city_input), 
                             "Enter city: ")) {
            printf("Error reading input.\n");
            continue;
        }
        
        if(strlen(city_input) == 0) {
            printf("City cannot be empty.\n");
            continue;
        }
        
        strncpy(data->criteria.city, city_input, MAX_CITY_SIZE - 1);
        data->criteria.city[MAX_CITY_SIZE - 1] = '\0';
        data->state.city_set = 1;
        
        printf("✓ City configured successfully: %s\n", data->criteria.city);
        break;
        
    } while (1);
}

// Function to handle year input
void handle_year_input(SearchData *data) {
    clear_screen();
    printf("=== YEAR OF VALIDITY CONFIGURATION ===\n\n");
    
    int year_input;
    
    do {
        if(!get_int_input(&year_input, "Enter year of validity: ")) {
            printf("Error reading input. Please enter a valid number.\n");
            continue;
        }
        
        if(!validate_year(year_input)) {
            printf("Invalid year. Must be between %d and %d.\n", MIN_YEAR, MAX_YEAR);
            continue;
        }
        
        data->criteria.year = year_input;
        data->state.year_set = 1;
        
        printf("✓ Year of validity configured successfully: %d\n", data->criteria.year);
        break;
        
    } while (1);
}

// Function to perform the search
void perform_search(const SearchData *data) {
    clear_screen();
    printf("=== PERFORMING SEARCH ===\n\n");
    
    if(!data->state.plate_set) {
        printf("\u2717 Error: Plate is mandatory to perform the search.\n");
        printf("Please configure the plate first.\n");
        pause_execution();
        return;
    }
    
    printf("Searching for fines with the following criteria:\n");
    print_current_criteria(data);

    // Open the client-to-server FIFO to write criteria
    int wfd = open(FIFO_C2S, O_WRONLY);
    if(wfd == -1) {
        perror("open FIFO_C2S");
        exit(EXIT_FAILURE);
    }
    if(write(wfd, &data->criteria, sizeof(SearchCriteria)) == -1) {
        perror("write FIFO_C2S");
        close(wfd);
        exit(EXIT_FAILURE);
    }
    close(wfd);

    // Open the server-to-client FIFO to read results
    int rfd = open(FIFO_S2C, O_RDONLY);
    if(rfd == -1) {
        perror("open FIFO_S2C");
        exit(EXIT_FAILURE);
    }

    int found;
    if(read(rfd, &found, sizeof(int)) == -1) {
        perror("read FIFO_S2C");
        close(rfd);
        exit(EXIT_FAILURE);
    }

    Fine results[found];
    for(int i = 0; i < found; i++) {
        if(read(rfd, &results[i], sizeof(Fine)) == -1) {
            perror("read FIFO_S2C");
            close(rfd);
            exit(EXIT_FAILURE);
        }
    }

    if(found == 0) {
        printf("==========================\n");
        printf("No records found.\n");
        printf("==========================\n");
        close(rfd);
        pause_execution();
        return;
    } else {
        printf("Found %d records\n", found);
    }

    for(int i = 0; i < found; i++) {
        printf("==========================\n");
        printf("Year: %d\n", results[i].year);
        printf("Plate: %s\n", results[i].plate);
        printf("Fine Date: %s\n", results[i].date);
        printf("Amount: %d\n", results[i].fine_amount);
        printf("City: %s\n", results[i].city);
        printf("Paid: %s\n", results[i].paid);
    }
    printf("==========================\n");
    close(rfd);
    pause_execution();
}

// Main function
int main(void) {
    SearchData data;
    int choice;
    
    // Initialize criteria
    init_search_data(&data);
    
    printf("Welcome to the Traffic Fine Finder\n");
    printf("Press Enter to continue...");
    getchar();
    
    do {
        print_menu(&data);
        
        // Get and validate menu input
        if(!get_int_input(&choice, "")) {
            printf("Invalid input. Please enter a number.\n");
            getchar();
            continue;
        }
        
        // Process selected option
        switch(choice) {
            case 1:
                handle_plate_input(&data);
                break;
                
            case 2:
                handle_city_input(&data);
                break;
                
            case 3:
                handle_year_input(&data);
                break;
                
            case 4:
                perform_search(&data);
                break;
                
            case 5:
                printf("You have selected to exit.\n");
                break;
                
            default:
                printf("Invalid option. Please select an option from 1 to 5.\n");
                getchar();
                continue;
        }
        
    } while (choice != 5);

    return 0;
}