//gcc attendance-system-pdcurses.c -I \PDCurses -L PDCurses\wincon -lpdcurses -o attendance-system-pdcurses.exe
//gcc attendance-system-pdcurses.c -lncurses -o attendance-system-pdcurses

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

// Global variables
Class classes[MAX_CLASSES];
AttendanceRecord records[MAX_RECORDS];
int recordCount = 0;

// Function declarations
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

    char *options[MAX_STUDENTS + 2];
    for (int i = 0; i < classes[classIndex].studentCount; i++) {
        options[i] = classes[classIndex].students[i].name;
        newRecord.attendance[i] = 'A';  // Default to absent
    }
    options[classes[classIndex].studentCount] = "Save and Exit";
    options[classes[classIndex].studentCount + 1] = "Discard and Exit";

    int selected;
    while (1) {
        selected = show_menu(options, classes[classIndex].studentCount + 2, 3);
        
        if (selected == classes[classIndex].studentCount) {  // Save and Exit
            save_record(&newRecord);
            break;
        } else if (selected == classes[classIndex].studentCount + 1) {  // Discard and Exit
            break;
        } else if (selected >= 0 && selected < classes[classIndex].studentCount) {
            // Toggle attendance
            newRecord.attendance[selected] = (newRecord.attendance[selected] == 'P') ? 'A' : 'P';
            mvprintw(3 + selected, 3, "%c", newRecord.attendance[selected]);
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

    // Count records for this class
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

void view_record(AttendanceRecord *record, bool editMode) {
    clear();
    mvprintw(0, 0, "View Report by date");
    mvprintw(1, 0, "%s Attendance report", record->className);
    mvprintw(2, 0, "Date: %s\n\n", record->date);

    char *options[record->studentCount + 2];
    int classIndex = -1;

    // Find corresponding class
    for (int i = 0; i < MAX_CLASSES; i++) {
        if (strcmp(classes[i].name, record->className) == 0) {
            classIndex = i;
            break;
        }
    }

    if (classIndex == -1) return;

    for (int i = 0; i < record->studentCount; i++) {
        char option[100];
        sprintf(option, "[%c] %s", record->attendance[i], classes[classIndex].students[i].name);
        options[i] = strdup(option);
    }
    options[record->studentCount] = "Edit record";
    options[record->studentCount + 1] = "Exit";

    int selected = show_menu(options, record->studentCount + 2, 4);
    
    // Free allocated memory
    for (int i = 0; i < record->studentCount; i++) {
        free(options[i]);
    }

    if (selected == record->studentCount && !editMode) {
        view_record(record, true);
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
