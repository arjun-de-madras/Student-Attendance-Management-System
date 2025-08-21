#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STUDENTS 100
#define MAX_INPUT 50
#define THRESHOLD 75.0f

typedef struct {
    int id;
    char name[MAX_INPUT];
    char course[MAX_INPUT];
    char password[MAX_INPUT];
    int totalClasses;
    int attendedClasses;
    bool isDefaulter;
} Student;

typedef enum {
    LOGIN, ADMIN_MENU, STUDENT_MENU, ADD_STUDENT, 
    MARK_ATTENDANCE, VIEW_RECORDS, REPORTS, STUDENT_VIEW
} Screen;

// Global variables
Student students[MAX_STUDENTS];
int studentCount = 0;
Screen currentScreen = LOGIN;
int currentStudent = -1;
char inputBuffer[MAX_INPUT] = "";
int activeField = 0;
bool isAdmin = false;
char tempId[MAX_INPUT] = "";
char tempName[MAX_INPUT] = "";
char tempCourse[MAX_INPUT] = "";
char tempPass[MAX_INPUT] = "";
char message[100] = "";
float messageTimer = 0;

// Admin credentials
const char* ADMIN_ID = "admin";
const char* ADMIN_PASSWORD = "admin123";

// Sample student credentials
const char* STUDENT_ID = "student";
const char* STUDENT_PASSWORD = "student123";

// Function prototypes
void SaveData(void);
void LoadData(void);
void UpdateStatus(void);
void ShowMessage(const char* msg);
void ProcessInput(void);
void RenderUI(void);
void InitializeDefaultData(void);

int main(void) {
    InitWindow(800, 600, "Student Attendance System");
    SetTargetFPS(60);
    
    // Initialize buffers
    memset(tempId, 0, MAX_INPUT);
    memset(tempName, 0, MAX_INPUT);
    memset(tempCourse, 0, MAX_INPUT);
    memset(tempPass, 0, MAX_INPUT);
    memset(inputBuffer, 0, MAX_INPUT);
    
    LoadData();
    UpdateStatus();
    
    while (!WindowShouldClose()) {
        ProcessInput();
        
        if (messageTimer > 0) {
            messageTimer -= GetFrameTime();
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        RenderUI();
        EndDrawing();
    }
    
    SaveData();
    CloseWindow();
    return 0;
}

// Initialize with default data if no file exists
void InitializeDefaultData(void) {
    // Default admin is already set in global variables
    
    // Add a default student
    if (studentCount == 0) {
        students[0].id = 1001;
        strcpy(students[0].name, "Demo Student");
        strcpy(students[0].course, "Computer Science");
        strcpy(students[0].password, STUDENT_PASSWORD);
        students[0].totalClasses = 10;
        students[0].attendedClasses = 7;
        students[0].isDefaulter = false;
        studentCount = 1;
        SaveData();
    }
}

void SaveData(void) {
    FILE* file = fopen("students.dat", "wb");
    if (file) {
        fwrite(&studentCount, sizeof(int), 1, file);
        if (studentCount > 0) {
            fwrite(students, sizeof(Student), studentCount, file);
        }
        fclose(file);
    }
}

void LoadData(void) {
    FILE* file = fopen("students.dat", "rb");
    if (file) {
        fread(&studentCount, sizeof(int), 1, file);
        if (studentCount > 0 && studentCount <= MAX_STUDENTS) {
            fread(students, sizeof(Student), studentCount, file);
        } else {
            studentCount = 0; // Reset if invalid count
        }
        fclose(file);
        InitializeDefaultData(); // Add default data if needed
    } else {
        InitializeDefaultData(); // Create default data if file doesn't exist
    }
}

void UpdateStatus(void) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].totalClasses > 0) {
            float percentage = (float)students[i].attendedClasses / students[i].totalClasses * 100.0f;
            students[i].isDefaulter = (percentage < THRESHOLD);
        } else {
            students[i].isDefaulter = false;
        }
    }
}

void ShowMessage(const char* msg) {
    strcpy(message, msg);
    messageTimer = 3.0f;
}

        
    

void ProcessInput(void) {
    // Process common key inputs for text fields
    if (activeField > 0) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(inputBuffer) < MAX_INPUT - 1)) {
                inputBuffer[strlen(inputBuffer)] = (char)key;
                inputBuffer[strlen(inputBuffer) + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE) && strlen(inputBuffer) > 0) {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            // Submit on Enter key
            if (currentScreen == LOGIN) {
                if (activeField == 2) { // If in password field, simulate login button press
                    activeField = 0;
                    // Handle login logic later
                }
            }
        }
    }
    
    // Handle screen-specific inputs
    switch (currentScreen) {
        case LOGIN: {
            Rectangle idBox = {300, 200, 200, 40};
            Rectangle passBox = {300, 260, 200, 40};
            Rectangle loginButton = {350, 320, 100, 40};
            
            if (CheckCollisionPointRec(GetMousePosition(), idBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 1;
                strcpy(inputBuffer, tempId);
            } else if (CheckCollisionPointRec(GetMousePosition(), passBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                activeField = 2;
                strcpy(inputBuffer, tempPass);
            } else if (CheckCollisionPointRec(GetMousePosition(), loginButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || 
                      (activeField == 2 && IsKeyPressed(KEY_ENTER))) {
                // Handle login
                if (strcmp(tempId, ADMIN_ID) == 0 && strcmp(tempPass, ADMIN_PASSWORD) == 0) {
                    isAdmin = true;
                    currentScreen = ADMIN_MENU;
                    ShowMessage("Admin login successful!");
                } else if (strcmp(tempId, STUDENT_ID) == 0 && strcmp(tempPass, STUDENT_PASSWORD) == 0) {
                    // Direct login for demo student
                    isAdmin = false;
                    for (int i = 0; i < studentCount; i++) {
                        if (students[i].id == 1001) {
                            currentStudent = i;
                            break;
                        }
                    }
                    currentScreen = STUDENT_MENU;
                    ShowMessage("Student login successful!");
                } else {
                    // Check registered student login
                    bool found = false;
                    for (int i = 0; i < studentCount; i++) {
                        char sid[MAX_INPUT];
                        sprintf(sid, "%d", students[i].id);
                        if (strcmp(tempId, sid) == 0 && strcmp(tempPass, students[i].password) == 0) {
                            isAdmin = false;
                            currentStudent = i;
                            currentScreen = STUDENT_MENU;
                            ShowMessage("Student login successful!");
                            found = true;
                            break;
                        }
                    }
                    if (!found) ShowMessage("Invalid credentials!");
                }
                activeField = 0;
                memset(tempId, 0, MAX_INPUT);
                memset(tempPass, 0, MAX_INPUT);
            }
            
            // Update input buffers
            if (activeField == 1) strcpy(tempId, inputBuffer);
            else if (activeField == 2) strcpy(tempPass, inputBuffer);
            break;
        }
        
        case ADMIN_MENU: {
            Rectangle buttons[5];
            for (int i = 0; i < 5; i++) {
                buttons[i] = (Rectangle){250, 150 + i*80, 300, 60};
            }
            
            for (int i = 0; i < 5; i++) {
                if (CheckCollisionPointRec(GetMousePosition(), buttons[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    switch (i) {
                        case 0: 
                            currentScreen = ADD_STUDENT; 
                            memset(tempId, 0, MAX_INPUT);
                            memset(tempName, 0, MAX_INPUT);
                            memset(tempCourse, 0, MAX_INPUT);
                            memset(tempPass, 0, MAX_INPUT);
                            break;
                        case 1: currentScreen = MARK_ATTENDANCE; break;
                        case 2: currentScreen = VIEW_RECORDS; break;
                        case 3: currentScreen = REPORTS; break;
                        case 4: 
                            currentScreen = LOGIN; 
                            isAdmin = false;
                            break;
                    }
                    activeField = 0;
                }
            }
            break;
        }
        
        case STUDENT_MENU: {
            Rectangle buttons[2];
            for (int i = 0; i < 2; i++) {
                buttons[i] = (Rectangle){250, 180 + i*80, 300, 60};
            }
            
            for (int i = 0; i < 2; i++) {
                if (CheckCollisionPointRec(GetMousePosition(), buttons[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    switch (i) {
                        case 0: currentScreen = STUDENT_VIEW; break;
                        case 1: 
                            currentScreen = LOGIN; 
                            currentStudent = -1; 
                            break;
                    }
                    activeField = 0;
                }
            }
            break;
        }
        
        case ADD_STUDENT: {
            Rectangle fields[4] = {
                {300, 150, 200, 40}, // ID
                {300, 200, 200, 40}, // Name
                {300, 250, 200, 40}, // Course
                {300, 300, 200, 40}  // Password
            };
            Rectangle addButton = {450, 360, 100, 40};
            Rectangle backButton = {250, 360, 100, 40};
            
            for (int i = 0; i < 4; i++) {
                if (CheckCollisionPointRec(GetMousePosition(), fields[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    activeField = i + 1;
                    switch (i) {
                        case 0: strcpy(inputBuffer, tempId); break;
                        case 1: strcpy(inputBuffer, tempName); break;
                        case 2: strcpy(inputBuffer, tempCourse); break;
                        case 3: strcpy(inputBuffer, tempPass); break;
                    }
                }
            }
            
            if (CheckCollisionPointRec(GetMousePosition(), addButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (strlen(tempId) > 0 && strlen(tempName) > 0 && strlen(tempCourse) > 0 && strlen(tempPass) > 0) {
                    int id = atoi(tempId);
                    bool exists = false;
                    for (int i = 0; i < studentCount; i++) {
                        if (students[i].id == id) {
                            exists = true;
                            break;
                        }
                    }
                    
                    if (!exists && studentCount < MAX_STUDENTS && id > 0) {
                        students[studentCount].id = id;
                        strcpy(students[studentCount].name, tempName);
                        strcpy(students[studentCount].course, tempCourse);
                        strcpy(students[studentCount].password, tempPass);
                        students[studentCount].totalClasses = 0;
                        students[studentCount].attendedClasses = 0;
                        students[studentCount].isDefaulter = false;
                        studentCount++;
                        SaveData();
                        ShowMessage("Student added successfully!");
                        memset(tempId, 0, MAX_INPUT);
                        memset(tempName, 0, MAX_INPUT);
                        memset(tempCourse, 0, MAX_INPUT);
                        memset(tempPass, 0, MAX_INPUT);
                        activeField = 0;
                    } else if (id <= 0) {
                        ShowMessage("ID must be a positive number!");
                    } else if (exists) {
                        ShowMessage("ID already exists!");
                    } else {
                        ShowMessage("Student limit reached!");
                    }
                } else {
                    ShowMessage("All fields are required!");
                }
            }
            
            if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = ADMIN_MENU;
                activeField = 0;
            }
            
            // Update input buffers
            switch (activeField) {
                case 1: strcpy(tempId, inputBuffer); break;
                case 2: strcpy(tempName, inputBuffer); break;
                case 3: strcpy(tempCourse, inputBuffer); break;
                case 4: strcpy(tempPass, inputBuffer); break;
            }
            break;
        }
        
        case MARK_ATTENDANCE: {
            Rectangle backButton = {50, 520, 100, 40};
            
            if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = ADMIN_MENU;
            }
            
            // Attendance marking buttons
            for (int i = 0; i < studentCount; i++) {
                Rectangle presentBtn = {600, 180 + i*40, 30, 30};
                Rectangle absentBtn = {640, 180 + i*40, 30, 30};
                
                if (CheckCollisionPointRec(GetMousePosition(), presentBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    students[i].totalClasses++;
                    students[i].attendedClasses++;
                    ShowMessage("Marked Present!");
                    SaveData();
                    UpdateStatus();
                } else if (CheckCollisionPointRec(GetMousePosition(), absentBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    students[i].totalClasses++;
                    ShowMessage("Marked Absent!");
                    SaveData();
                    UpdateStatus();
                }
            }
            break;
        }
        
        case VIEW_RECORDS: case REPORTS: case STUDENT_VIEW: {
            Rectangle backButton = {50, 520, 100, 40};
            
            if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = isAdmin ? ADMIN_MENU : STUDENT_MENU;
            }
            break;
        }
    }
}

void RenderUI(void) {
    // Draw page header
    DrawText("STUDENT ATTENDANCE MANAGEMENT SYSTEM", 180, 50, 24, DARKBLUE);
    
    // Draw notification if active
    if (messageTimer > 0) {
        DrawRectangle(250, 100, 300, 40, Fade(SKYBLUE, 0.8f));
        DrawRectangleLines(250, 100, 300, 40, BLUE);
        DrawText(message, 260, 110, 18, DARKBLUE);
    }
    
    // Draw screen-specific UI
    switch (currentScreen) {
        case LOGIN: {
            Rectangle idBox = {300, 200, 200, 40};
            Rectangle passBox = {300, 260, 200, 40};
            Rectangle loginButton = {350, 320, 100, 40};
            
            DrawText("LOGIN", 370, 150, 30, MAROON);
            
            // Draw ID field
            DrawRectangleRec(idBox, LIGHTGRAY);
            DrawRectangleLines(idBox.x, idBox.y, idBox.width, idBox.height, (activeField == 1) ? BLUE : GRAY);
            DrawText("ID:", 230, 210, 20, DARKGRAY);
            DrawText(tempId, 310, 210, 20, DARKGRAY);
            
            // Draw password field
            DrawRectangleRec(passBox, LIGHTGRAY);
            DrawRectangleLines(passBox.x, passBox.y, passBox.width, passBox.height, (activeField == 2) ? BLUE : GRAY);
            DrawText("Password:", 180, 270, 20, DARKGRAY);
            
            // Display password as asterisks
            char hiddenPass[MAX_INPUT] = "";
            for (int i = 0; i < strlen(tempPass); i++) hiddenPass[i] = '*';
            DrawText(hiddenPass, 310, 270, 20, DARKGRAY);
            
            // Draw login button
            DrawRectangleRec(loginButton, SKYBLUE);
            DrawRectangleLines(loginButton.x, loginButton.y, loginButton.width, loginButton.height, BLUE);
            DrawText("LOGIN", loginButton.x + 25, loginButton.y + 10, 20, DARKBLUE);
            
            DrawText("Admin Login: ID = admin, Password = admin123", 250, 380, 16, GRAY);
            DrawText("Student Login: ID = student, Password = student123", 240, 400, 16, GRAY);
            DrawText("Or login with credentials for any added student", 250, 420, 16, GRAY);
            break;
        }
        
        case ADMIN_MENU: {
            
            const char* menuItems[] = {
                "Add New Student",
                "Mark Attendance",
                "View Records",
                "Generate Reports",
                "Logout"
            };
            
            for (int i = 0; i < 5; i++) {
                Rectangle btn = {250, 150 + i*80, 300, 60};
                DrawRectangleRec(btn, SKYBLUE);
                DrawRectangleLines(btn.x, btn.y, btn.width, btn.height, BLUE);
                DrawText(menuItems[i], btn.x + 20, btn.y + 15, 20, DARKBLUE);
            }
            break;
        }
        
        case STUDENT_MENU: {
            
            if (currentStudent >= 0 && currentStudent < studentCount) {
                DrawText(TextFormat("Welcome, %s", students[currentStudent].name), 
                    300, 170, 20, DARKGRAY);
            }
            
            const char* menuItems[] = {
                "View My Attendance",
                "Logout"
            };
            
            for (int i = 0; i < 2; i++) {
                Rectangle btn = {250, 210 + i*80, 300, 60};
                DrawRectangleRec(btn, SKYBLUE);
                DrawRectangleLines(btn.x, btn.y, btn.width, btn.height, BLUE);
                DrawText(menuItems[i], btn.x + 60, btn.y + 15, 20, DARKBLUE);
            }
            break;
        }
        
        case ADD_STUDENT: {
            
            const char* labels[] = {"ID:", "Name:", "Course:", "Password:"};
            const char* values[] = {tempId, tempName, tempCourse, tempPass};
            
            for (int i = 0; i < 4; i++) {
                Rectangle field = {300, 150 + i*50, 200, 40};
                DrawRectangleRec(field, LIGHTGRAY);
                DrawRectangleLines(field.x, field.y, field.width, field.height, (activeField == i+1) ? BLUE : GRAY);
                DrawText(labels[i], field.x - 200, field.y + 10, 20, DARKGRAY);
                
                if (i == 3) { // Password field
                    char hidden[MAX_INPUT] = "";
                    for (int j = 0; j < strlen(values[i]); j++) hidden[j] = '*';
                    DrawText(hidden, field.x + 10, field.y + 10, 20, DARKGRAY);
                } else {
                    DrawText(values[i], field.x + 10, field.y + 10, 20, DARKGRAY);
                }
            }
            
            // Add & Back buttons
            Rectangle addBtn = {450, 360, 100, 40};
            Rectangle backBtn = {250, 360, 100, 40};
            
            DrawRectangleRec(addBtn, SKYBLUE);
            DrawRectangleLines(addBtn.x, addBtn.y, addBtn.width, addBtn.height, BLUE);
            DrawText("ADD", addBtn.x + 30, addBtn.y + 10, 20, DARKBLUE);
            
            DrawRectangleRec(backBtn, LIGHTGRAY);
            DrawRectangleLines(backBtn.x, backBtn.y, backBtn.width, backBtn.height, GRAY);
            DrawText("BACK", backBtn.x + 25, backBtn.y + 10, 20, DARKGRAY);
            break;
        }
        
        case MARK_ATTENDANCE: {
            
            // Draw student list
            DrawText("Students:", 150, 150, 20, DARKGRAY);
            
            if (studentCount == 0) {
                DrawText("No students found. Add students first.", 200, 200, 20, DARKGRAY);
            } else {
                for (int i = 0; i < studentCount; i++) {
                    Rectangle item = {150, 180 + i*40, 400, 30};
                    Color bgColor = (i % 2 == 0) ? LIGHTGRAY : WHITE;
                    DrawRectangleRec(item, bgColor);
                    DrawText(TextFormat("%d - %s (%s)", students[i].id, students[i].name, students[i].course), 
                            item.x + 10, item.y + 5, 18, DARKGRAY);
                    
                    // Attendance buttons
                    Rectangle presentBtn = {600, 180 + i*40, 30, 30};
                    Rectangle absentBtn = {640, 180 + i*40, 30, 30};
                    
                    DrawRectangleRec(presentBtn, GREEN);
                    DrawText("P", presentBtn.x + 10, presentBtn.y + 8, 16, WHITE);
                    
                    DrawRectangleRec(absentBtn, RED);
                    DrawText("A", absentBtn.x + 10, absentBtn.y + 8, 16, WHITE);
                }
            }
            
            // Back button
            Rectangle backBtn = {50, 520, 100, 40};
            DrawRectangleRec(backBtn, LIGHTGRAY);
            DrawRectangleLines(backBtn.x, backBtn.y, backBtn.width, backBtn.height, GRAY);
            DrawText("BACK", backBtn.x + 25, backBtn.y + 10, 20, DARKGRAY);
            break;
        }
        
        case VIEW_RECORDS: {
            DrawText("VIEW STUDENT RECORDS", 260, 110, 30, DARKBLUE);
            
            // Draw student list with attendance
            if (studentCount == 0) {
                DrawText("No students found. Add students first.", 200, 200, 20, DARKGRAY);
            } else {
                for (int i = 0; i < studentCount; i++) {
                    Rectangle item = {150, 150 + i*40, 500, 30};
                    Color bgColor = (i % 2 == 0) ? LIGHTGRAY : WHITE;
                    DrawRectangleRec(item, bgColor);
                    
                    float percentage = students[i].totalClasses > 0 ? 
                                    (float)students[i].attendedClasses / students[i].totalClasses * 100.0f : 0;
                    Color statusColor = percentage < THRESHOLD ? RED : GREEN;
                    
                    DrawText(TextFormat("%d - %s (%s)", students[i].id, students[i].name, students[i].course), 
                            item.x + 10, item.y + 5, 18, DARKGRAY);
                    DrawText(TextFormat("%.1f%%", percentage), 
                            item.x + 400, item.y + 5, 18, statusColor);
                }
            }
            
            // Back button
            Rectangle backBtn = {50, 520, 100, 40};
            DrawRectangleRec(backBtn, LIGHTGRAY);
            DrawRectangleLines(backBtn.x, backBtn.y, backBtn.width, backBtn.height, GRAY);
            DrawText("BACK", backBtn.x + 25, backBtn.y + 10, 20, DARKGRAY);
            break;
        }
        
        case REPORTS: {
            DrawText("ATTENDANCE REPORTS", 280, 110, 30, DARKBLUE);
            
            // Count defaulters
            int defaulters = 0;
            for (int i = 0; i < studentCount; i++) {
                if (students[i].isDefaulter) defaulters++;
            }
            
            DrawText(TextFormat("Total Students: %d", studentCount), 300, 170, 20, DARKGRAY);
            DrawText(TextFormat("Defaulters: %d", defaulters), 300, 200, 20, RED);
            
            if (defaulters > 0) {
                DrawText("Defaulter List (Below 75% attendance):", 200, 240, 20, RED);
                
                int index = 0;
                for (int i = 0; i < studentCount; i++) {
                    if (students[i].isDefaulter) {
                        float percentage = students[i].totalClasses > 0 ? 
                                        (float)students[i].attendedClasses / students[i].totalClasses * 100.0f : 0;
                        
                        DrawText(TextFormat("%d. %s (ID: %d) - %.1f%%", 
                                index+1, students[i].name, students[i].id, percentage),
                                220, 270 + index*30, 18, DARKGRAY);
                        index++;
                    }
                }
            } else if (studentCount > 0) {
                DrawText("No defaulters found. All students have good attendance.", 200, 240, 20, GREEN);
            } else {
                DrawText("No students found. Add students first.", 200, 240, 20, DARKGRAY);
            }
            
            // Back button
            Rectangle backBtn = {50, 520, 100, 40};
            DrawRectangleRec(backBtn, LIGHTGRAY);
            DrawRectangleLines(backBtn.x, backBtn.y, backBtn.width, backBtn.height, GRAY);
            DrawText("BACK", backBtn.x + 25, backBtn.y + 10, 20, DARKGRAY);
            break;
        }
        
        case STUDENT_VIEW: {
            if (currentStudent >= 0 && currentStudent < studentCount) {
                Student* student = &students[currentStudent];
                
                DrawText(TextFormat("ID: %d", student->id), 300, 170, 20, DARKGRAY);
                DrawText(TextFormat("Course: %s", student->course), 300, 200, 20, DARKGRAY);
                DrawText(TextFormat("Total Classes: %d", student->totalClasses), 300, 230, 20, DARKGRAY);
                DrawText(TextFormat("Classes Attended: %d", student->attendedClasses), 300, 260, 20, DARKGRAY);
                
                float percentage = student->totalClasses > 0 ? 
                                (float)student->attendedClasses / student->totalClasses * 100.0f : 0;
                Color statusColor = percentage < THRESHOLD ? RED : GREEN;
                
                DrawText(TextFormat("Attendance: %.1f%%", percentage), 300, 290, 20, statusColor);
                
                // Notification for defaulters
                if (percentage < THRESHOLD) {
                    DrawRectangle(200, 330, 400, 60, Fade(RED, 0.2f));
                    DrawRectangleLines(200, 330, 400, 60, RED);
                    DrawText("WARNING: Your attendance is below 75%!", 210, 340, 20, RED);
                    DrawText("Please improve your attendance!", 210, 365, 18, RED);
                }
                
                // Visual representation of attendance+
                
                
                DrawRectangle(300, 410, 200, 30, LIGHTGRAY);
                if (percentage > 0) {
                    DrawRectangle(300, 410, (int)(percentage * 2), 30, statusColor);
                }
                DrawRectangleLines(300, 410, 200, 30, DARKGRAY);
                DrawText(TextFormat("%.1f%%", percentage), 520, 415, 18, DARKGRAY);
                
                // Show minimum required
                DrawLine(300 + (int)(THRESHOLD * 2), 410, 300 + (int)(THRESHOLD * 2), 440, DARKGREEN);
                DrawText("Min 75%", 300 + (int)(THRESHOLD * 2) - 30, 445, 14, DARKGREEN);
            }
            
            // Back button
            Rectangle backBtn = {50, 520, 100, 40};
            DrawRectangleRec(backBtn, LIGHTGRAY);
            DrawRectangleLines(backBtn.x, backBtn.y, backBtn.width, backBtn.height, GRAY);
            DrawText("BACK", backBtn.x + 25, backBtn.y + 10, 20, DARKGRAY);
            break;
        }
    }
}