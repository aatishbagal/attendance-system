#include "pdcurses/curses.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_STUDENTS 40
#define MAX_CLASSES 10
#define MAX_RECORDS 100
#define MAX_FILENAME 100

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
    char studentNames[MAX_STUDENTS][50];  // Store student names for CSV
    int studentCount;
} AttendanceRecord;

Class classes[MAX_CLASSES];
int classCount = 0;
AttendanceRecord records[MAX_RECORDS];
int recordCount = 0;

void init_curses();
int show_menu(char *options[], int count, int startY);
void getCurrentDate(char *date);
void save_class(Class *class);
void load_class(const char *className, Class *class);
void load_classes();
void save_classes();
void save_attendance_record(AttendanceRecord *record);
void load_attendance_records();
void create_new_class();
void save_classes();
void load_classes();

void ensure_directory_exists(const char *path) {
    #ifdef _WIN32
        _mkdir(path);
    #else
        mkdir(path, 0755);
    #endif
}

void init_curses() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
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
        clear();
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
            case '\n':
                return selected;
            case 27:  // ESC key
                return count - 1;  // Exit option
        }
    }
}

void create_new_class() {
    clear();
    mvprintw(0, 0, "Create New Class");
    mvprintw(2, 0, "Enter Class Name: ");
    
    echo();
    curs_set(1);
    char className[20];
    getstr(className);
    noecho();
    curs_set(0);

    if (strlen(className) > 0 && classCount < MAX_CLASSES) {
        // Check if class already exists
        for (int i = 0; i < classCount; i++) {
            if (strcmp(classes[i].name, className) == 0) {
                mvprintw(4, 0, "Class already exists!");
                refresh();
                getch();
                return;
            }
        }

        // Create new class
        strcpy(classes[classCount].name, className);
        classes[classCount].studentCount = 0;
        save_class(&classes[classCount]);
        classCount++;
        save_classes();

        mvprintw(4, 0, "Class created successfully!");
        refresh();
        getch();
    }
}

void save_class(Class *class) {
    ensure_directory_exists("classes");
    char filename[100];
    sprintf(filename, "classes/%s.csv", class->name);

    FILE *file = fopen(filename, "w");
    if (!file) {
        mvprintw(0, 0, "Error saving class %s", class->name);
        refresh();
        getch();
        return;
    }

    fprintf(file, "ID,Name\n");
    for (int i = 0; i < class->studentCount; i++) {
        fprintf(file, "%d,%s\n", class->students[i].id, class->students[i].name);
    }
    fclose(file);
}

void save_attendance_record(AttendanceRecord *record) {
    ensure_directory_exists("attendance");
    char filename[100];
    sprintf(filename, "classes/%s_attendance.csv", record->className);

    FILE *file = fopen(filename, "a");  // Append mode
    if (!file) {
        mvprintw(0, 0, "Error saving attendance record");
        refresh();
        getch();
        return;
    }

    // Write header if file is empty
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Date,");
        for (int i = 0; i < record->studentCount; i++) {
            fprintf(file, "%s,", record->studentNames[i]);
        }
        fprintf(file, "\n");
    }

    // Write attendance data
    fprintf(file, "%s,", record->date);
    for (int i = 0; i < record->studentCount; i++) {
        fprintf(file, "%c,", record->attendance[i]);
    }
    fprintf(file, "\n");

    fclose(file);
}

int select_class() {
    clear();
    mvprintw(0, 0, "Select Class");

    if (classCount == 0) {
        mvprintw(2, 0, "No classes exist. Please create a class first.");
        refresh();
        getch();
        return -1;
    }

    char *options[classCount + 2];
    for (int i = 0; i < classCount; i++) {
        options[i] = classes[i].name;
    }
    options[classCount] = "Create New Class";
    options[classCount + 1] = "Exit";

    int choice = show_menu(options, classCount + 2, 2);

    if (choice == classCount) {
        create_new_class();
        return -1;
    }
    else if (choice == classCount + 1 || choice == -1) {
        return -1;
    }

    return choice;
}

void mark_attendance() {
    int classIndex = select_class();
    if (classIndex < 0) return;

    Class *selectedClass = &classes[classIndex];
    
    clear();
    char date[11];
    getCurrentDate(date);

    mvprintw(0, 0, "Mark Attendance for %s", selectedClass->name);
    mvprintw(1, 0, "Date: %s", date);

    AttendanceRecord newRecord;
    strcpy(newRecord.date, date);
    strcpy(newRecord.className, selectedClass->name);
    newRecord.studentCount = selectedClass->studentCount;

    // Initialize all as absent and copy student names
    for (int i = 0; i < newRecord.studentCount; i++) {
        newRecord.attendance[i] = 'A';
        strcpy(newRecord.studentNames[i], selectedClass->students[i].name);
    }

    int selected = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(0, 0, "Mark Attendance for %s", selectedClass->name);
        mvprintw(1, 0, "Date: %s", date);

        for (int i = 0; i < selectedClass->studentCount; i++) {
            mvprintw(3 + i, 0, "[ ] %d - %s [%c]", 
                     selectedClass->students[i].id, 
                     selectedClass->students[i].name,
                     newRecord.attendance[i]);
            
            if (i == selected) {
                mvprintw(3 + i, 1, "*");
            }
        }

        mvprintw(3 + selectedClass->studentCount + 1, 0, "[ ] Save Attendance");
        mvprintw(3 + selectedClass->studentCount + 2, 0, "[ ] Cancel");

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + selectedClass->studentCount + 3) % (selectedClass->studentCount + 3);
                break;
            case KEY_DOWN:
                selected = (selected + 1) % (selectedClass->studentCount + 3);
                break;
            case ' ':
                if (selected < selectedClass->studentCount) {
                    newRecord.attendance[selected] = 
                        (newRecord.attendance[selected] == 'P') ? 'A' : 'P';
                }
                break;
            case '\n':
                if (selected == selectedClass->studentCount) {
                    save_attendance_record(&newRecord);
                    return;
                }
                else if (selected == selectedClass->studentCount + 1) {
                    return;
                }
                break;
            case 27:  // ESC key
                return;
        }
    }
}

void edit_attendance_record(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;

    Class *selectedClass = NULL;
    for (int i = 0; i < classCount; i++) {
        if (strstr(filename, classes[i].name)) {
            selectedClass = &classes[i];
            break;
        }
    }

    if (!selectedClass) {
        fclose(file);
        return;
    }

    AttendanceRecord editRecord;
    strcpy(editRecord.className, selectedClass->name);
    editRecord.studentCount = selectedClass->studentCount;

    // Read header and current attendance
    char line[1000];
    fgets(line, sizeof(line), file);  // Header
    fgets(line, sizeof(line), file);  // Last attendance record

    char *token = strtok(line, ",");
    if (token) {
        strcpy(editRecord.date, token);
        
        for (int i = 0; i < selectedClass->studentCount; i++) {
            token = strtok(NULL, ",");
            editRecord.attendance[i] = token ? token[0] : 'A';
            strcpy(editRecord.studentNames[i], selectedClass->students[i].name);
        }
    }
    fclose(file);

    int selected = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(0, 0, "Edit Attendance Record for %s", selectedClass->name);
        mvprintw(1, 0, "Date: %s", editRecord.date);

        for (int i = 0; i < selectedClass->studentCount; i++) {
            mvprintw(3 + i, 0, "[ ] %d - %s [%c]", 
                     selectedClass->students[i].id, 
                     selectedClass->students[i].name,
                     editRecord.attendance[i]);
            
            if (i == selected) {
                mvprintw(3 + i, 1, "*");
            }
        }

        mvprintw(3 + selectedClass->studentCount + 1, 0, "[ ] Save Changes");
        mvprintw(3 + selectedClass->studentCount + 2, 0, "[ ] Cancel");

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + selectedClass->studentCount + 3) % (selectedClass->studentCount + 3);
                break;
            case KEY_DOWN:
                selected = (selected + 1) % (selectedClass->studentCount + 3);
                break;
            case ' ':
                if (selected < selectedClass->studentCount) {
                    editRecord.attendance[selected] = 
                        (editRecord.attendance[selected] == 'P') ? 'A' : 'P';
                }
                break;
            case '\n':
                if (selected == selectedClass->studentCount) {
                    save_attendance_record(&editRecord);
                    return;
                }
                else if (selected == selectedClass->studentCount + 1) {
                    return;
                }
                break;
            case 27:  // ESC key
                return;
        }
    }
}

void view_records() {
    int classIndex = select_class();
    if (classIndex < 0) return;

    clear();
    mvprintw(0, 0, "Attendance Records for %s", classes[classIndex].name);

    char searchPattern[100];
    sprintf(searchPattern, "classes/%s_attendance.csv", classes[classIndex].name);

    FILE *file = fopen(searchPattern, "r");
    if (!file) {
        mvprintw(2, 0, "No attendance records found");
        refresh();
        getch();
        return;
    }

    // Count lines to determine record count
    int recordCount = 0;
    char line[1000];
    while (fgets(line, sizeof(line), file)) {
        recordCount++;
    }
    rewind(file);

    if (recordCount <= 1) {
        mvprintw(2, 0, "No attendance records found");
        fclose(file);
        refresh();
        getch();
        return;
    }

    int selected = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(0, 0, "Attendance Records for %s", classes[classIndex].name);

        rewind(file);
        fgets(line, sizeof(line), file);  // Skip header

        for (int i = 0; i < recordCount - 1; i++) {
            fgets(line, sizeof(line), file);
            line[strcspn(line, "\n")] = 0;
            mvprintw(2 + i, 0, "[ ] %s", line);
            
            if (i == selected) {
                mvprintw(2 + i, 1, "*");
            }
        }
        mvprintw(2 + recordCount, 0, "[ ] Exit");

        refresh();
        ch = getch();

        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + (recordCount + 1)) % (recordCount + 1);
                break;
            case KEY_DOWN:
                selected = (selected + 1) % (recordCount + 1);
                break;
            case '\n':
                if (selected == recordCount) {
                    fclose(file);
                    return;
                }
                
                // Edit record
                {
                    rewind(file);
                    fgets(line, sizeof(line), file);  // Skip header
                    for (int i = 0; i < selected; i++) {
                        fgets(line, sizeof(line), file);
                    }
                    }
                    fgets(line, sizeof(line), file);  // Get the selected record

                    char recordPath[200];
                    sprintf(recordPath, "classes/%s_attendance.csv", classes[classIndex].name);
                    edit_attendance_record(recordPath);
                break;
            case 27:  // ESC key
                fclose(file);
                return;
            }
        }
    }
void view_edit_classes() {
    int classIndex = select_class();
    if (classIndex < 0) return;

    Class *selectedClass = &classes[classIndex];
    int selected = 0;
    int ch;
    int totalOptions = selectedClass->studentCount + 3;

    while (1) {
        clear();
        mvprintw(0, 0, "Manage Students in %s", selectedClass->name);
        mvprintw(1, 0, "Total Students: %d", selectedClass->studentCount);
        mvprintw(2, 0, "Press SPACE to toggle student status");

        for (int i = 0; i < selectedClass->studentCount; i++) {
            mvprintw(4 + i, 0, "[ ] %s", selectedClass->students[i].name);
            
            if (i == selected) {
                mvprintw(4 + i, 1, "*");
            }
        }

        mvprintw(4 + selectedClass->studentCount + 1, 0, "[ ] Add Student");
        mvprintw(4 + selectedClass->studentCount + 2, 0, "[ ] Save and Exit");
        mvprintw(4 + selectedClass->studentCount + 3, 0, "[ ] Exit without Saving");

        // Highlight the selected option even in the menu area
        if (selected >= selectedClass->studentCount) {
            mvprintw(4 + selected, 1, "*");
        }

        refresh();
        ch = getch();

        switch(ch) {
            case KEY_UP:
                selected = (selected - 1 + totalOptions) % totalOptions;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % totalOptions;
                break;
            case ' ':
                if (selected < selectedClass->studentCount) {
                    // Edit student name
                    char newName[50];
                    char oldName[50];
                    strcpy(oldName, selectedClass->students[selected].name);

                    // Clear screen for name editing
                    clear();
                    mvprintw(0, 0, "Edit Student Name");
                    mvprintw(2, 0, "Previous Name: %s", oldName);
                    mvprintw(4, 0, "New Name: ");
                    echo();
                    curs_set(1);
                    getstr(newName);
                    noecho();
                    curs_set(0);

                    // Update name if not empty
                    if (strlen(newName) > 0) {
                        strcpy(selectedClass->students[selected].name, newName);
                    }
                }
                break;
            case '\n':
                if (selected == selectedClass->studentCount) {
                    // Add student
                    if (selectedClass->studentCount < MAX_STUDENTS) {
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
                        }
                    }
                }
                else if (selected == selectedClass->studentCount + 1) {
                    // Save and Exit
                    save_class(selectedClass);
                    save_classes();
                    return;
                }
                else if (selected == selectedClass->studentCount + 2) {
                    // Exit without Saving
                    return;
                }
                break;
            case 27:  // ESC key
                return;
        }
    }
}
void save_classes() {
    FILE *file = fopen("classes/class_list.txt", "w");
    if (!file) return;

    fprintf(file, "%d\n", classCount);
    for (int i = 0; i < classCount; i++) {
        fprintf(file, "%s\n", classes[i].name);
    }
    fclose(file);
}
void load_classes() {
    ensure_directory_exists("classes");
    FILE *file = fopen("classes/class_list.txt", "r");
    if (!file) return;

    fscanf(file, "%d\n", &classCount);
    for (int i = 0; i < classCount; i++) {
        fgets(classes[i].name, sizeof(classes[i].name), file);
        classes[i].name[strcspn(classes[i].name, "\n")] = 0;
        load_class(classes[i].name, &classes[i]);
    }
    fclose(file);
}
void load_class(const char *className, Class *class) {
    char filename[100];
    sprintf(filename, "classes/%s.csv", className);

    FILE *file = fopen(filename, "r");
    if (!file) return;

    // Skip header
    char line[200];
    fgets(line, sizeof(line), file);

    // Reset student count
    class->studentCount = 0;

    // Read students
    while (fgets(line, sizeof(line), file) && class->studentCount < MAX_STUDENTS) {
        char *idStr = strtok(line, ",");
        char *name = strtok(NULL, "\n");

        if (idStr && name) {
            class->students[class->studentCount].id = atoi(idStr);
            strcpy(class->students[class->studentCount].name, name);
            class->studentCount++;
        }
    }

    fclose(file);
}
int main() {
    init_curses();
    load_classes();

    char *main_options[] = {
        "Mark Attendance",
        "View Attendance Records",
        "Manage Classes",
        "Exit"
    };

    while (1) {
        clear();
        mvprintw(0, 0, "Attendance Management System");
        mvprintw(1, 0, "Main Menu");

        int choice = show_menu(main_options, 4, 3);

        switch (choice) {
            case 0:
                mark_attendance();
                break;
            case 1:
                view_records();
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