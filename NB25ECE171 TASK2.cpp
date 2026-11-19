#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// Base Class
class Student {
protected:
    int rollNo;
    string name;

public:
    Student(int r, const string& n) : rollNo(r), name(n) {}
    virtual ~Student() = default;

    int getRollNo() const { return rollNo; }
    string getName() const { return name; }

    // Virtual functions for runtime polymorphism
    virtual double getPercentage() const = 0;
    virtual string serialize() const = 0;

    virtual void display() const {
        cout << left << setw(8) << rollNo
             << setw(20) << name;
    }
};

// Derived Class
class Result : public Student {
private:
    double sub1, sub2, sub3;
    double totalMarks;
    double percentage;
    string status;

    static void validateMark(double mark, const string& subject) {
        if (mark < 0.0 || mark > 100.0) {
            throw out_of_range("Invalid mark (" + to_string(mark) + ") in " +
                               subject + ". Marks must be between 0 and 100.");
        }
    }

    void calculateResult() {
        totalMarks = sub1 + sub2 + sub3;
        percentage = totalMarks / 3.0;
        // Student passes only if they score at least 35 in every subject
        status = (sub1 >= 35.0 && sub2 >= 35.0 && sub3 >= 35.0) ? "PASS" : "FAIL";
    }

public:
    Result(int r, const string& n, double m1, double m2, double m3)
        : Student(r, n) {
        validateMark(m1, "Subject 1");
        validateMark(m2, "Subject 2");
        validateMark(m3, "Subject 3");

        sub1 = m1;
        sub2 = m2;
        sub3 = m3;
        calculateResult();
    }

    double getPercentage() const override {
        return percentage;
    }

    void display() const override {
        Student::display();
        cout << fixed << setprecision(2)
             << setw(8) << sub1
             << setw(8) << sub2
             << setw(8) << sub3
             << setw(10) << totalMarks
             << setw(11) << (to_string(percentage).substr(0, 5) + "%")
             << status << '\n';
    }

    string serialize() const override {
        ostringstream oss;
        oss << rollNo << "," << name << "," << sub1 << "," << sub2 << "," << sub3;
        return oss.str();
    }
};

// Write all records to a text file
void saveRecordsToFile(const vector<unique_ptr<Student>>& records, const string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw runtime_error("File Error: Unable to open '" + filename + "' for writing.");
    }

    for (const auto& record : records) {
        outFile << record->serialize() << '\n';
        if (outFile.fail()) {
            throw runtime_error("File Error: Failed while writing data to '" + filename + "'.");
        }
    }
}

// Read all records from a text file
vector<unique_ptr<Student>> loadRecordsFromFile(const string& filename) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        throw runtime_error("File Error: Unable to open '" + filename + "' for reading.");
    }

    vector<unique_ptr<Student>> loadedRecords;
    string line;
    int lineNumber = 0;

    while (getline(inFile, line)) {
        ++lineNumber;
        if (line.empty()) continue;

        stringstream ss(line);
        string rollStr, name, m1Str, m2Str, m3Str;

        if (!getline(ss, rollStr, ',') ||
            !getline(ss, name, ',') ||
            !getline(ss, m1Str, ',') ||
            !getline(ss, m2Str, ',') ||
            !getline(ss, m3Str, ',')) {
            cerr << "Warning: Malformed record at line " << lineNumber << " skipped.\n";
            continue;
        }

        try {
            int roll = stoi(rollStr);
            double m1 = stod(m1Str);
            double m2 = stod(m2Str);
            double m3 = stod(m3Str);
            loadedRecords.push_back(make_unique<Result>(roll, name, m1, m2, m3));
        } catch (const exception& e) {
            cerr << "Warning: Skipping invalid record at line " << lineNumber
                 << " (" << e.what() << ")\n";
        }
    }

    return loadedRecords;
}

void printTableHeader() {
    cout << "\n" << left
         << setw(8) << "Roll"
         << setw(20) << "Name"
         << setw(8) << "Sub1"
         << setw(8) << "Sub2"
         << setw(8) << "Sub3"
         << setw(10) << "Total"
         << setw(11) << "Percent"
         << "Status\n";
    cout << string(79, '-') << '\n';
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    const string filename = "student_results.txt";
    vector<unique_ptr<Student>> students;

    // Seed initial records and persist them so the file exists on first run
    try {
        students.push_back(make_unique<Result>(101, "Aarav Sharma", 88.0, 92.5, 85.0));
        students.push_back(make_unique<Result>(102, "Diya Nair", 74.0, 79.5, 81.0));
        students.push_back(make_unique<Result>(103, "Rohan Verma", 95.0, 91.0, 94.0));
        students.push_back(make_unique<Result>(104, "Meera Iyer", 48.0, 31.0, 55.0));

        saveRecordsToFile(students, filename);
        students = loadRecordsFromFile(filename);
    } catch (const exception& e) {
        cerr << "Initialization Error: " << e.what() << '\n';
        return 1;
    }

    int choice = 0;
    while (true) {
        cout << "\n=== Student Result Management System ===\n"
             << "1. Add Student Record\n"
             << "2. Display All Records (from File)\n"
             << "3. Filter Students Above Percentage Threshold\n"
             << "4. Exit\n"
             << "Enter choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }

        if (choice == 1) {
            int roll;
            string name;
            double m1, m2, m3;

            cout << "Enter Roll Number: ";
            if (!(cin >> roll)) {
                cout << "Invalid roll number.\n";
                clearInputBuffer();
                continue;
            }
            clearInputBuffer();

            cout << "Enter Student Name: ";
            getline(cin, name);

            cout << "Enter Marks for 3 Subjects (0-100): ";
            if (!(cin >> m1 >> m2 >> m3)) {
                cout << "Invalid numeric input for marks.\n";
                clearInputBuffer();
                continue;
            }

            try {
                students.push_back(make_unique<Result>(roll, name, m1, m2, m3));
                saveRecordsToFile(students, filename);
                cout << "Record validated and saved to '" << filename << "'.\n";
            } catch (const out_of_range& e) {
                cout << "[Validation Error] " << e.what() << '\n';
            } catch (const runtime_error& e) {
                cout << "[File Error] " << e.what() << '\n';
            }

        } else if (choice == 2) {
            try {
                students = loadRecordsFromFile(filename);
                if (students.empty()) {
                    cout << "No student records found.\n";
                } else {
                    printTableHeader();
                    for (const auto& s : students) {
                        s->display();
                    }
                }
            } catch (const runtime_error& e) {
                cout << "[File Error] " << e.what() << '\n';
            }

        } else if (choice == 3) {
            double threshold;
            cout << "Enter minimum percentage threshold: ";
            if (!(cin >> threshold)) {
                cout << "Invalid percentage input.\n";
                clearInputBuffer();
                continue;
            }

            // STL algorithm with lambda expression to count and display matching students
            auto aboveThreshold = [threshold](const unique_ptr<Student>& s) {
                return s->getPercentage() > threshold;
            };

            int matchCount = count_if(students.begin(), students.end(), aboveThreshold);

            if (matchCount == 0) {
                cout << "No students found with percentage greater than " << threshold << "%.\n";
            } else {
                cout << "\nStudents scoring above " << threshold << "% (" << matchCount << " found):";
                printTableHeader();
                for_each(students.begin(), students.end(), [&](const unique_ptr<Student>& s) {
                    if (aboveThreshold(s)) {
                        s->display();
                    }
                });
            }

        } else if (choice == 4) {
            cout << "Exiting program.\n";
            break;
        } else {
            cout << "Invalid option. Choose between 1 and 4.\n";
        }
    }

    return 0;
}