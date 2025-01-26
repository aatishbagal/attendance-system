//gcc test-2.c -I \PDCurses -L PDCurses\wincon -lpdcurses -o test-2.exe
//gcc test-2.c -lncurses -o test-2

#include "pdcurses/curses.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

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
void ensure_data_directory();
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
void save_class_to_csv(Class *class);
void load_class_from_csv(Class *class);
void edit_student_name(Class *class, int studentIndex);
void view_edit_classes();
bool show_confirmation_dialog(char* date);
void add_student(Class *class);

void init_curses() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
}

void ensure_data_directory() {
    #ifdef _WIN32
        _mkdir("data");
    #else
        mkdir("data/", 0755);
    #endif
}

void init_sample_data() {
    ensure_data_directory();
    
    char *classNames[] = {"CS-A", "CS-B", "CS-C", "CS-D"};
    for (int i = 0; i < MAX_CLASSES; i++) {
        strcpy(classes[i].name, classNames[i]);
        classes[i].studentCount = 5;
        for (int j = 0; j < classes[i].studentCount; j++) {
            sprintf(classes[i].students[j].name, "Student %d", j + 1);
            classes[i].students[j].id = j + 1;
        }
        save_class_to_csv(&classes[i]);
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
void save_class_to_csv(Class *class) {
    char filename[100];
    sprintf(filename, "data/%s.csv", class->name);
    
    FILE *file = fopen(filename, "w");
    if (!file) return;
    
    fprintf(file, "ID,Name\n");
    for (int i = 0; i < class->studentCount; i++) {
        fprintf(file, "%d,%s\n", 
                class->students[i].id, 
                class->students[i].name);
    }
    fclose(file);
}
void load_class_from_csv(Class *class) {
    char filename[100];
    sprintf(filename, "data/%s.csv", class->name);
    
    FILE *file = fopen(filename, "r");
    if (!file) return;
    
    char line[200];
    fgets(line, sizeof(line), file);
    
    class->studentCount = 0;
    while (fgets(line, sizeof(line), file) && 
           class->studentCount < MAX_STUDENTS) {
        int id;
        char name[50];
        sscanf(line, "%d,%[^\n]", &id, name);
        
        class->students[class->studentCount].id = id;
        strcpy(class->students[class->studentCount].name, name);
        class->studentCount++;
    }
    fclose(file);
}
void edit_student_name(Class *class, int studentIndex) {
    clear();
    mvprintw(0, 0, "Edit Student Name");
    mvprintw(2, 0, "Current Name: %s", 
             class->students[studentIndex].name);
    
    mvprintw(4, 0, "New Name: ");
    echo();
    curs_set(1);
    
    char newName[50];
    getstr(newName);
    
    noecho();
    curs_set(0);
    
    if (strlen(newName) > 0) {
        strcpy(class->students[studentIndex].name, newName);
        save_class_to_csv(class);
    }
}
void view_edit_classes() {
    int classIndex = select_class();
    if (classIndex >= MAX_CLASSES) return;

    Class *selectedClass = &classes[classIndex];
    load_class_from_csv(selectedClass);

    int selected = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(0, 0, "Attendance System [%s]", selectedClass->name);
        mvprintw(1, 0, "Manage Students\n\n");

        for (int i = 0; i < selectedClass->studentCount; i++) {
            mvprintw(3 + i, 0, "[ ]");
            mvprintw(3 + i, 4, "%d - %s", 
                    selectedClass->students[i].id,
                    selectedClass->students[i].name);
            if (i == selected) {
                mvprintw(3 + i, 1, "*");
            }
        }
    
        mvprintw(3 + selectedClass->studentCount, 0, "[ ]");
        mvprintw(3 + selectedClass->studentCount, 4, "Add Student");
        mvprintw(4 + selectedClass->studentCount, 0, "[ ]");
        mvprintw(4 + selectedClass->studentCount, 4, "Exit");

        if (selected == selectedClass->studentCount) {
            mvprintw(3 + selectedClass->studentCount, 1, "*");
        }
        if (selected == selectedClass->studentCount + 1) {
            mvprintw(4 + selectedClass->studentCount, 1, "*");
        }

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + selectedClass->studentCount + 2) % (selectedClass->studentCount + 2);
                break;
            case KEY_DOWN:
                selected = (selected + 1) % (selectedClass->studentCount + 2);
                break;
            case ' ': 
                if (selected < selectedClass->studentCount) {
                    // Edit student name
                    clear();
                    mvprintw(0, 0, "Edit Student Name");
                    mvprintw(2, 0, "Current Name: %s", 
                             selectedClass->students[selected].name);
                    
                    mvprintw(4, 0, "New Name: ");
                    echo();
                    curs_set(1);
                    
                    char newName[50];
                    getstr(newName);
                    
                    noecho();
                    curs_set(0);
                    
                    if (strlen(newName) > 0) {
                        strcpy(selectedClass->students[selected].name, newName);
                        save_class_to_csv(selectedClass);
                    }
                }
                break;
            case '\n':
                if (selected == selectedClass->studentCount) {
                    add_student(selectedClass);
                } else if (selected == selectedClass->studentCount + 1) {
                    return;
                }
                break;
        }
    }
}
void add_student(Class *selectedClass) {
    if (selectedClass->studentCount >= MAX_STUDENTS) {
        return;
    }

    clear();
    mvprintw(0, 0, "Add New Student");
    mvprintw(2, 0, "Enter Student Name: ");
    
    echo();
    curs_set(1);
    char newStudentName[50];
    getstr(newStudentName);
    noecho();
    curs_set(0);

    if (strlen(newStudentName) > 0) {
        strcpy(selectedClass->students[selectedClass->studentCount].name, newStudentName);
        selectedClass->students[selectedClass->studentCount].id = selectedClass->studentCount + 1;
        selectedClass->studentCount++;
        save_class_to_csv(selectedClass);
    }
}
void save_attendance_record(AttendanceRecord *record) {
    char filename[100];
    sprintf(filename, "data/%s_attendance.csv", record->className);
    
    FILE *file = fopen(filename, "a");
    if (!file) return;
    
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Date,");
        for (int i = 0; i < record->studentCount; i++) {
            fprintf(file, "Student %d,", i+1);
        }
        fprintf(file, "\n");
    }
    
    fprintf(file, "%s,", record->date);
    for (int i = 0; i < record->studentCount; i++) {
        fprintf(file, "%c,", record->attendance[i]);
    }
    fprintf(file, "\n");
    
    fclose(file);
}


void load_attendance_records() {
    for (int i = 0; i < MAX_CLASSES; i++) {
        char filename[100];
        sprintf(filename, "data/%s_attendance.csv", classes[i].name);
        
        FILE *file = fopen(filename, "r");
        if (!file) continue;

        char line[1000];
        fgets(line, sizeof(line), file);

        while (fgets(line, sizeof(line), file) && recordCount < MAX_RECORDS) {
            char *token = strtok(line, ",");
            if (!token) continue;

            strcpy(records[recordCount].date, token);
            strcpy(records[recordCount].className, classes[i].name);
            records[recordCount].studentCount = classes[i].studentCount;

            for (int j = 0; j < classes[i].studentCount; j++) {
                token = strtok(NULL, ",");
                records[recordCount].attendance[j] = token ? token[0] : 'A';
            }

            recordCount++;
        }
        fclose(file);
    }
}

int main() {
    init_curses();
    ensure_data_directory();
    init_sample_data();
    load_attendance_records();
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
                view_edit_classes();
                break;
            case 3:
                endwin();
                return 0;
        }
    }

    endwin();
    return 0;
}
