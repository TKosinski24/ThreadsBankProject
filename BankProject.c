//=============================================================================
//
// Title:      Bank Project
// Course:     CSC 4420
// Author:     Timothy Kosinski
// Date:       06DEC2023   
// 
// ==============================================================================
//    
// PROJECT INCORPORATES:
// 
// 1. Creating a new account with a Unique ID
// 2. Perform Deposits and Withdraws from a specific account
// 3. Check the balance of a specific Account ID
// 4. Retreve the last 5 Transactions from a specific Account ID
// 5. Recieve all Transactions from current session (If selected or Previous)
// 
//  - Use Pthreads to associate each thread with its respected region
//  - This is off off the first leading digit (N,E,S,W)
//               1 - East Branch
//               2 - West Branch
//               3 - North Branch
//               4 - South Branch
// 
//=============================================================================

//Version 6.0  (Extra credit question 1) - fully incorporated


//Libraries Needed
#include <stdio.h> // 'printf', 'scanf'
#include <stdlib.h> // 'malloc', 'free', 'exit'
#include <string.h>// 'strcpy', strcmp' ,'strlen'
#include <pthread.h> // Creating, Managing and Synchronizing Threads
#include <stdbool.h> // 'bool', 'true', 'false'
#include <unistd.h> // 'sleep'

// Define Constants
#define MAX_ACCOUNTS 100
#define MAX_BRANCHES 4
#define LINE_LENGTH 256
#define NUM_THREADS 4

// Account Structure
typedef struct {
    int account_id;
    double balance;
    char branch[50]; // Store the branch name here
    pthread_mutex_t lock;
} Account;

// Thread Data Structure
typedef struct {
    char branch_name[50]; // Branch handled by the thread
} ThreadData;

// Initializing Account Array & Count
Account accounts[MAX_ACCOUNTS];
int num_accounts = 0;


pthread_mutex_t accounts_mutex; // Mutex for synchronizing access to the accounts array

//Function Prototypes
void* branch_handler(void* arg);
void update_account(int account_id, const char* branch, const char* operation, double amount);
void Initial_Transactions(const char *filename);
bool account_exists(int account_id, const char* filename);
void create_new_account(const char *starter_filename);
void deposit_withdraw(const char *filename);
void check_balance();
void five_transactions(const char *filename);
void retrieve_history(const char *filename);
void sort_accounts_by_id(Account accounts[], int num_accounts);
void DuplicateFile(const char* input_filename, const char* output_filename);
void write_account_to_branch_file(int account_id, const char* branch);
char* getBranchName(int account_id);
void display_loading();


// Used for appending branch specific files (North,East,South,West)
void* branch_handler(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    //printf("Handler for branch: %s\n", data->branch_name); //help with debugging... no longer needed

    // Create file for branch files (N,E,S,W)
    char filename[64];
    sprintf(filename, "%sbranch.csv", data->branch_name);

    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Account ID,Branch,Total Balance\n");
    }

    // lock the accounts_mutex to ensure safety to accounts array!
    pthread_mutex_lock(&accounts_mutex);

    // Iterates and appends to branch specific file
    for (int i = 0; i < num_accounts; i++) {
        if (strcmp(accounts[i].branch, data->branch_name) == 0) {
            fprintf(file, "%d,%s,%.2f\n", accounts[i].account_id, accounts[i].branch, accounts[i].balance);
        }
    }

    // Unlocks accounts_mutex to release lock
    pthread_mutex_unlock(&accounts_mutex);

    //Close File
    fclose(file);
    return NULL;
}



// Used to update specific account balance when Depositing / Withdrawing
void update_account(int account_id, const char* branch, const char* operation, double amount) {

    // Lock the accounts_mutex to ensure safety to accounts array
    pthread_mutex_lock(&accounts_mutex);

    int found = 0; // used as a flag
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_id == account_id) {
            found = 1; // account with specific ID found

            // Check operation type (Withdraw or Deposit) - update account accordinly 
            if (strcmp(operation, "deposit") == 0) {
                accounts[i].balance += amount;// Deposit - increases balance
            } else if (strcmp(operation, "withdraw") == 0) {
                accounts[i].balance -= amount;// Withdraw - decreases balance 
            }

            // Upsates Branch!
            strcpy(accounts[i].branch, branch); // Update branch
            break; // breaks loop once account is updated and found
        }
    }

    if (!found) {
        // If not found, create new account!!
        accounts[num_accounts].account_id = account_id;

        if (strcmp(operation, "deposit") == 0) {
            accounts[num_accounts].balance = amount;
        }
        else if (strcmp(operation, "withdraw") == 0) {
            accounts[num_accounts].balance = -amount;
        }
    
        strcpy(accounts[num_accounts].branch, branch); // Store branch name

        // Used to initialize mutex lock for new account!
        pthread_mutex_init(&accounts[num_accounts].lock, NULL);
        num_accounts++;
        printf("New account created: ID %d, Branch %s, Balance %.2f\n", account_id, branch, accounts[num_accounts].balance);
    }

    //Unlocks accounts_mutex to release lock
    pthread_mutex_unlock(&accounts_mutex);
}


// Reads initial transactions from a CSV file and updates accounts to file format
void Initial_Transactions(const char* filename) {
    // Open file and Error Checking
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[LINE_LENGTH];
    int account_id;
    char branch[50], operation[10];
    double amount;

    fgets(line, LINE_LENGTH, file); // Skips header

    while (fgets(line, LINE_LENGTH, file)) {
        if (sscanf(line, "%*[^,],%d,%[^,],%*[^,],%[^,],%lf", &account_id, branch, operation, &amount) == 4) {
            update_account(account_id, branch, operation, amount);
        }
    }

    // Close File
    fclose(file);
}


//Checks if an account with specific id exists in the .CSV file
bool account_exists(int account_id, const char* filename) {
    // Open File and Error Checking
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open the file to check for existing account");
        return false; // if file doesnt open. return false
    }

    char line[LINE_LENGTH];
    int existing_account_id;
    // Read the file line by line
    while (fgets(line, LINE_LENGTH, file) != NULL) {
        if (sscanf(line, "%d", &existing_account_id) == 1) {
            if (existing_account_id == account_id) {
                fclose(file);
                return true; // Found the account ID, return true.
            }
        }
    }

    //Close File
    fclose(file);
    return false; // Reached end of file without finding the account ID, return false.
}

// Create new account for newly added users
void create_new_account(const char* starter_filename) {
    int account_id;
    char branch_name[50];
    char branch_filename[64];

    // User interface
    while (true) {
        printf("\nCreate NEW Account:\n");
        printf("Enter Account ID (Leading Digit: 1- East, 2- West, 3- North, 4- South) \n");
        printf("User Input: ");

        if (scanf("%d", &account_id) != 1) {
            printf("Error! Please enter a Valid Input!\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        // Determines first digit in the account_id
        int start_digit = account_id;
        while (start_digit >= 10) {
            start_digit /= 10;
        }

        // 4 branches - 4 digits 
        if (start_digit < 1 || start_digit > 4) {
            printf("Error! Invalid Branch\n");
            continue;
        }

        // Validate and Assign branch name
        strcpy(branch_name, getBranchName(start_digit));

        // Generates branch file
        sprintf(branch_filename, "%sbranch.csv", branch_name);

        // Check for unique Account ID
        if (account_exists(account_id, branch_filename)) {
            printf("An account with this ID already exists in the %s branch. Please enter a unique Account ID.\n", branch_name);
        }
        else {
            break; // Unique ID found, break the loop
        }
    }

    // Lock for thread safety
    pthread_mutex_lock(&accounts_mutex);
    // Add the new account to the accounts array
    accounts[num_accounts].account_id = account_id;
    accounts[num_accounts].balance = 0.0; // Initialize balance to zero
    strcpy(accounts[num_accounts].branch, branch_name);
    pthread_mutex_init(&accounts[num_accounts].lock, NULL);
    num_accounts++;
    printf("\nNew account created:\nAccount ID: %d, Branch: %s, Balance: %.2f\n", account_id, branch_name, accounts[num_accounts - 1].balance);
    // Unlock thread after completion 
    pthread_mutex_unlock(&accounts_mutex);

    // Append to the starter.csv file
    FILE* file = fopen(starter_filename, "a"); // Open for appending
    if (file) {
        fprintf(file, "%s,%d,%s,%s,%s,%.2f\n", "none", account_id, branch_name, "none", "deposit", 0.0); // Appending initial deposit of 0
        //Close File
        fclose(file);
        //printf("New account [%d] created and written to starter.csv\n", account_id); // Debug print, not needed anymore
    }
    else {
        perror("Error opening file"); // Error Checking
    }
}



// Deposit or Withdraw from an Existing Account
void deposit_withdraw(const char* filename) {
    int account_id;

    // User Interface
    printf("\nDeposit or Withdraw from an account:\n");
    printf("Enter valid account ID: ");
    scanf("%d", &account_id); // User input

    // Find the account in the accounts array
    int found = 0;
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_id == account_id) {
            found = 1;

            // Perform deposit or withdrawal
            char operation[10];
            double amount;
            printf("Enter operation (deposit/withdraw): ");
            scanf("%s", operation);

            if (strcmp(operation, "deposit") == 0) {
                printf("Enter deposit amount: ");
                scanf("%lf", &amount);
                if (amount > 0) {
                    // Lock the account for protecton
                    pthread_mutex_lock(&accounts[i].lock);
                    accounts[i].balance += amount;

                    // Unlock after adding account balance
                    pthread_mutex_unlock(&accounts[i].lock);
                    printf("Deposit of $%.2f successful. New balance: $%.2f\n", amount, accounts[i].balance);

                    // Append transaction to starter.csv
                    FILE* file = fopen(filename, "a");
                    if (file) {
                        fprintf(file, "%s,%d,%s,%s,%s,%.2f\n", "none", account_id, accounts[i].branch, "none", operation, amount);
                        fclose(file);

                    }
                    else {
                        perror("Error opening file for transaction");
                    }
                }
                else {
                    printf("Invalid deposit amount. Amount must be greater than 0.\n");
                }
            }
            else if (strcmp(operation, "withdraw") == 0) {
                printf("Enter withdrawal amount: ");
                scanf("%lf", &amount);
                if (amount > 0 && amount <= accounts[i].balance) {
                    // Lock the account for protection
                    pthread_mutex_lock(&accounts[i].lock);
                    accounts[i].balance -= amount;

                    // Unlock after computing decreasing account balance
                    pthread_mutex_unlock(&accounts[i].lock);
                    printf("Withdrawal of $%.2f successful. New balance: $%.2f\n", amount, accounts[i].balance);

                    // Append transaction to starter.csv
                    FILE* file = fopen(filename, "a");
                    if (file) {
                        fprintf(file, "%s,%d,%s,%s,%s,%.2f\n", "none", account_id, accounts[i].branch, "none", "withdraw", amount);
                        fclose(file);
                    }
                    else {
                        perror("Error opening file for transaction");
                    }
                }
                else {
                    printf("Invalid withdrawal. Check amount and account balance.\n");
                }
            }
            else {
                printf("Invalid operation. Please enter 'deposit' or 'withdraw'.\n");
            }

            // Update the specific branch .csv file
            write_account_to_branch_file(account_id, accounts[i].branch);
            break;
        }
    }

    if (!found) {
        printf("Account ID not found\n"); // Error Checking
    }
}



// Checks current balance of an Account ID
void check_balance() {

    // User Account ID
    int account_id;
    printf("\nChecking current account balance:\n");
    printf("Enter valid account ID: ");
    scanf("%d", &account_id);

    char branch_name[50]; // Declare branch_name 
    strcpy(branch_name, getBranchName(account_id)); // Determine Branch name

    char branch_filename[64];
    sprintf(branch_filename, "%sbranch.csv", branch_name);

    // Open Branch File and Error Checking
    FILE *file = fopen(branch_filename, "r");
    if (!file) {
        perror("Error opening branch file");
        return;
    }


    char line[LINE_LENGTH];
    int found_account_id;
    double balance;
    bool found = false;

    // Skip header line
    fgets(line, LINE_LENGTH, file);
    
    // Read file line by line
    while (fgets(line, LINE_LENGTH, file)) {
        sscanf(line, "%d,%*[^,],%lf", &found_account_id, &balance);
        if (found_account_id == account_id) {
            found = true;
            break;
        }
    }
    // Close File
    fclose(file);

    
    if (found) {
        // Message if found display Account ID, Branch and Balance
        printf("Account ID: %d, Branch: %s, Balance: $%.2f\n", account_id, branch_name, balance);
    } else {
        // Message Account ID not found
        printf("Account ID not found");
    }
}


// Retrieves all current history from current section ( Reading contents of a .csv File )
void retrieve_history(const char *filename) {

    // Open file, with error checking
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[LINE_LENGTH];
    int transaction_number = 0;
    int account_id;
    char branch[50], operation[10];
    double amount;

    // Formattted otput
    printf("\nPrinting Updated History from all transactions\n");
    printf("------------------------------------------------------------------\n");
    printf("Transaction | Account ID  |   Branch  | Operation   |   Amount   \n");
    printf("------------------------------------------------------------------\n");

    // Skip header line
    fgets(line, LINE_LENGTH, file);

    while (fgets(line, LINE_LENGTH, file)) {
        if (sscanf(line, "%*[^,],%d,%[^,],%*[^,],%[^,],%lf", &account_id, branch, operation, &amount) == 4) {
            printf("  [ %04d ]  |  [ %4d ]   |   %-6s  |  %-9s  |  $%8.2f\n", ++transaction_number, account_id, branch, operation, amount);
            
        }
    }
    printf("------------------------------------------------------------------\n");

    // Close File
    fclose(file);
}


// Displays the last 5 transactions of a given Account ID
void five_transactions(const char *filename) {
    int account_id;
    printf("\nRetrieve the last 5 transactions for a Valid account ID:\n");
    printf("NOTE: If INVALID account, then there will be no transaction history to display!\n");
    printf("Enter account ID: ");
    scanf("%d", &account_id);

    // Opens "starter.csv" file 
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening starter file");
        return;
    }

    char line[LINE_LENGTH];
    int transaction_count = 0;
    char operation[10];
    double amount;

    // Stores transactions into a struct, then reads them
    struct Transaction {
        int account_id;
        char operation[10];
        double amount;
    } transactions[1000]; 

    // Read all transactions for this account ID
    while (fgets(line, LINE_LENGTH, file) && transaction_count < 1000) {
        int read_account_id;
        if (sscanf(line, "%*[^,],%d,%*[^,],%*[^,],%[^,],%lf", &read_account_id, operation, &amount) == 3) {
            if (read_account_id == account_id) {
                transactions[transaction_count].account_id = read_account_id;
                strcpy(transactions[transaction_count].operation, operation);
                transactions[transaction_count].amount = amount;
                transaction_count++;
            }
        }
    }

    // File no longer needed, Close File
    fclose(file);

    // Display the last 5 transactions
    printf("\nLast 5 transactions for Account ID: %d\n", account_id);
    printf("--------------------------------------------\n");
    printf("Transaction  | Operation  | Amount\n");
    printf("--------------------------------------------\n");

    // Prints up to 5 transactions
    int start_index = transaction_count - 5 > 0 ? transaction_count - 5 : 0; 
    for (int i = start_index; i < transaction_count; i++) {
        printf("   [%d]       | %-10s | $%.2f\n", i + 1 - start_index, transactions[i].operation, transactions[i].amount);
    }
    // If less than 5 transactions, fill in the rest with 'none'
    for (int i = transaction_count; i < start_index + 5; i++) {
        printf("   [%d]       | %-10s | $%s\n", i + 1 - start_index, "none", "0.00");
    }
    printf("--------------------------------------------\n");
}




// Used for formatting user display in main function
// NOTE: displays account from ( Least - Greatest ) from Account ID
void sort_accounts_by_id(Account accounts[], int num_accounts) {
    int i, j;
    for (i = 0; i < num_accounts - 1; i++) {
        for (j = 0; j < num_accounts - i - 1; j++) {
            if (accounts[j].account_id > accounts[j + 1].account_id) {
                // Swap accounts - to create order
                Account temp = accounts[j];
                accounts[j] = accounts[j + 1];
                accounts[j + 1] = temp;
            }
        }
    }
}


// Used to duplicate initial_transactions -> starter.csv : to ensure that the file starts off fresh each session!
// If user wants too
void DuplicateFile(const char* input_filename, const char* output_filename) {

    // Opening input_filname and Error Checking
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Error opening input file");
        return;
    }

    // Opening output_filename and Error Checking
    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Error opening output file");
        fclose(input_file);
        return;
    }

    // Grabbing line and copying to Output File
    char buffer[LINE_LENGTH];
    while (fgets(buffer, LINE_LENGTH, input_file) != NULL) {
        fputs(buffer, output_file);
    }

    // Closing ( Input & Output ) Files
    fclose(input_file);
    fclose(output_file);
}

// Writes to branch specific File (North, East, South, West)
void write_account_to_branch_file(int account_id, const char* branch) {
    char filename[64];
    sprintf(filename, "%sbranch.csv", branch);

    FILE* file = fopen(filename, "a"); // Open for appending
    if (!file) {
        perror("Error opening branch file"); // Error checking
        return;
    }

    //Prints to file
    fprintf(file, "%d,%s,%.2f\n", account_id, branch, accounts[num_accounts-1].balance);
 
}


// Determines branch name (1-East, 2-West, 3-North, 4-South)
char* getBranchName(int account_id) {
    int start_digit = account_id;

    // Determines first leading Digit in Account ID
    while (start_digit >= 10) {
        start_digit /= 10;
    }

    static char branch_name[50]; // Static buffer to store the branch name

    // (1-East, 2-West, 3-North, 4-South)
    switch (start_digit) {
    case 1: strcpy(branch_name, "east"); break;
    case 2: strcpy(branch_name, "west"); break;
    case 3: strcpy(branch_name, "north"); break;
    case 4: strcpy(branch_name, "south"); break;
    default:

        // Last ditch effort just incase something goes wrong - Invalid Branch!
        strcpy(branch_name, "Invalid"); // Used for error checking
        printf("Invalid! Branch Name!!\n");
    }

    return branch_name; // returning branch
}

// Function to display loading dots
void display_Processing(const char* text) {
    for (int i = 0; i < 3; i++) {
        printf("\r%s", text);
        for (int j = 0; j < i % 3 + 1; j++) {
            printf(".");
        }
        fflush(stdout);
        sleep(1);
    }
    printf("\r            \r"); // Clear the loading dots
}

//Driver Program!! Heart of the Multi-Region Banking System!! :)
int main() {
    pthread_mutex_init(&accounts_mutex, NULL);
    int sessionChoice; // User input for (New Session or Resume Session)
    
    // Displaying "Loading Program..."
    display_Processing("Loading Program");
    system("clear");
    
    // Display Option for ( New Session or Resume Session )
    printf("-------------------------------------------\n");
    printf(" !! Welcome to 4 Region Banking System !!\n");
    printf("-------------------------------------------\n");
    printf("Choose session type:\n");
    printf("NOTE: If it's the first time, type 1 and press Enter!\n");
    printf("1 - Start a new session (overwrites file)\n");
    printf("2 - Continue from a previous session (appends to file)\n");
    printf("Enter your choice: ");
    scanf("%d", &sessionChoice);

    // Call DuplicateFile with the user's choice (1 - Overwrite, 2 - Append)
    if (sessionChoice == 1) {
        printf("\nYou've Selected to Start from a NEW Session!\n");
        DuplicateFile("initial_transaction.csv", "starter.csv");
        display_Processing("Loading");
    } else if (sessionChoice == 2) {
        printf("\nYou've Selected to Continue from PREVIOUS Session.\n");
        display_Processing("Loading");
    } else {
        printf("Error! Invalid input.\n");
        display_Processing("Exiting Program");
        system("clear");
        return 1; // Exit the program if the input is invalid
    }

    // Processing "starter.csv" File
    Initial_Transactions("starter.csv");

    // Create and join threads
    ThreadData threadData[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    const char* branchNames[] = { "north", "east", "south", "west" };

    // User Choice - for each function
    int choice;
    while (true) {
        // Clear Terminal at the start and every iteration - To look "New"
        system("clear");

        // Create threads for each branch
        for (int i = 0; i < NUM_THREADS; i++) {
            strcpy(threadData[i].branch_name, branchNames[i]);
            pthread_create(&threads[i], NULL, branch_handler, (void*)&threadData[i]);
        }

        // Join threads to wait for their completion
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        // Sort accounts by ID
        sort_accounts_by_id(accounts, num_accounts);

        // Display account information
        //NOTE: if any Account ID is larger than "0000" - formatting will be off, Didnt have time to incorporate!
        printf("\n\n!!! Accounts Loaded from all Regions ( North, East, South, West ) !!!\n");
        printf("---------------------------------------------------------------------\n");
        printf("  Account #      |   Account ID    |   Branch     |   Total Balance   \n");
        printf("---------------------------------------------------------------------\n");

        for (int i = 0; i < num_accounts; i++) {
            pthread_mutex_lock(&accounts[i].lock);
            printf("    [ %04d ]     |     [ %4d ]    |    %-10s|    $%.2f \n",
                i + 1, accounts[i].account_id, accounts[i].branch, accounts[i].balance);
            pthread_mutex_unlock(&accounts[i].lock);
        }

        printf("---------------------------------------------------------------------\n\n");

        // Menu for bank operations
        printf("\nBank Operations:\n");
        printf("1. Create a new account\n");
        printf("2. Deposit or withdraw from an account\n");
        printf("3. Check account balance\n");
        printf("4. Retrieve the last 5 transactions for an account\n");
        printf("5. Retrieve all history from this current session\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // switch statements for different functions
        switch (choice) {
        case 1:
            create_new_account("starter.csv");
            printf("\nPress Enter to continue...");
            break;
        case 2:
            deposit_withdraw("starter.csv");
            printf("\nPress Enter to continue...");
            break;
        case 3:
            check_balance();
            printf("\nPress Enter to continue...");
            break;
        case 4:
            five_transactions("starter.csv");
            printf("\nPress Enter to continue...");
            break;
        case 5:
            printf("\nClearing screen hold tight!\n");
            display_Processing("Loading");
            
            system("clear");
            retrieve_history("starter.csv");
            printf("\nPress Enter to continue...");
            break;
        case 6:
            // Exit the program
            display_Processing("Exiting Program");
            system("clear");
            exit(0); // Used to terminate the program
        default:
            printf("Invalid choice. Please try again.\n");
            printf("\nPress Enter to continue...");
        }

        // Waits for user input to refresh the screen (System ("clear") - will be called)
        getchar(); // Takes in user input (Enter)
        getchar(); // Waits for user input (Enter Again)
    }

    // Clean up - Mutex
    for (int i = 0; i < num_accounts; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
    pthread_mutex_destroy(&accounts_mutex);

    return 0;
    // End of program!!! - this was a fun project :)
}
