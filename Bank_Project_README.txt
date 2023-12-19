==================================================================================================
## Bank Project - C Code ##

- Class: CSC 4420 - Operating Systems 
- Project: Mini Project - Bank System

==================================================================================================
## Overview ##
- This C program simulates a banking system, incorporating various functionalities such
  as account creation, deposits, withdrawals, balance checking, and transaction history 
  retrieval. It uses pthreads to handle different branches identified by the first digit 
  of the account ID (North, East, South, West).

==================================================================================================
## Features ##
- Create a new account with a unique ID.
- Perform deposits and withdrawals on specific accounts.
- Check the balance of a specific account ID.
- Retrieve the last 5 transactions of a specific account ID.
- Receive all transactions from the current or previous session.
- Multi-threaded design to manage different bank branches.

==================================================================================================
## Prerequisites ##
- GCC compiler
- pthread library

==================================================================================================
## Files Needed ## 
 Ensure that these files are present in the same directory as the source code (`BankProject.c`):
- `northbranch.csv`
- `eastbranch.csv`
- `southbranch.csv`
- `westbranch.csv`
- `starter.csv`
- `initial_transaction.csv`

==================================================================================================
## Compilation and Execution - Created on Linux! ##
- Compile the program using GCC with pthread support:

- Terminal Execution Example:
```bash
gcc -o BankProject BankProject.c -pthread
./BankProject
```

==================================================================================================
## Development Environment ##
- Developed on Ubuntu 22.04.3 LTS
- GCC Compiler Version: gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0

==================================================================================================
## How to Use ##
1. Start the program.
2. Choose between starting a new session (which overwrites existing data) or continuing from a previous session.
3. Follow the on-screen instructions to perform various banking operations.
4. To exit, choose the exit option from the main menu.

==================================================================================================
## Author ##
- Timothy Kosinski

## Date Finished ##
- 06 December 2023
