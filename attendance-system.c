#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STUDENTS 40
#define MAX_CLASSES 4
#define MAX_RECORDS 100
#define MAX_NAME_LENGTH 50

// Structures
typedef struct {
    char name[MAX_NAME_LENGTH];
    int id;
} Student;

typedef struct {
    char name[20];
    Student students[MAX_STUDENTS];
    int studentCount;
} Class;

typedef struct {
    char date[11];
    char className[20];
    char attendance[MAX_STUDENTS];
    int studentCount;
} AttendanceRecord;

// Global variables
Class classes[MAX_CLASSES];
AttendanceRecord records[MAX_RECORDS];
int recordCount = 0;

// Function declarations
void initializeClasses();
void mainMenu();
void createNewRecord();
void selectClass();
void markAttendance(char* className);
void viewEditRecords();
void selectRecordClass();
void viewRecordsByDate(char* className);
void viewRecord(AttendanceRecord* record);
void getCurrentDate(char* date);
void saveRecord(AttendanceRecord* record);
int findRecord(char* date, char* className);

// Initialize sample data
void initializeClasses() {
    // Initialize CS-A
    strcpy(classes[0].name, "CS-A");
    classes[0].studentCount = 5;
    for (int i = 0; i < classes[0].studentCount; i++) {
        sprintf(classes[0].students[i].name, "Student %d-A", i + 1);
        classes[0].students[i].id = i + 1;
    }

    // Initialize CS-B
    strcpy(classes[1].name, "CS-B");
    classes[1].studentCount = 5;
    for (int i = 0; i < classes[1].studentCount; i++) {
        sprintf(classes[1].students[i].name, "Student %d-B", i + 1);
        classes[1].students[i].id = i + 1;
    }

    // Initialize CS-C
    strcpy(classes[2].name, "CS-C");
    classes[2].studentCount = 5;
    for (int i = 0; i < classes[2].studentCount; i++) {
        sprintf(classes[2].students[i].name, "Student %d-C", i + 1);
        classes[2].students[i].id = i + 1;
    }

    // Initialize CS-D
    strcpy(classes[3].name, "CS-D");
    classes[3].studentCount = 5;
    for (int i = 0; i < classes[3].studentCount; i++) {
        sprintf(classes[3].students[i].name, "Student %d-D", i + 1);
        classes[3].students[i].id = i + 1;
    }
}

void getCurrentDate(char* date) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    sprintf(date, "%02d-%02d-%04d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

void mainMenu() {
    system("clear"); // Use "cls" for Windows
    printf("Attendance System\n");
    printf("Main Menu\n\n");
    printf("[ ] Create New Record\n");
    printf("[ ] View and edit records by date\n");
    printf("[ ] View and edit classes\n");
    printf("[ ] Exit program\n\n");
    printf(">> Press space to select and enter to continue.\n");
}

void selectClass() {
    system("clear");
    printf("Attendance System\n");
    printf("Select class\n\n");
    for (int i = 0; i < MAX_CLASSES; i++) {
        printf("[ ] %s\n", classes[i].name);
    }
    printf("\n[ ] Exit\n\n");
    printf(">> Press space to select and enter to continue.\n");
}

void markAttendance(char* className) {
    system("clear");
    printf("Attendance System [%s]\n", className);
    
    char date[11];
    getCurrentDate(date);
    printf("Date: %s\n\n", date);

    Class* selectedClass = NULL;
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(classes[i].name, className) == 0) {
            selectedClass = &classes[i];
            break;
        }
    }

    if (selectedClass == NULL) return;

    AttendanceRecord newRecord;
    strcpy(newRecord.date, date);
    strcpy(newRecord.className, className);
    newRecord.studentCount = selectedClass->studentCount;

    for (int i = 0; i < selectedClass->studentCount; i++) {
        printf("[ ] %d - %s\n", 
            selectedClass->students[i].id,
            selectedClass->students[i].name);
        newRecord.attendance[i] = 'A'; // Default to absent
    }

    printf("\n[ ] Save and Exit\n");
    printf("[ ] Discard and Exit\n\n");
    printf(">> Press space to select and enter to continue.\n");

    // In a real implementation, we would handle user input here
    // and update the attendance array accordingly
    saveRecord(&newRecord);
}

void saveRecord(AttendanceRecord* record) {
    int index = findRecord(record->date, record->className);
    if (index == -1 && recordCount < MAX_RECORDS) {
        records[recordCount] = *record;
        recordCount++;
    } else if (index != -1) {
        records[index] = *record;
    }
}

int findRecord(char* date, char* className) {
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(records[i].date, date) == 0 && 
            strcmp(records[i].className, className) == 0) {
            return i;
        }
    }
    return -1;
}

void viewEditRecords() {
    selectClass();
    // In a real implementation, after class selection:
    // viewRecordsByDate(selectedClassName);
}

void viewRecordsByDate(char* className) {
    system("clear");
    printf("View Report by date\n");
    printf("Select record\n\n");

    // Display available records for the selected class
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(records[i].className, className) == 0) {
            printf("[ ] %s\n", records[i].date);
        }
    }

    printf("\n[ ] Exit\n\n");
    printf(">> Press space to select and enter to continue.\n");
}

void viewRecord(AttendanceRecord* record) {
    system("clear");
    printf("View Report by date\n");
    printf("%s Attendance report\n", record->className);
    printf("Date: %s\n\n", record->date);

    Class* selectedClass = NULL;
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(classes[i].name, record->className) == 0) {
            selectedClass = &classes[i];
            break;
        }
    }

    if (selectedClass == NULL) return;

    for (int i = 0; i < record->studentCount; i++) {
        printf("[%c] %s\n", 
            record->attendance[i],
            selectedClass->students[i].name);
    }

    printf("\n[ ] Edit record\n");
    printf("[ ] Exit\n");
    printf("\n>> Press space to select and enter to continue.\n");
}

int main() {
    initializeClasses();
    mainMenu();
    
    // In a real implementation, we would handle user input here
    // and navigate between different screens based on user selection
    
    return 0;
}
