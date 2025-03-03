//gcc test-2.c -I \PDCurses -L PDCurses\wincon -lpdcurses -o test-2.exe
//gcc test-2.c -lncurses -o test-2

#include "pdcurses/curses.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STUDENTS 40
#define MAX_CLASSES 4
#define MAX_RECORDS 100
#define MAX_OPTIONS 10

typedef struct {
    char name[50];
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

Class classes[MAX_CLASSES];
AttendanceRecord records[MAX_RECORDS];
int recordCount = 0;

void init_curses();
int show_menu(char *options[], int count, int startY);
void create_new_record();
void view_edit_records();
void init_sample_data();
void getCurrentDate(char *date);
int select_class();
void mark_attendance(int classIndex);
void view_records_by_date(int classIndex);
void view_record(AttendanceRecord *record, bool editMode);
void save_record(AttendanceRecord *record);
void edit_record(AttendanceRecord *record);
void delete_record(int recordIndex);
bool show_confirmation_dialog(char* date);

void init_curses() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
}

void init_sample_data() {
    char *classNames[] = {"CS-A", "CS-B", "CS-C", "CS-D"};
    for (int i = 0; i < MAX_CLASSES; i++) {
        strcpy(classes[i].name, classNames[i]);
        classes[i].studentCount = 5;
        for (int j = 0; j < classes[i].studentCount; j++) {
            sprintf(classes[i].students[j].name, "Student %d", j + 1);
            classes[i].students[j].id = j + 1;
        }
    }
}

void getCurrentDate(char *date) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    sprintf(date, "%02d-%02d-%04d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

int show_menu(char *options[], int count, int startY) {
    int selected = 0;
    int ch;

    while (1) {
        for (int i = 0; i < count; i++) {
            mvprintw(startY + i, 2, "[ ] %s", options[i]);
            if (i == selected) {
                mvprintw(startY + i, 3, "*");
            }
        }
        
        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + count) % count;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % count;
                break;
            case ' ':
                mvprintw(startY + selected, 3, "*");
                refresh();
                break;
            case '\n':
                return selected;
        }
    }
}

int select_class() {
    clear();
    mvprintw(0, 0, "Attendance System");
    mvprintw(1, 0, "Select class\n\n");

    char *options[MAX_CLASSES + 1];
    for (int i = 0; i < MAX_CLASSES; i++) {
        options[i] = classes[i].name;
    }
    options[MAX_CLASSES] = "Exit";

    return show_menu(options, MAX_CLASSES + 1, 3);
}

void mark_attendance(int classIndex) {
    if (classIndex < 0 || classIndex >= MAX_CLASSES) return;

    clear();
    char date[11];
    getCurrentDate(date);

    mvprintw(0, 0, "Attendance System [%s]", classes[classIndex].name);
    mvprintw(1, 0, "Date: %s\n\n", date);

    AttendanceRecord newRecord;
    strcpy(newRecord.date, date);
    strcpy(newRecord.className, classes[classIndex].name);
    newRecord.studentCount = classes[classIndex].studentCount;

    for (int i = 0; i < classes[classIndex].studentCount; i++) {
        newRecord.attendance[i] = 'A';
    }

    int selected = 0;
    int ch;

    while (1) {
        for (int i = 0; i < classes[classIndex].studentCount; i++) {
            mvprintw(3 + i, 0, "[ ]");
            mvprintw(3 + i, 4, "[%c] %d - %s", 
                    newRecord.attendance[i], 
                    classes[classIndex].students[i].id,
                    classes[classIndex].students[i].name);
            if (i == selected) {
                mvprintw(3 + i, 1, "*");
            }
        }
    
        mvprintw(3 + classes[classIndex].studentCount, 0, "[ ]");
        mvprintw(3 + classes[classIndex].studentCount, 4, "Save and Exit");
        mvprintw(4 + classes[classIndex].studentCount, 0, "[ ]");
        mvprintw(4 + classes[classIndex].studentCount, 4, "Discard and Exit");

        if (selected == classes[classIndex].studentCount) {
            mvprintw(3 + classes[classIndex].studentCount, 1, "*");
        }
        if (selected == classes[classIndex].studentCount + 1) {
            mvprintw(4 + classes[classIndex].studentCount, 1, "*");
        }

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + classes[classIndex].studentCount + 2) % (classes[classIndex].studentCount + 2);
                break;
            case KEY_DOWN:
                selected = (selected + 1) % (classes[classIndex].studentCount + 2);
                break;
            case ' ': 
                if (selected < classes[classIndex].studentCount) {
                    newRecord.attendance[selected] = (newRecord.attendance[selected] == 'P') ? 'A' : 'P';
                }
                break;
            case '\n':
                if (selected == classes[classIndex].studentCount) {
                    save_record(&newRecord);
                    return;
                } else if (selected == classes[classIndex].studentCount + 1) {
                    return;
                }
                break;
        }
    }
}

void save_record(AttendanceRecord *record) {
    if (recordCount < MAX_RECORDS) {
        records[recordCount] = *record;
        recordCount++;
    }
}

void view_records_by_date(int classIndex) {
    if (classIndex < 0 || classIndex >= MAX_CLASSES) return;

    clear();
    mvprintw(0, 0, "View Report by date");
    mvprintw(1, 0, "Select record\n\n");

    int classRecordCount = 0;
    int recordIndices[MAX_RECORDS];
    char *options[MAX_RECORDS + 1];
    
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(records[i].className, classes[classIndex].name) == 0) {
            options[classRecordCount] = records[i].date;
            recordIndices[classRecordCount] = i;
            classRecordCount++;
        }
    }
    options[classRecordCount] = "Exit";

    int selected = show_menu(options, classRecordCount + 1, 3);
    if (selected < classRecordCount) {
        view_record(&records[recordIndices[selected]], false);
    }
}

void edit_record(AttendanceRecord *record) {
    clear();
    mvprintw(0, 0, "Edit Attendance Record [%s]", record->className);
    mvprintw(1, 0, "Date: %s\n\n", record->date);

    int classIndex = -1;
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(classes[i].name, record->className) == 0) {
            classIndex = i;
            break;
        }
    }

    if (classIndex == -1) return;

    AttendanceRecord tempRecord = *record;
    int selected = 0;
    int ch;

    while (1) {
        for (int i = 0; i < tempRecord.studentCount; i++) {
            mvprintw(3 + i, 0, "[ ]");
            mvprintw(3 + i, 4, "[%c] %d - %s", 
                    tempRecord.attendance[i], 
                    classes[classIndex].students[i].id,
                    classes[classIndex].students[i].name);
            if (i == selected) {
                mvprintw(3 + i, 1, "*");
            }
        }
        
        // Display Save and Exit options
        mvprintw(3 + tempRecord.studentCount, 0, "[ ]");
        mvprintw(3 + tempRecord.studentCount, 4, "[ ] Save and Exit");
        mvprintw(4 + tempRecord.studentCount, 0, "[ ]");
        mvprintw(4 + tempRecord.studentCount, 4, "[ ] Discard and Exit");

        if (selected == tempRecord.studentCount) {
            mvprintw(3 + tempRecord.studentCount, 1, "*");
        }
        if (selected == tempRecord.studentCount + 1) {
            mvprintw(4 + tempRecord.studentCount, 1, "*");
        }

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + tempRecord.studentCount + 2) % (tempRecord.studentCount + 2);
                break;
            case KEY_DOWN:
                selected = (selected + 1) % (tempRecord.studentCount + 2);
                break;
            case ' ': // Toggle attendance
                if (selected < tempRecord.studentCount) {
                    tempRecord.attendance[selected] = (tempRecord.attendance[selected] == 'P') ? 'A' : 'P';
                }
                break;
            case '\n':
                if (selected == tempRecord.studentCount) { // Save and Exit
                    *record = tempRecord; // Copy back the edited data
                    return;
                } else if (selected == tempRecord.studentCount + 1) { // Discard and Exit
                    return;
                }
                break;
        }
    }
}

void view_record(AttendanceRecord *record, bool editMode) {
    if (editMode) {
        edit_record(record);
        return;
    }

    clear();
    mvprintw(0, 0, "View Report by date");
    mvprintw(1, 0, "%s Attendance report", record->className);
    mvprintw(2, 0, "Date: %s\n\n", record->date);

    int classIndex = -1;
    // Find corresponding class
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(classes[i].name, record->className) == 0) {
            classIndex = i;
            break;
        }
    }

    if (classIndex == -1) return;

    char *options[record->studentCount + 3];  
    
    for (int i = 0; i < record->studentCount; i++) {
        char *option = (char *)malloc(100);
        sprintf(option, "[%c] %d - %s", 
                record->attendance[i],
                classes[classIndex].students[i].id,
                classes[classIndex].students[i].name);
        options[i] = option;
    }
    options[record->studentCount] = "Edit record";
    options[record->studentCount + 1] = "Delete record";  
    options[record->studentCount + 2] = "Exit";

    int selected = show_menu(options, record->studentCount + 3, 4);
    
    for (int i = 0; i < record->studentCount; i++) {
        free(options[i]);
    }

    if (selected == record->studentCount) {
        view_record(record, true);
    }
    else if (selected == record->studentCount + 1) {  
        int recordIndex = -1;
        
        for (int i = 0; i < recordCount; i++) {
            if (strcmp(records[i].date, record->date) == 0 && 
                strcmp(records[i].className, record->className) == 0) {
                recordIndex = i;
                break;
            }
        }
        
        if (recordIndex != -1 && show_confirmation_dialog(record->date)) {
            delete_record(recordIndex);
            return;
        }
        else {
            view_record(record, false);  
        }
    }
}

void create_new_record() {
    int classIndex = select_class();
    if (classIndex < MAX_CLASSES) {
        mark_attendance(classIndex);
    }
}

void view_edit_records() {
    int classIndex = select_class();
    if (classIndex < MAX_CLASSES) {
        view_records_by_date(classIndex);
    }
}   
void delete_record(int recordIndex) {
    for (int i = recordIndex; i < recordCount - 1; i++) {
        records[i] = records[i + 1];
    }
    recordCount--;
}
bool show_confirmation_dialog(char* date) {
    clear();
    mvprintw(0, 0, "Delete Record");
    mvprintw(2, 0, "Are you sure you want to delete the record for %s?", date);

    char *options[] = {"Yes", "No"};
    int choice = show_menu(options, 2, 4);
    
    return choice == 0;  
}
void view_edit_classes() {
    clear();
    mvprintw(0, 0, "View and Edit Classes");

    char *options[] = {
        "Create New Class",
        "Edit Classes",
        "Exit"
    };

    int selected = show_menu(options, 3, 2);
    switch (selected) {
        case 0:
            create_new_class();
            break;
        case 1:
            edit_classes();
            break;
        case 2:
            return;
    }
}

void create_new_class() {
    clear();
    mvprintw(0, 0, "Create New Class");

    if (classCount >= MAX_CLASSES) {
        mvprintw(2, 0, "Maximum class limit reached.");
        getch();
        return;
    }

    char className[20];
    mvprintw(2, 0, "Enter class name: ");
    echo();
    getstr(className);
    noecho();

    // Check if class already exists
    for (int i = 0; i < classCount; i++) {
        if (strcmp(classes[i].name, className) == 0) {
            mvprintw(4, 0, "Class already exists!");
            getch();
            return;
        }
    }

    Class newClass;
    strcpy(newClass.name, className);
    newClass.studentCount = 0;

    mvprintw(4, 0, "Enter student details (ID and Name). Press ENTER on empty ID to stop:");
    for (int i = 0; i < MAX_STUDENTS; i++) {
        char idStr[10];
        char name[50];

        mvprintw(6 + i, 0, "ID: ");
        echo();
        getstr(idStr);
        noecho();
        if (strlen(idStr) == 0) break;

        mvprintw(6 + i, 20, "Name: ");
        echo();
        getstr(name);
        noecho();

        newClass.students[i].id = atoi(idStr);
        strcpy(newClass.students[i].name, name);
        newClass.studentCount++;
    }

    classes[classCount] = newClass;
    classCount++;

    save_class(&newClass);
    save_classes();
}

void edit_classes() {
    clear();
    mvprintw(0, 0, "Edit Classes");

    if (classCount == 0) {
        mvprintw(2, 0, "No classes exist.");
        getch();
        return;
    }

    char *options[classCount + 1];
    for (int i = 0; i < classCount; i++) {
        options[i] = classes[i].name;
    }
    options[classCount] = "Exit";

    int selected = show_menu(options, classCount + 1, 2);
    if (selected == classCount) return;

    Class *class = &classes[selected];

    clear();
    mvprintw(0, 0, "Edit Class: %s", class->name);

    // Display current students
    for (int i = 0; i < class->studentCount; i++) {
        mvprintw(2 + i, 0, "Student %d: ID %d, Name %s", 
                 i + 1, class->students[i].id, class->students[i].name);
    }

    mvprintw(2 + class->studentCount + 2, 0, "Press any key to start editing, ESC to cancel");
    int ch = getch();
    if (ch == 27) return; // ESC key

    // Edit students
    for (int i = 0; i < class->studentCount; i++) {
        char idStr[10], name[50];

        mvprintw(4 + i, 0, "Edit Student %d", i + 1);
        
        mvprintw(5 + i, 0, "Current ID (%d): ", class->students[i].id);
        echo();
        getstr(idStr);
        noecho();

        mvprintw(6 + i, 0, "Current Name (%s): ", class->students[i].name);
        echo();
        getstr(name);
        noecho();

        if (strlen(idStr) > 0) {
            class->students[i].id = atoi(idStr);
        }
        if (strlen(name) > 0) {
            strcpy(class->students[i].name, name);
        }
    }

    save_class(class);
    save_classes();
}

int main() {
    init_curses();
    init_sample_data();

    char *main_options[] = {
        "Create New Record",
        "View and edit records by date",
        "View and edit classes",
        "Exit program"
    };

    while (1) {
        clear();
        mvprintw(0, 0, "Attendance System");
        mvprintw(1, 0, "Main Menu\n\n");

        int choice = show_menu(main_options, 4, 3);

        switch (choice) {
            case 0:
                create_new_record();
                break;
            case 1:
                view_edit_records();
                break;
            case 2:
                // TODO: Implement class editing
                break;
            case 3:
                endwin();
                return 0;
        }
    }

    endwin();
    return 0;
}
