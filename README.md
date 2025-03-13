# C++ Library Management System

## Operations
### For All Users
- **Borrow/Return Books**  
  Students (3 books max), Faculty (5 books max)
- **View History** - Full borrowing timeline with due/return dates
- **Pay Fines** - ₹10/day overdue
- **List Books** - Filter by availability

### Librarian Exclusive
- **Manage Books** - Add/remove, change status (Available/Borrowed/Reserved)
- **Manage Users** - Add/remove Students/Faculty/Librarians
- **System Oversight** - View all books/users with detailed statuses

---

## Running the Project

### Using Makefile
##### 
Go into the directory you extracted from the zip file and then run the required commands

```sh
cd <Directory_name>
make       # Compile the project
./final     # Run the program
make clean # Remove compiled files
```
## Data Files
### books.txt
```ISBN,Title,Author,Publisher,Year,Status ```

For Example:
```sh
ISBN101,Book 1,Author 1,Publisher 1,2001,Available
ISBN109,Book 9,Author 9,Publisher 9,2009,Reserved
```
### users.txt
```UserType|Name|ID|BooksBorrowed;ISBN,IssueTimestamp;...|DueDates;ReturnDates ```

For Example:
```sh
Student|John Doe|1001|1;ISBN101,1672400000|2023-12-25|Not Returned
Faculty|Dr. Alice|2001|2;ISBN103,1672400000;ISBN105,1679000000|2023-12-25|2024-01-03
Librarian|Mr. Pikachu|3001||
```
