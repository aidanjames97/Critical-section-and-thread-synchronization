// assign5
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// struct for completing transactions
typedef struct {
    char *type; // deposit, withdrawl
    char *accountName;
    int amount;
} Transaction;

// struct for clients
typedef struct {
    char *name; // (C1, C2, etc)
    int numTransactions;
    Transaction transactions[10];
} Client;

// struct for accounts
typedef struct {
    char *name; // (A1, A2, etc.)
    int balance; // holds acc amount
} Account;

// global variables
pthread_mutex_t lock; // lock for semaphore
int numAccounts = 0; // count accounts
int numClients = 0; // count clients
Account *accounts; // init account pointer
Client *clients; // init clients pointer

// called from threads changes accounts balance
void processTransaction(Transaction transaction) {
    pthread_mutex_lock(&lock); // entre critical region
    Account *account;
    // loop through accounts
    for(int i = 0; i < numAccounts; i++) {
        if(strcmp(accounts[i].name, transaction.accountName) == 0) {
            account = &accounts[i];
            break;
        }
    }

    // handle transaction from client
    if(strcmp(transaction.type, "deposit") == 0) {
        account->balance += transaction.amount;
    } else if(strcmp(transaction.type, "withdraw") == 0){
        if(account->balance - transaction.amount >= 0) {
            account->balance -= transaction.amount;
        }
    }
    pthread_mutex_unlock(&lock); // exit critical region
}

// called from main handles thread
void* clientThread(void* thread) {
    Client client = *(Client *)thread;
    for(int i = 0; i < client.numTransactions; i++) {
        Transaction transaction = client.transactions[i];
        processTransaction(transaction);
    }
    // exit thread, ex. complete
    pthread_exit(NULL);
}

int main() {
    pthread_mutex_init(&lock, NULL); // init lock for threads

    // opening file
    FILE* file = fopen("assignment_5_input.txt", "r");
    if(file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // loop through file to get account and client counts for memory allocation
    char line[1024];
    while(fgets(line, sizeof(line), file)) {
        char type = line[0];
        // check if line is account
        if(type == 'A') {
            numAccounts++;
        } else if(type == 'C') {
            numClients++;
        }
    }

    // opening file again
    FILE* file2 = fopen("assignment_5_input.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // allocate space for accounts
    accounts = (Account *)malloc(sizeof(Account) * numAccounts);
    int accountIndex = 0;
    // allocate space for clients
    clients = (Client *)malloc(sizeof(Client) * numClients);
    int clientIndex = 0;

    // read file again
    while(fgets(line, sizeof(line), file2)) {
        char type = line[0]; // first char in line
        char* token; // string being read from line

        // check if line is account (new account)
        if(type == 'A') {
            Account account; // new account

            // get account name
            char* accountName = strtok(line, " ");
            account.name = (char *)malloc(sizeof(char) * strlen(accountName));
            strcpy(account.name, accountName);
            strtok(NULL, " "); // get next token (string in line)

            // get account balance
            char *balanceStr = strtok(NULL, " ");
            int balance = atoi(balanceStr);
            account.balance = balance;

            // add account to accounts array
            accounts[accountIndex++] = account;

            // new client
        } else if(type == 'C') {
            Client client; // init new client

            // get client (c1, c2, etc)
            char* clientName = strtok(line, " ");
            client.name = (char *)malloc(sizeof(char) * strlen(clientName));
            strcpy(client.name, clientName);

            // count number for transactions
            int numTransactions = 0;
            token = strtok(NULL, " ");
            while(token != NULL) {
                Transaction transaction; // init new transaction

                // allocate space for transaction type and set
                char* transactionType = (char *)malloc(sizeof(char) * strlen(token));
                strcpy(transactionType, token);
                transaction.type = transactionType;

                // allocate space for name, and set
                char* accountName = strtok(NULL, " ");
                transaction.accountName = (char *)malloc(sizeof(char) * strlen(accountName));
                strcpy(transaction.accountName, accountName);

                // set transaction amount
                char* amount = strtok(NULL, " ");
                transaction.amount = atoi(amount);

                // add transaction to client's transaction array
                client.transactions[numTransactions++] = transaction;
                token = strtok(NULL, " "); // advance to next
            }

            // set transactions count
            client.numTransactions = numTransactions;
            // add client to clients array
            clients[clientIndex++] = client;
        }
    }

    // initialize and create threads (one per client)
    pthread_t threads[numClients];
    for(int i = 0; i < numClients; i++) {
        pthread_create(&threads[i], NULL, &clientThread, &clients[i]);
    }

    // join threads (wait for thread to finish before continuing)
    for(int i = 0; i < numClients; i++) {
        pthread_join(threads[i], NULL);
    }

    // destroy mutex lock
    pthread_mutex_destroy(&lock);

    // final output (num accounts and clients) (all account info)
    printf("\nNo. of Accounts: %d\n", numAccounts);
    printf("No. of Clients: %d\n", numClients);
    for(int i = 0; i < numAccounts; i++) {
        Account account = accounts[i];
        printf("%s balance %d\n", account.name, account.balance);
    }
    printf("\n"); // for spacing

    // close files (same one)
    fclose(file);
    fclose(file2);
    // default return (0 = succesful ex.)
    return 0;
}
