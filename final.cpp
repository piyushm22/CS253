#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <map>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

// Forward declarations
class User;
class Book;

time_t getCurrentTime() { return time(0); }
string timeToString(time_t t) {
    tm* timeinfo = localtime(&t);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d", timeinfo);
    return string(buffer);
}

class Book {
private:
    string title, author, publisher, ISBN;
    int year;
    string status; // Available, Borrowed, Reserved

public:
    Book(string t, string a, string p, string i, int y, string s = "Available")
        : title(t), author(a), publisher(p), ISBN(i), year(y), status(s) {}

    // Getters
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    string getISBN() const { return ISBN; }
    int getYear() const { return year; }
    string getStatus() const { return status; }

    void setStatus(const string& s) { status = s; }

    string serialize() const {
        return ISBN + "," + title + "," + author + "," + publisher + "," +
               to_string(year) + "," + status;
    }

    static Book deserialize(const string& data) {
        stringstream ss(data);
        string parts[6];
        for (int i = 0; i < 6; i++) getline(ss, parts[i], ',');
        return Book(parts[1], parts[2], parts[3], parts[0], stoi(parts[4]), parts[5]);
    }
};

class Account {
private:
    vector<pair<string, time_t>> borrowedBooks;
    vector<string> borrowingHistory;
    double fine;

public:
    Account() : fine(0.0) {}

    void addBook(const string& ISBN, time_t dueDate) {
        borrowedBooks.emplace_back(ISBN, dueDate);
        string dueDateStr = timeToString(dueDate);  
        borrowingHistory.push_back(ISBN + "|" + dueDateStr + "|Not Returned");
    }

    bool removeBook(const string& ISBN, time_t returnDate) {
        auto it = find_if(borrowedBooks.begin(), borrowedBooks.end(),
            [&ISBN](const pair<string, time_t>& b) { return b.first == ISBN; });
    
        if (it != borrowedBooks.end()) {
            time_t dueDate = it->second;
            string dueDateStr = timeToString(dueDate);  
            string returnDateStr = timeToString(returnDate);  
    
            for (string& entry : borrowingHistory) {
                stringstream ss(entry);
                string recordedISBN, storedDueDate, returned;
                getline(ss, recordedISBN, '|');
                getline(ss, storedDueDate, '|');
                getline(ss, returned);
    
                if (recordedISBN == ISBN && returned == "Not Returned") {
                    entry = ISBN + "|" + storedDueDate + "|" + returnDateStr;
                    break;
                }
            }
    
            borrowedBooks.erase(it);
    
            double daysOverdue = difftime(returnDate, dueDate) / (60 * 60 * 24);
            if (daysOverdue > 0) {
                fine += daysOverdue * 10;
                if (fine < 1.0) {  
                    fine = 0.0;
                }
            }
    
            return true;
        }
        return false;
    }     
    

    double getFine() const { return fine; }
    void payFine(double amount) { 
        fine = max(0.0, fine - amount);
        if (fine < 1.0) {  
            fine = 0.0;
        }
    }
    vector<pair<string, time_t>> getBorrowedBooks() const { return borrowedBooks; }
    vector<string> getHistory() const { return borrowingHistory; }

    string serialize() const {
        stringstream ss;
        
        // Serialize borrowed books
        ss << borrowedBooks.size() << ";";
        for (const auto& entry : borrowedBooks) {
            ss << entry.first << "," << entry.second << ";";
        }
        
        // Serialize borrowing history
        ss << borrowingHistory.size() << ";";
        for (const auto& entry : borrowingHistory) {
            ss << entry << ";";
        }
    
        // Serialize fine
        ss << fine << ";";
    
        return ss.str();
    } 

    void deserialize(const string& data) {
        borrowedBooks.clear();
        borrowingHistory.clear();
        fine = 0.0;
    
        stringstream ss(data);
        string part;
        vector<string> parts;
    
        while (getline(ss, part, ';')) {
            if (!part.empty()) parts.push_back(part);
        }
    
        size_t index = 0;
        if (parts.empty()) return;
    
        try {
            // Load borrowed books
            if (index < parts.size()) {
                int borrowedCount = stoi(parts[index++]);
                for (int i = 0; i < borrowedCount && index < parts.size(); i++) {
                    stringstream entryStream(parts[index++]);
                    string isbn, dueDateStr;
                    if (getline(entryStream, isbn, ',') && getline(entryStream, dueDateStr)) {
                        time_t dueDate = stol(dueDateStr);
                        borrowedBooks.emplace_back(isbn, dueDate);
                    }
                }
            }
    
            // Load borrowing history
            if (index < parts.size()) {
                int historyCount = stoi(parts[index++]);
                for (int i = 0; i < historyCount && index < parts.size(); i++) {
                    string entry = parts[index++];
                    
                    stringstream historyStream(entry);
                    string isbn, dueDate, returnDate;
    
                    getline(historyStream, isbn, '|');
                    getline(historyStream, dueDate, '|');
                    getline(historyStream, returnDate);
    
                    // **Fix: If dueDate is empty, extract it from borrowedBooks**
                    if (dueDate.empty() || dueDate == "Unknown") {
                        for (const auto& borrowed : borrowedBooks) {
                            if (borrowed.first == isbn) {
                                dueDate = timeToString(borrowed.second);
                                break;
                            }
                        }
                    }
    
                    // If no return date, mark as "Not Returned"
                    if (returnDate.empty()) returnDate = "Not Returned";
    
                    borrowingHistory.push_back(isbn + "|" + dueDate + "|" + returnDate);
                }
            }
    
            // Load fine
            if (index < parts.size()) {
                string fineStr = parts[index];
                if (!fineStr.empty()) fine = stod(fineStr);
            }
        } catch (const exception& e) {
            cerr << "Error loading account data: " << e.what() << endl;
            borrowedBooks.clear();
            borrowingHistory.clear();
            fine = 0.0;
        }
    }         
};

class User {
protected:
    string name;
    int id;
    Account account;

public:
    User(string n, int i) : name(n), id(i) {}
    virtual ~User() = default;

    // Getters
    string getName() const { return name; }
    int getId() const { return id; }
    Account& getAccount() { return account; }

    // Pure virtual functions
    virtual void borrowBook(Book& book) = 0;
    virtual void returnBook(Book& book) = 0;
    virtual void displayMenu() = 0;
    virtual int getMaxBooks() const = 0;
    virtual int getBorrowPeriod() const = 0;
    virtual string getType() const = 0;

    // Common functionality
    virtual bool canBorrow() const {
        return account.getFine() == 0 && 
               account.getBorrowedBooks().size() < getMaxBooks() &&
               !hasOverdueBooks();
    }

    bool hasOverdueBooks(int maxDays = 0) const {
        time_t now = getCurrentTime();
        for (const auto& entry : account.getBorrowedBooks()) {
            double daysOverdue = difftime(now, entry.second) / (60 * 60 * 24);
            if (daysOverdue > maxDays) return true;
        }
        return false;
    }
};

class Student : public User {
public:
    Student(string n, int i) : User(n, i) {}

    void borrowBook(Book& book) override {
        if (canBorrow() && book.getStatus() == "Available") {
            time_t dueDate = getCurrentTime() + getBorrowPeriod() * 24 * 60 * 60;
            book.setStatus("Borrowed");
            account.addBook(book.getISBN(), dueDate);
            cout << "Successfully borrowed: " << book.getTitle() << endl;
        } else {
            cout << "Cannot borrow book, Please check availability or your limits.\n";
        }
    }

    void returnBook(Book& book) override {
        if (account.removeBook(book.getISBN(), getCurrentTime())) {
            book.setStatus("Available");
            cout << "Successfully returned: " << book.getTitle() << endl;
        } else {
            cout << "You have not borrowed this book.\n";
        }
    }

    void displayMenu() override {
        cout << "\nStudent Menu\n1. Borrow Book\n2. Return Book\n3. View Fines\n4. Pay Fines\n5. View History\n6. Exit\nChoice: ";
    }

    int getMaxBooks() const override { return 3; }
    int getBorrowPeriod() const override { return 15; }
    string getType() const override { return "Student"; }
};

class Faculty : public User {
public:
    Faculty(string n, int i) : User(n, i) {}

    void borrowBook(Book& book) override {
        if (canBorrow() && book.getStatus() == "Available") {
            time_t dueDate = getCurrentTime() + getBorrowPeriod() * 24 * 60 * 60;
            book.setStatus("Borrowed");
            account.addBook(book.getISBN(), dueDate);
            cout << "Successfully borrowed: " << book.getTitle() << endl;
        } else {
            cout << "Cannot borrow book, please check availability or your limits.\n";
        }
    }

    void returnBook(Book& book) override {
        if (account.removeBook(book.getISBN(), getCurrentTime())) {
            book.setStatus("Available");
            cout << "Successfully returned: " << book.getTitle() << endl;
        } else {
            cout << "You have not borrowed this book.\n";
        }
    }

    bool canBorrow() const override {
        return User::canBorrow() && !hasOverdueBooks(60);
    }

    void displayMenu() override {
        cout << "\nFaculty Menu\n1. Borrow Book\n2. Return Book\n3. View History\n4. Exit\nChoice: ";
    }

    int getMaxBooks() const override { return 5; }
    int getBorrowPeriod() const override { return 30; }
    string getType() const override { return "Faculty"; }
};

class Librarian : public User {
public:
    Librarian(string n, int i) : User(n, i) {}

    void borrowBook(Book& book) override { cout << "Librarians cannot borrow books.\n"; }
    void returnBook(Book& book) override { cout << "Librarians cannot return books.\n"; }

    void displayMenu() override {
        cout << "\nLibrarian Menu\n1. Add Book\n2. Remove Book\n3. Add User\n4. Remove User\n5. View All Books\n6. Change Book Status\n7. Exit\nChoice: ";
    }

    void addBook(vector<Book>& books, const Book& newBook) {
        books.push_back(newBook);
        cout << "Added new book: " << newBook.getTitle() << endl;
    }

    void removeBook(vector<Book>& books, const string& ISBN) {
        auto it = remove_if(books.begin(), books.end(),
            [&ISBN](const Book& b) { return b.getISBN() == ISBN; });
        if (it != books.end()) {
            books.erase(it, books.end());
            cout << "Book removed successfully.\n";
        } else {
            cout << "Book not found.\n";
        }
    }

    template<typename T>
    void addUser(vector<unique_ptr<User>>& users, string name, int id) {
        users.push_back(make_unique<T>(name, id));
        cout << "User added successfully.\n";
    }

    void removeUser(vector<unique_ptr<User>>& users, int id) {
        auto it = remove_if(users.begin(), users.end(),
            [id](const unique_ptr<User>& u) { return u->getId() == id; });
        if (it != users.end()) {
            users.erase(it, users.end());
            cout << "User removed successfully.\n";
        } else {
            cout << "User not found.\n";
        }
    }

    int getMaxBooks() const override { return 0; }
    int getBorrowPeriod() const override { return 0; }
    string getType() const override { return "Librarian"; }
};

class LibrarySystem {
private:
    vector<Book> books;
    vector<unique_ptr<User>> users;
    User* currentUser = nullptr;

    void loadBooks() {
        ifstream file("books.txt");
        string line;
        while (getline(file, line)) {
            books.push_back(Book::deserialize(line));
        }
    }

    void saveBooks() {
        ofstream file("books.txt");
        for (const auto& book : books) {
            file << book.serialize() << endl;
        }
    }

    void loadUsers() {
        ifstream file("users.txt");
        if (!file) return;
        string line;
        while (getline(file, line)) {
            vector<string> userParts;
            stringstream ss(line);
            string part;
    
            while (getline(ss, part, '|')) {
                userParts.push_back(part);
            }
    
            if (userParts.size() < 4) {
                if (userParts.size() != 3) continue;
                userParts.push_back("");
            }
    
            string type = userParts[0];
            string name = userParts[1];
            int id = stoi(userParts[2]);
            string accountData = userParts[3];
    
            unique_ptr<User> user;
            if (type == "Student") user = make_unique<Student>(name, id);
            else if (type == "Faculty") user = make_unique<Faculty>(name, id);
            else if (type == "Librarian") user = make_unique<Librarian>(name, id);
            else continue;
    
            try {
                if (!accountData.empty()) {
                    user->getAccount().deserialize(accountData);
                }
            } catch (const exception& e) {
                cerr << "Error loading account for user " << id << ": " << e.what() << endl;
            }
    
            users.push_back(std::move(user));
        }
    }    

    void saveUsers() {
        ofstream file("users.txt");
        for (const auto& user : users) {
            file << user->getType() << "|" << user->getName() << "|" 
                 << user->getId() << "|" << user->getAccount().serialize() << endl;
        }
    }    

    void displayAvailableBooks() {
        cout << "\nAvailable Books:\n";
        for (const auto& book : books) {
            if (book.getStatus() == "Available") {
                cout << "ISBN: " << book.getISBN() << " | Title: " << book.getTitle() 
                     << " | Author: " << book.getAuthor() << endl;
            }
        }
    }

    void displayBorrowedBooks() {
        auto borrowed = currentUser->getAccount().getBorrowedBooks();
        if (borrowed.empty()) {
            cout << "No books currently borrowed.\n";
            return;
        }
        cout << "\nBorrowed Books:\n";
        for (const auto& entry : borrowed) {
            cout << "ISBN: " << entry.first << " | Due Date: " << timeToString(entry.second) << endl;
        }
    }

    void handleStudent(int choice) {
        switch (choice) {
            case 1: { // Borrow Book
                displayAvailableBooks();
                string isbn;
                cout << "Enter ISBN of the book to borrow: ";
                cin >> isbn;
                auto it = find_if(books.begin(), books.end(),
                    [&isbn](const Book& b) { return b.getISBN() == isbn && b.getStatus() == "Available"; });
                if (it != books.end()) {
                    currentUser->borrowBook(*it);
                } else {
                    cout << "Book not available or invalid ISBN.\n";
                }
                break;
            }
            case 2: { // Return Book
                displayBorrowedBooks();
                string isbn;
                cout << "Enter ISBN of the book to return: ";
                cin >> isbn;
                auto it = find_if(books.begin(), books.end(),
                    [&isbn](const Book& b) { return b.getISBN() == isbn; });
                if (it != books.end()) {
                    currentUser->returnBook(*it);
                } else {
                    cout << "Book not found.\n";
                }
                break;
            }
            case 3: // View Fines
                cout << "Outstanding fines: " << currentUser->getAccount().getFine() << " rupees\n";
                break;
            case 4: { // Pay Fines
                double amount;
                cout << "Enter amount to pay: ";
                cin >> amount;
                currentUser->getAccount().payFine(amount);
                cout << "Paid " << amount << " rupees. Remaining fines: " 
                     << currentUser->getAccount().getFine() << endl;
                break;
            }
            case 5: { // View History (For Students)
                auto history = currentUser->getAccount().getHistory();
                if (history.empty()) {
                    cout << "No borrowing history.\n";
                    break;
                }
                cout << "\nBorrowing History:\n";
                for (const auto& entry : history) {
                    string isbn, dueDate, returnDate;
                    stringstream ss(entry);
                    getline(ss, isbn, '|');
                    getline(ss, dueDate, '|');
                    getline(ss, returnDate);
            
                    cout << "ISBN: " << isbn 
                         << " | Due: " << (dueDate.empty() ? "Not Available" : dueDate) 
                         << " | Returned: " << returnDate << endl;
                }
                break;
            }                                    
            case 6: // Exit
                break;
            default:
                cout << "Invalid choice.\n";
        }
    }

    void handleFaculty(int choice) {
        switch (choice) {
            case 1: { // Borrow Book
                displayAvailableBooks();
                string isbn;
                cout << "Enter ISBN of the book to borrow: ";
                cin >> isbn;
                auto it = find_if(books.begin(), books.end(),
                    [&isbn](const Book& b) { return b.getISBN() == isbn && b.getStatus() == "Available"; });
                if (it != books.end()) {
                    currentUser->borrowBook(*it);
                } else {
                    cout << "Book not available or invalid ISBN.\n";
                }
                break;
            }
            case 2: { // Return Book
                displayBorrowedBooks();
                string isbn;
                cout << "Enter ISBN of the book to return: ";
                cin >> isbn;
                auto it = find_if(books.begin(), books.end(),
                    [&isbn](const Book& b) { return b.getISBN() == isbn; });
                if (it != books.end()) {
                    currentUser->returnBook(*it);
                } else {
                    cout << "Book not found.\n";
                }
                break;
            }
            case 3: { // View History (For Faculty)
                auto history = currentUser->getAccount().getHistory();
                if (history.empty()) {
                    cout << "No borrowing history.\n";
                    break;
                }
                cout << "\nBorrowing History:\n";
                for (const auto& entry : history) {
                    string isbn, dueDate, returnDate;
                    stringstream ss(entry);
                    getline(ss, isbn, '|');
                    getline(ss, dueDate, '|');
                    getline(ss, returnDate);
            
                    cout << "ISBN: " << isbn 
                         << " | Due: " << (dueDate.empty() ? "Not Available" : dueDate) 
                         << " | Returned: " << returnDate << endl;
                }
                break;
            }                                    
            case 4: // Exit
                break;
            default:
                cout << "Invalid choice.\n";
        }
    }

    void handleLibrarian(int choice) {
        Librarian* lib = dynamic_cast<Librarian*>(currentUser);
        switch (choice) {
            case 1: {
                string title, author, publisher, isbn;
                int year;
                cout << "Enter book title: ";
                cin.ignore(); getline(cin, title);
                cout << "Enter author: "; getline(cin, author);
                cout << "Enter publisher: "; getline(cin, publisher);
                cout << "Enter ISBN: "; cin >> isbn;
                cout << "Enter publication year: "; cin >> year;
                lib->addBook(books, Book(title, author, publisher, isbn, year));
                break;
            }
            case 2: {
                string isbn;
                cout << "Enter ISBN of the book to remove: ";
                cin >> isbn;
                lib->removeBook(books, isbn);
                break;
            }
            case 3: {
                int typeChoice, id;
                string name;
                cout << "Enter user type (1. Student, 2. Faculty, 3. Librarian): ";
                cin >> typeChoice;
                cout << "Enter name: ";
                cin.ignore(); getline(cin, name);
                cout << "Enter ID: "; cin >> id;
                if (typeChoice == 1) lib->addUser<Student>(users, name, id);
                else if (typeChoice == 2) lib->addUser<Faculty>(users, name, id);
                else if (typeChoice == 3) lib->addUser<Librarian>(users, name, id);
                else cout << "Invalid type.\n";
                break;
            }
            case 4: {
                int id;
                cout << "Enter user ID to remove: ";
                cin >> id;
                lib->removeUser(users, id);
                break;
            }
            case 5: {
                cout << "\nAll Books:\n";
                for (const auto& book : books) {
                    cout << "ISBN: " << book.getISBN() << " | Title: " << book.getTitle() 
                         << " | Status: " << book.getStatus() << endl;
                }
                break;
            }
            case 6: {
                string isbn, newStatus;
                cout << "Enter ISBN of the book to update: ";
                cin >> isbn;
                auto it = find_if(books.begin(), books.end(),
                    [&isbn](const Book& b) { return b.getISBN() == isbn; });
                if (it != books.end()) {
                    cout << "Enter new status : ";
                    cin >> newStatus;
                    if (newStatus == "Available" || newStatus == "Borrowed" || newStatus == "Reserved") {
                        it->setStatus(newStatus);
                        cout << "Status updated successfully.\n";
                    } else {
                        cout << "Invalid status! Use Available/Borrowed/Reserved.\n";
                    }
                } else {
                    cout << "Book not found.\n";
                }
                break;
            }
            case 7:  // Updated exit condition
                cout << "Exiting Librarian Menu...\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    }

public:
    LibrarySystem() {
        loadBooks();
        loadUsers();
    }

    ~LibrarySystem() {
        saveBooks();
        saveUsers();
    }

    void login() {
        int id;
        cout << "Enter user ID: ";
        cin >> id;

        auto it = find_if(users.begin(), users.end(),
            [id](const unique_ptr<User>& u) { return u->getId() == id; });

        if (it != users.end()) {
            currentUser = it->get();
            cout << "Welcome, " << currentUser->getName() << " (" << currentUser->getType() << ")\n";
        } else {
            cout << "User not found.\n";
        }
    }

    void run() {
        if (!currentUser) {
            cout << "Please log in first.\n";
            return;
        }

        while (true) {
            currentUser->displayMenu();
            int choice;
            cin >> choice;
            if (currentUser->getType() == "Student") handleStudent(choice);
            else if (currentUser->getType() == "Faculty") handleFaculty(choice);
            else if (currentUser->getType() == "Librarian") handleLibrarian(choice);

            if ((currentUser->getType() == "Student" && choice == 6) || 
                (currentUser->getType() == "Faculty" && choice == 4) || 
                (currentUser->getType() == "Librarian" && choice == 7))
                break;
        }
    }
};

int main() {
    LibrarySystem system;
    system.login();
    system.run();
    return 0;
}