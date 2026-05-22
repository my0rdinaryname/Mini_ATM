#include <iostream>
#include <string>
#include <vector>
#include <functional>   // for std::hash
#include <limits>       // for cin.ignore
#include <ctime>        // for timestamps

using namespace std;

// ─────────────────────────────────────────────
//  Utility: simple hash for PIN (not cryptographic,
//  but demonstrates awareness; use bcrypt in production)
// ─────────────────────────────────────────────
size_t hashPIN(int pin) {
    return hash<int>{}(pin);
}

void clearScreen() {
    cout << "\033[2J\033[H";   // ANSI escape — portable, no system()
}

void pause() {
    cout << "\n[Press Enter to continue...]";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ─────────────────────────────────────────────
//  ATM Class
// ─────────────────────────────────────────────
class ATM {
private:
    long long     accountNo;
    string        name;
    size_t        hashedPIN;          // PIN stored as hash, never plain int
    double        balance;
    string        mobileNo;
    vector<string> transactionHistory;
    bool          isLocked   = false;
    int           pinAttempts = 0;

    static const int    MAX_ATTEMPTS      = 3;
    static const int    WITHDRAW_LIMIT    = 20000;
    static const int    LOCKOUT_SECONDS   = 30;

    string timestamp() {
        time_t now = time(nullptr);
        string ts  = ctime(&now);
        ts.pop_back();             // remove trailing newline
        return ts;
    }

    void log(const string& entry) {
        transactionHistory.push_back("[" + timestamp() + "] " + entry);
    }

public:
    // ── Constructor (proper OOP — no setData needed) ──────────────────
    ATM(long long accNo, string nm, int pin, double bal, string mob)
        : accountNo(accNo), name(nm), hashedPIN(hashPIN(pin)),
          balance(bal), mobileNo(mob) {}

    // ── Getters ───────────────────────────────────────────────────────
    long long            getAccountNo()         const { return accountNo; }
    string               getName()              const { return name; }
    double               getBalance()           const { return balance; }
    string               getMobileNo()          const { return mobileNo; }
    const vector<string>& getTransactionHistory() const { return transactionHistory; }
    bool                 getLockStatus()        const { return isLocked; }

    bool verifyPIN(int pin) const {
        return hashPIN(pin) == hashedPIN;
    }

    // ── Authentication helpers ────────────────────────────────────────
    void trackPinAttempts() {
        pinAttempts++;
        if (pinAttempts >= MAX_ATTEMPTS) lockAccount();
    }

    void resetPinAttempts() { pinAttempts = 0; }

    void lockAccount() {
        isLocked = true;
        log("Account locked after " + to_string(MAX_ATTEMPTS) + " failed attempts.");
        cout << "\nAccount locked due to multiple incorrect PIN attempts.\n";
    }

    void unlockAccount() {
        isLocked    = false;
        pinAttempts = 0;
        log("Account unlocked.");
        cout << "\nAccount unlocked. Please login again.\n";
    }

    // ── Transactions ──────────────────────────────────────────────────
    void deposit(double amount) {
        if (amount <= 0) {
            cout << "\nInvalid deposit amount.";
            return;
        }
        balance += amount;
        log("Deposited: ₹" + to_string(amount) + " | Balance: ₹" + to_string(balance));
        cout << "\nDeposit successful! New Balance: ₹" << balance;
    }

    void withdraw(double amount) {
        if (amount <= 0) {
            cout << "\nInvalid amount entered.";
        } else if (amount > WITHDRAW_LIMIT) {
            cout << "\nWithdrawal limit is ₹" << WITHDRAW_LIMIT << " per transaction.";
        } else if (amount > balance) {
            cout << "\nInsufficient balance.";
        } else {
            balance -= amount;
            log("Withdrew: ₹" + to_string(amount) + " | Balance: ₹" + to_string(balance));
            cout << "\nPlease collect your cash.";
            cout << "\nRemaining Balance: ₹" << balance;
        }
    }

    void updateMobile(const string& oldMob, const string& newMob) {
        if (oldMob != mobileNo) {
            cout << "\nIncorrect current mobile number.";
            log("Failed mobile update attempt.");
            return;
        }
        if (newMob.length() != 10) {
            cout << "\nInvalid mobile number (must be 10 digits).";
            return;
        }
        mobileNo = newMob;
        log("Mobile number updated to: " + newMob);
        cout << "\nMobile number updated successfully.";
    }
};

// ─────────────────────────────────────────────
//  Input helpers (guards against bad cin state)
// ─────────────────────────────────────────────
long long readLongLong(const string& prompt) {
    long long val;
    while (true) {
        cout << prompt;
        if (cin >> val) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return val; }
        cout << "Invalid input. Please enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int readInt(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return val; }
        cout << "Invalid input. Please enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readDouble(const string& prompt) {
    double val;
    while (true) {
        cout << prompt;
        if (cin >> val) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return val; }
        cout << "Invalid input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readString(const string& prompt) {
    string val;
    cout << prompt;
    cin >> val;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return val;
}

// ─────────────────────────────────────────────
//  Main menu (shown after successful login)
// ─────────────────────────────────────────────
void showMenu(ATM& user) {
    bool sessionActive = true;

    while (sessionActive) {
        if (user.getLockStatus()) {
            cout << "\nAccount is locked. Please contact support.\n";
            return;
        }

        clearScreen();
        cout << "\n===== ATM MENU =====";
        cout << "\nWelcome, " << user.getName();
        cout << "\n1. Check Balance";
        cout << "\n2. Deposit Cash";
        cout << "\n3. Withdraw Cash";
        cout << "\n4. Show Account Details";
        cout << "\n5. Update Mobile Number";
        cout << "\n6. View Transaction History";
        cout << "\n7. Lock Account";
        cout << "\n8. Logout\n";

        int choice = readInt("Select option: ");

        switch (choice) {
            case 1:
                cout << "\nBalance: ₹" << user.getBalance();
                pause();
                break;

            case 2: {
                double amt = readDouble("Enter deposit amount: ₹");
                user.deposit(amt);
                pause();
                break;
            }

            case 3: {
                double amt = readDouble("Enter withdrawal amount: ₹");
                user.withdraw(amt);
                pause();
                break;
            }

            case 4:
                cout << "\n--- Account Details ---";
                cout << "\nAccount No : " << user.getAccountNo();
                cout << "\nName       : " << user.getName();
                cout << "\nBalance    : ₹" << user.getBalance();
                cout << "\nMobile     : " << user.getMobileNo();
                pause();
                break;

            case 5: {
                string oldMob = readString("Enter current mobile number: ");
                string newMob = readString("Enter new mobile number   : ");
                user.updateMobile(oldMob, newMob);
                pause();
                break;
            }

            case 6: {
                const vector<string>& hist = user.getTransactionHistory();
                cout << "\n--- Transaction History ---";
                if (hist.empty()) cout << "\nNo transactions yet.";
                for (const string& t : hist) cout << "\n" << t;
                pause();
                break;
            }

            case 7:
                user.lockAccount();
                pause();
                sessionActive = false;   // force logout after manual lock
                break;

            case 8:
                cout << "\nLogged out successfully. Thank you, " << user.getName() << "!";
                pause();
                sessionActive = false;
                break;

            default:
                cout << "\nInvalid choice. Please select 1–8.";
                pause();
        }
    }
}

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    // Multi-user support — extend vector to add more accounts
    vector<ATM> accounts = {
        ATM(987654321012LL, "Kriti Goyal",  4321, 75000, "9216932999"),
        ATM(123456789012LL, "Hardik Sharma", 1234, 50000, "9370054900"),
        ATM(112233445566LL, "Rahul Verma",   9876, 30000, "9988776655")
    };

    bool running = true;

    while (running) {
        clearScreen();
        cout << "\n====== Welcome to SecureATM ======\n";

        long long enteredAccNo = readLongLong("Enter Account Number: ");
        int       enteredPIN   = readInt(     "Enter PIN           : ");

        ATM* currentUser = nullptr;
        for (ATM& acc : accounts) {
            if (acc.getAccountNo() == enteredAccNo) {
                currentUser = &acc;
                break;
            }
        }

        if (currentUser == nullptr) {
            cout << "\nAccount not found.";
            pause();
            continue;
        }

        if (currentUser->getLockStatus()) {
            cout << "\nThis account is locked. Please contact bank support.";
            pause();
            continue;
        }

        if (currentUser->verifyPIN(enteredPIN)) {
            currentUser->resetPinAttempts();
            showMenu(*currentUser);
        } else {
            cout << "\nIncorrect PIN.";
            currentUser->trackPinAttempts();

            if (currentUser->getLockStatus()) {
                char retry;
                cout << "\nRetry after " << 30 << " seconds? (y/n): ";
                cin >> retry;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (retry == 'y' || retry == 'Y') {
                    cout << "Waiting 30 seconds...\n";
                    // sleep(30);  // uncomment to enable real lockout delay
                    currentUser->unlockAccount();
                } else {
                    cout << "\nExiting. Goodbye.\n";
                    running = false;
                }
            } else {
                pause();
            }
        }
    }

    return 0;
}
