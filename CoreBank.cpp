
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <windows.h>
#include <sstream>
using namespace std;

void setColor(int c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

bool isValidPIN(const string& pin) {
    if (pin.length() != 4) return false;
    for (int i = 0; i < 4; ++i)
        if (!isdigit(pin[i])) return false;
    return true;
}

string getTimestamp() {
    time_t now = time(0);
    char buf[80];
    strftime(buf, 80, "%c", localtime(&now));
    return string(buf);
}

string getFilename(int id) {
    ostringstream oss;
    oss << "account_" << id << ".txt";
    return oss.str();
}

bool accountExists(int id) {
    ifstream f(getFilename(id).c_str());
    return f.good();
}

double getBalance(int id) {
    ifstream f(getFilename(id).c_str());
    string line;
    while (getline(f, line)) {
        if (line.find("Balance:") != string::npos) {
            return atof(line.substr(line.find(":") + 1).c_str());
        }
    }
    return 0.0;
}

void updateBalance(int id, double newBalance) {
    string file = getFilename(id);
    ifstream in(file.c_str());
    stringstream buffer;
    string line;
    while (getline(in, line)) {
        if (line.find("Balance:") != string::npos) {
            buffer << "Balance: " << fixed << setprecision(2) << newBalance << endl;
        } else {
            buffer << line << endl;
        }
    }
    in.close();
    ofstream out(file.c_str());
    out << buffer.str();
    out.close();
}

void createAccount() {
    int id;
    string name, pin;
    double balance;

    cout << "Enter Account ID: ";
    cin >> id;
    if (accountExists(id)) {
        setColor(12); cout << "Account already exists!\n"; setColor(7); return;
    }

    cin.ignore();
    cout << "Enter Full Name: ";
    getline(cin, name);
    cout << "Enter 4-digit PIN: ";
    cin >> pin;
    if (!isValidPIN(pin)) {
        setColor(12); cout << "Invalid PIN format.\n"; setColor(7); return;
    }
    cout << "Initial Deposit: ";
    cin >> balance;

    ofstream f(getFilename(id).c_str());
    f << "===== ACCOUNT INFO =====\n";
    f << "Name: " << name << "\n";
    f << "ID: " << id << "\n";
    f << "Balance:"  << fixed << setprecision(2) << balance << "$" "\n";
    f << "PIN: " << pin << "\n";
    f << "Created: " << getTimestamp() << "\n";
    f << "========================\n\n";
    f.close();



    setColor(10); cout << "Account created successfully!\n"; setColor(7);
}



bool verifyPIN(int id, const string& pin) {
    ifstream f(getFilename(id).c_str());
    string line;
    while (getline(f, line)) {
        if (line.find("PIN:") != string::npos) {
            string storedPIN = line.substr(line.find(":") + 2);
            return storedPIN == pin;
        }
    }
    return false;
}


void deleteAccount() {
    int id;
    string pin;
    cout << "Enter Account ID: ";
    cin >> id;
    cout << "Enter PIN: ";
    cin >> pin;

    if (!accountExists(id) || !verifyPIN(id, pin)) {
        setColor(12); cout << "Invalid ID or PIN.\n"; setColor(7); return;
    }

    remove(getFilename(id).c_str());
    remove(("pin_" + to_string(id) + ".dat").c_str());
    setColor(10); cout << "Account deleted.\n"; setColor(7);
}

void deposit() {
    int id; string pin; double amt;
    cout << "Enter ID: "; cin >> id;
    cout << "Enter PIN: "; cin >> pin;
    if (!accountExists(id) || !verifyPIN(id, pin)) {
        setColor(12); cout << "Invalid login.\n"; setColor(7); return;
    }
    cout << "Amount to deposit: "; cin >> amt;
    double bal = getBalance(id) + amt;
    updateBalance(id, bal);
    ofstream f(getFilename(id).c_str(), ios::app);
    f << "[Deposit] +$" << amt << " at " << getTimestamp() << endl;
    f.close();
    setColor(10); cout << "Deposit successful.\n"; setColor(7);
}

void withdraw() {
    int id; string pin; double amt;
    cout << "Enter ID: "; cin >> id;
    cout << "Enter PIN: "; cin >> pin;
    if (!accountExists(id) || !verifyPIN(id, pin)) {
        setColor(12); cout << "Invalid login.\n"; setColor(7); return;
    }
    cout << "Amount to withdraw: "; cin >> amt;
    double bal = getBalance(id);
    if (amt > bal) {
        setColor(12); cout << "Insufficient funds.\n"; setColor(7); return;
    }
    updateBalance(id, bal - amt);
    ofstream f(getFilename(id).c_str(), ios::app);
    f << "[Withdraw] -$" << amt << " at " << getTimestamp() << endl;
    f.close();
    setColor(10); cout << "Withdrawal successful.\n"; setColor(7);
}

void transfer() {
    int from, to; string pin; double amt;
    cout << "From ID: "; cin >> from;
    cout << "Enter PIN: "; cin >> pin;
    if (!accountExists(from) || !verifyPIN(from, pin)) {
        setColor(12); cout << "Invalid sender.\n"; setColor(7); return;
    }
    cout << "To ID: "; cin >> to;
    if (!accountExists(to)) {
        setColor(12); cout << "Receiver not found.\n"; setColor(7); return;
    }
    cout << "Amount: "; cin >> amt;
    double bal = getBalance(from);
    if (amt > bal) {
        setColor(12); cout << "Insufficient funds.\n"; setColor(7); return;
    }
    updateBalance(from, bal - amt);
    updateBalance(to, getBalance(to) + amt);
    ofstream f1(getFilename(from).c_str(), ios::app);
    f1 << "[Transfer OUT] -$" << amt << " to ID " << to << " at " << getTimestamp() << endl;
    f1.close();
    ofstream f2(getFilename(to).c_str(), ios::app);
    f2 << "[Transfer IN] +$" << amt << " from ID " << from << " at " << getTimestamp() << endl;
    f2.close();
    setColor(10); cout << "Transfer successful.\n"; setColor(7);
}

void viewAccount() {
    int id;
    cout << "Enter ID to view: ";
    cin >> id;
    if (!accountExists(id)) {
        setColor(12); cout << "Not found.\n"; setColor(7); return;
    }
    ifstream f(getFilename(id).c_str());
    string line;
    setColor(11);
    while (getline(f, line)) cout << line << endl;
    setColor(7);
}


void editAccount() {
    int id;
    string oldPIN;
    cout << "Enter Account ID to edit: ";
    cin >> id;
    if (!accountExists(id)) {
        setColor(12); cout << "Account not found.\n"; setColor(7); return;
    }

    cout << "Enter current PIN: ";
    cin >> oldPIN;
    if (!verifyPIN(id, oldPIN)) {
        setColor(12); cout << "Incorrect PIN.\n"; setColor(7); return;
    }

    cin.ignore();
    string newName, newPIN;
    cout << "Enter new full name: ";
    getline(cin, newName);
    cout << "Enter new 4-digit PIN: ";
    cin >> newPIN;
    if (!isValidPIN(newPIN)) {
        setColor(12); cout << "Invalid PIN format.\n"; setColor(7); return;
    }

    ifstream in(getFilename(id).c_str());
    stringstream updated;
    string line;
    while (getline(in, line)) {
        if (line.find("Name:") != string::npos)
            updated << "Name: " << newName << endl;
        else if (line.find("PIN:") != string::npos)
            updated << "PIN: " << newPIN << endl;
        else
            updated << line << endl;
    }
    in.close();
    ofstream out(getFilename(id).c_str());
    out << updated.str();
    out.close();

    setColor(10); cout << "Account information updated successfully.\n"; setColor(7);
}


int main() {
    setColor(11);
    cout << "\n==============================" << endl;
    cout << "      Welcome to CoreBank     " << endl;
    cout << "==============================\n" << endl;
    setColor(7);

    SetConsoleOutputCP(65001); // UTF-8 support
    int c;
    do {
        setColor(9);
        cout << "\n==== CoreBank Menu ====" << endl;
        setColor(7);
        cout << "1. Create\n2. Delete\n3. Deposit\n4. Withdraw\n5. Transfer\n6. View\n7. Edit\n0. Exit\nChoice: ";
        cin >> c;
        switch(c) {
            case 1: createAccount(); break;
            case 2: deleteAccount(); break;
            case 3: deposit(); break;
            case 4: withdraw(); break;
            case 5: transfer(); break;
            case 6: viewAccount(); break;
            case 7: editAccount(); break;
            case 0: cout << "Exiting...\n"; break;
            default: cout << "Invalid.\n";
        }
    } while (c != 0);
    return 0;
}
