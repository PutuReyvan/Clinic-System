#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#define TABLE_SIZE 100
#define ROLE_CLIENT 0
#define ROLE_ADMIN 1
#define ROLE_DOCTOR 2

typedef struct reservation_node {
    char date[20];
    char time[10];
    char doctor[50];
    char notes[100];
    struct reservation_node *next;
} ReservationNode;

typedef struct user {
    char username[20];
    char password[20];
    int role; // 0 = client, 1 = admin
    int available; // 0 = not available, 1 = available
    struct reservation_node *reservations;
    struct user *next;
} User;

typedef struct {
    User *table[TABLE_SIZE];
} hash_table;


int hash_function(const char *username) {
    int hash = 0;
    for (int i = 0; username[i]; i++) {
        hash = (hash * 31 + username[i]) % TABLE_SIZE;
    }
    return hash;
}

void insert_user(hash_table *ht, const char *username, const char *password, int role) {
    int idx = hash_function(username);
    User *u = (User *)malloc(sizeof(User));

    if (!u) {
        puts("Memory allocation failed");
        return;
    }

    strncpy(u->username, username, sizeof(u->username) - 1);
    u->username[sizeof(u->username) - 1] = '\0';
    strncpy(u->password, password, sizeof(u->password) - 1);
    u->password[sizeof(u->password) - 1] = '\0';

    u->role = role;
    u->reservations = NULL;
    u->next = ht->table[idx];
    ht->table[idx] = u;
    u->available = (role == ROLE_DOCTOR) ? 1 : 0;
}

User *find_user(hash_table *ht, const char *username) {
    int idx = hash_function(username);
    for (User *cur = ht->table[idx]; cur; cur = cur->next) {
        if (strcmp(cur->username, username) == 0) return cur;
    }
    return NULL;
}


void pause_console(void) {
    printf("Press any key to continue...");
    _getch();
    printf("\n");
}


void view_all_users(hash_table *ht) {
    puts("=== List of Users ===");
    for (int i = 0; i < TABLE_SIZE; i++) {
        User *cur = ht->table[i];
        while (cur) {
            printf("Username: %s | Role: %s\n", cur->username, cur->role == 1 ? "Admin" : "Client");
            cur = cur->next;
        }
    }
}

void delete_user(hash_table *ht, const char *username) {
    int idx = hash_function(username);
    User *cur = ht->table[idx];
    User *prev = NULL;

    while (cur) {
        if (strcmp(cur->username, username) == 0) {
            if (prev == NULL) {
                ht->table[idx] = cur->next;
            } else {
                prev->next = cur->next;
            }
            
            ReservationNode *res = cur->reservations;
            while (res) {
                ReservationNode *next = res->next;
                free(res);
                res = next;
            }
            free(cur);
            printf("User '%s' has been deleted.\n", username);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    printf("User '%s' not found.\n", username);
}

void admin_menu(hash_table *ht) {
    int choice;
    do {
        system("cls");
        puts("=== ADMIN MENU ===");
        puts("1. View Users");
        puts("2. Delete User");
        puts("3. Generate Report");
        puts("4. Rating Summary");
        puts("0. Logout");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = -1;
        }
        getchar();

        switch (choice) {
            case 1:
                view_all_users(ht);
                pause_console();
                break;
            case 2: {
                char uname[20];
                printf("Enter username to delete: ");
                scanf("%19s", uname);
                getchar();
                delete_user(ht, uname);
                pause_console();
                break;
            }
        }
    } while (choice != 0);
}

// === CLIENT ===
void create_reservation(User *u, hash_table *ht)
{
    ReservationNode *res = (ReservationNode *)malloc(sizeof(ReservationNode));
    if (!res)
    {
        puts("Memory allocation failed.");
        return;
    }

    printf("Enter doctor's name: ");
    scanf(" %[^\n]", res->doctor);

    User *doctor = find_user(ht, res->doctor);
    if (!doctor || doctor->role != ROLE_DOCTOR)
    {
        puts("Doctor not found.");
        free(res);
        return;
    }
    if (!doctor->available)
    {
        puts("Doctor is currently unavailable.");
        free(res);
        return;
    }

    printf("Enter date (YYYY-MM-DD): ");
    scanf("%19s", res->date);
    getchar();

    printf("Enter time (HH:MM): ");
    scanf("%9s", res->time);
    getchar();

    printf("Enter notes: ");
    scanf(" %[^\n]", res->notes);

    res->next = u->reservations;
    u->reservations = res;

    puts("Reservation created successfully!");
}

void view_reservation(User *u) {
    puts("=== Your Reservations ===");
    ReservationNode *res = u->reservations;
    if (!res) {
        puts("No reservations found.");
        return;
    }

    int i = 1;
    while (res) {
        printf("Reservation #%d:\n", i++);
        printf("Date   : %s\n", res->date);
        printf("Time   : %s\n", res->time);
        printf("Doctor : %s\n", res->doctor);
        printf("Notes  : %s\n\n", res->notes);
        res = res->next;
    }
}

void client_menu(User *u, hash_table *ht)
{
    int choice;
    do {
        system("cls");
        printf("=== CLIENT MENU (User: %s) ===\n", u->username);
        puts("1. Create Reservation");
        puts("2. View My Reservations");
        puts("3. Cancel Reservation");
        puts("4. Payment");
        puts("5. View Doctors List");
        puts("6. Rate Doctor");
        puts("0. Logout");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1){
            while (getchar() != '\n');
            choice = -1;
        }
        getchar();

        switch (choice){
            case 1:
                create_reservation(u, ht);
                pause_console();
                break;
            case 2:
                view_reservation(u);
                pause_console();
                break;
        }
    } while (choice != 0);
}
// Bagian dokter

// Untuk liat list appointment Dokter A
void view_doctor_appointments(hash_table *ht, const char *doctor_name)
{
    puts("=== Doctor's Appointments ===");
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        User *u = ht->table[i];
        while (u)
        {
            ReservationNode *res = u->reservations;
            while (res)
            {
                if (strcmp(res->doctor, doctor_name) == 0)
                {
                    printf("Patient: %s\nDate: %s\nTime: %s\nNotes: %s\n\n",
                           u->username, res->date, res->time, res->notes);
                }
                res = res->next;
            }
            u = u->next;
        }
    }
}
// Untuk set dokter available ato engga
void toggle_availability(User *u)
{
    u->available = !u->available;
    puts(u->available ? "You are now available." : "You are now unavailable.");
}

// Dokter Menu
void doctor_menu(User *u, hash_table *ht)
{
    int choice;
    do
    {
        system("cls");
        printf("=== DOCTOR MENU (User: %s) ===\n", u->username);
        puts("1. View My Appointments");
        puts("2. Toggle Availability");
        puts("0. Logout");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1)
        {
            while (getchar() != '\n')
                ;
            choice = -1;
        }
        getchar();

        switch (choice)
        {
        case 1:
            view_doctor_appointments(ht, u->username);
            pause_console();
            break;
        case 2:
            toggle_availability(u);
            pause_console();
            break;
        }
    } while (choice != 0);
}
// === AUTH ===
void login(hash_table *ht) {
    char username[20], password[20];

    printf("Username: ");
    scanf("%19s", username);
    getchar();

    printf("Password: ");
    scanf("%19s", password);
    getchar();

    User *u = find_user(ht, username);
    if (u && strcmp(u->password, password) == 0) {
        if (u->role == ROLE_ADMIN)
        {
            puts("Login successful as ADMIN.");
            pause_console();
            admin_menu(ht);
        }
        else if (u->role == ROLE_DOCTOR)
        {
            puts("Login successful as DOCTOR.");
            pause_console();
            doctor_menu(u, ht); // Implement doctor menu if needed
        }
        else
        {
            puts("Login successful as CLIENT.");
            pause_console();
            client_menu(u, ht);
        }
    } else {
        puts("Invalid username or password.");
        pause_console();
    }
}

void register_client(hash_table *ht) {
    char username[20], password[20];

    puts("=== Client Registration ===");
    printf("Choose username: ");
    scanf("%19s", username);
    getchar();

    if (find_user(ht, username)) {
        puts("Username already exists.");
        pause_console();
        return;
    }

    printf("Choose password: ");
    scanf("%19s", password);
    getchar();

    insert_user(ht, username, password, 0);
    puts("Registration successful!");
    pause_console();
}

// === MAIN ===
int main() {
    hash_table ht = { 0 };
    insert_user(&ht, "admin", "admin123", 1); // default admin
    insert_user(&ht, "DokterDOom", "kamartaj", 2); // default doctor

    int choice;
    do {
        system("cls");
        puts("=== Clinic System ===");
        puts("1. Register (Client)");
        puts("2. Login");
        puts("0. Exit");
        printf("Choose: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = -1;
        }
        getchar();

        switch (choice) {
            case 1:
                register_client(&ht);
                break;
            case 2:
                login(&ht);
                break;
            case 0:
                puts("Goodbye!");
                break;
            default:
                puts("Invalid choice.");
                pause_console();
        }
    } while (choice != 0);

    return 0;
}
