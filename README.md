# Unix Shell Implementation - Operating Systems Course

## Project Overview

This project implements a custom Unix shell as part of the Operating Systems course.
The shell simulates a loan application system with user and manager interfaces.

## Features

1. **Main Shell**: Handles user interactions for loan applications.
   - GetLoan: Submit a new loan application
   - CheckStatus: Check the status of an existing application
   - ApproveLoan: Approve a loan offer
   - MLogin: Manager login
   - exit: Exit the shell

2. **Manager Shell**: Provides administrative functions for loan management.
   - ReqNumShow: Display total number of requests
   - ShowAReqNum: Show number of approved requests
   - ShowUnAReqNum: Show number of unapproved requests
   - RequestListCheck: Check number of pending requests
   - ShowReqDetails: Display details of a specific request
   - CriteriaShow: Show loan approval criteria
   - UpdateCriteria: Update loan approval criteria
   - ApproveReqLoan: Approve a loan request
   - UnApproveReqLoan: Reject a loan request
   - LogOut: Return to Main Shell

## Implementation Details

- Developed in C for Unix (Ubuntu) environment
- Uses Unix system calls for file operations and process management
- Implements file-based data storage for user information and loan requests

## Project Structure

- `Main_shell.c`: Main shell implementation
- `Manager_shell.c`: Manager shell implementation
- `Sys_shell.sh`: Script for compiling and running the shell
- Custom command files (for specially marked commands)

## Setup and Execution

1. Ensure you have a Unix-like environment (i used Ubuntu)
2. Clone the repository or download as ZIP
3. Run the `Sys_shell.sh`

## Notes

- This project demonstrates the use of Unix system calls and shell programming
- It simulates a basic loan management system for educational purposes
