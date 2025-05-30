#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <ctype.h>

#define TABLE_SIZE 100
#define ROLE_CLIENT 0
#define ROLE_ADMIN 1
#define ROLE_DOCTOR 2
#define ALPHABET_SIZE 26

// Node untuk menyimpan data reservasi
//  yang akan disimpan dalam queue

typedef struct reservation_node {
    char date[20];
    char time[10];
    char doctor[50];
    char notes[100];
    struct reservation_node *next;
} ReservationNode;

// Struktur untuk menyimpan data user
// yang akan disimpan dalam hash table
// dengan chaining untuk mengatasi collision (linked list)
typedef struct user {
    char username[20];
    char password[20];
    int role; // 0 = client, 1 = admin, 2 = doctor
    int available; // 0 = not available, 1 = available
    int total_rating;
    int rating_count;
    ReservationNode *reservations_front;  // queue front
    ReservationNode *reservations_rear;   // queue rear
    struct user *next;
} User;

typedef struct {
    User *table[TABLE_SIZE];
} hash_table;

// Struct untuk Tries
typedef struct trie_node
{
    struct trie_node *children[ALPHABET_SIZE];
    int is_end_of_word; //
    char username[20];  //
} TrieNode;

TrieNode *trie_root = NULL;

// Fungsi untuk membuat hash dari username
// Menggunakan metode hash sederhana dengan perkalian
int hash_function(const char *username) {
    int hash = 0;
    for (int i = 0; username[i]; i++) {
        hash = (hash * 31 + username[i]) % TABLE_SIZE;
    }
    return hash;
}

TrieNode *create_trie_node()
{
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (node)
    {
        node->is_end_of_word = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++)
            node->children[i] = NULL;
        node->username[0] = '\0';
    }
    return node;
}

void insert_trie(TrieNode *root, const char *username)
{
    TrieNode *cur = root;
    for (int i = 0; username[i]; i++)
    {
        char ch = tolower(username[i]);
        if (ch < 'a' || ch > 'z')
            continue;

        int idx = ch - 'a';
        if (!cur->children[idx])
            cur->children[idx] = create_trie_node();
        cur = cur->children[idx];
    }
    cur->is_end_of_word = 1;

    // Store lowercase version to match what's in the hash table
    strncpy(cur->username, username, sizeof(cur->username));
    cur->username[sizeof(cur->username) - 1] = '\0';
}

// Function untuk insert user ke hash table
// dengan chaining untuk mengatasi collision
// Jika username sudah ada, tidak akan ditambahkan
// dan akan menampilkan pesan error
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
    u->available = (role == ROLE_DOCTOR) ? 1 : 0;
    u->reservations_front = NULL;
    u->reservations_rear = NULL;
    u->next = ht->table[idx];

    u->total_rating = 0; // Initialize total rating
    u->rating_count = 0; // Initialize rating count

    ht->table[idx] = u;
}

// Function untuk mencari user berdasarkan username
User *find_user(hash_table *ht, const char *username) {
    int idx = hash_function(username);
    for (User *cur = ht->table[idx]; cur; cur = cur->next) {
        if (strcmp(cur->username, username) == 0) return cur;
    }
    return NULL;
}

// Pause console untuk menunggu input dari user
void pause_console(void) {
    printf("Press any key to continue...");
    _getch();
    printf("\n");
}

// ==================================== ADMIN ===========================

// Fungsi untuk menampilkan semua user
// yang ada di hash table
void view_all_users(hash_table *ht) {
    puts("=== List of Users ===");
    for (int i = 0; i < TABLE_SIZE; i++) {
        User *cur = ht->table[i];
        while (cur) {
            const char *role_str = (cur->role == ROLE_ADMIN) ? "Admin" : (cur->role == ROLE_DOCTOR) ? "Doctor"
                                                                                                    : "Client";
            printf("Username: %s | Role: %s\n", cur->username, role_str);
            cur = cur->next;
        }
    }
}

// Fungsi untuk menghapus user berdasarkan username
// Jika user ditemukan, akan menghapus user dan semua reservasi yang dimilikinya
// Jika tidak ditemukan, akan menampilkan pesan error
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
            // Free all reservations
            ReservationNode *res = cur->reservations_front;
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

void to_lowercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

void print_rating_trie(TrieNode *node, hash_table *ht)
{
    if (!node)
        return;

    if (node->is_end_of_word)
    {
        char lowered[20];
        strncpy(lowered, node->username, sizeof(lowered));
        lowered[sizeof(lowered) - 1] = '\0';
        to_lowercase(lowered);

        User *u = find_user(ht, node->username);
        if (u && u->role == ROLE_DOCTOR)
        {
            if (u->rating_count > 0)
            {
                float avg = (float)u->total_rating / u->rating_count;
                printf("- %s - Avg Rating: %.2f (%d ratings)\n", u->username, avg, u->rating_count);
            }
            else
            {
                printf("- %s - No ratings yet\n", u->username);
            }
        }
        else
        {
            printf("- %s - (Doctor not found in system)\n", node->username);
        }
    }

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (node->children[i])
        {
            print_rating_trie(node->children[i], ht);
        }
    }
}

void insert_user_and_trie(hash_table *ht, TrieNode *trie, const char *username, const char *password, int role)
{
    char lowered[20];
    strncpy(lowered, username, sizeof(lowered));
    lowered[sizeof(lowered) - 1] = '\0';
    to_lowercase(lowered);

    insert_user(ht, lowered, password, role); // lowercase stored in hash table
    insert_trie(trie, lowered);               // and in Trie
}

void search_rating_by_prefix(TrieNode *root, hash_table *ht, const char *prefix)
{
    TrieNode *cur = root;
    for (int i = 0; prefix[i]; i++)
    {
        char ch = tolower(prefix[i]);
        if (ch < 'a' || ch > 'z')
            continue;
        int idx = ch - 'a';

        if (!cur->children[idx])
        {
            printf("No doctor found with prefix '%s'.\n", prefix);
            return;
        }
        cur = cur->children[idx];
    }

    char buffer[50];
    strcpy(buffer, prefix);
    print_rating_trie(cur, ht);
}

// Fungsi untuk menampilkan menu admin
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
            case 3:
                puts("Generate Report feature not implemented yet.");
                pause_console();
                break;
            case 4:
                char prefix[20];
                printf("Enter doctor name prefix: ");
                scanf("%19s", prefix);
                search_rating_by_prefix(trie_root, ht, prefix);
                pause_console();
                break;
        }
    } while (choice != 0);
}

// ========================= CLIENT==============================

// Fungsi untuk membuat reservasi
// User akan memasukkan nama dokter, tanggal, waktu, dan catatan
// Jika dokter tidak ditemukan atau tidak tersedia, akan menampilkan pesan error

void view_doctors_list(hash_table *ht)
{
    puts("=== List of Doctors ===");
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        User *cur = ht->table[i];
        while (cur)
        {
            if (cur->role == ROLE_DOCTOR)
            {
                printf("Doctor: %s | Available: %s\n", cur->username, cur->available ? "Yes" : "No");
            }
            cur = cur->next;
        }
    }
}

void create_reservation(User *u, hash_table *ht) {
    view_doctors_list(ht);
    ReservationNode *res = (ReservationNode *)malloc(sizeof(ReservationNode));
    if (!res) {
        puts("Memory allocation failed.");
        return;
    }

    puts("=== Create Reservation ===");

    printf("Enter doctor's name: ");
    scanf(" %[^\n]", res->doctor);

    User *doctor = find_user(ht, res->doctor);
    if (!doctor || doctor->role != ROLE_DOCTOR) {
        puts("Doctor not found.");
        free(res);
        return;
    }
    if (!doctor->available) {
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

    res->next = NULL;
    if (u->reservations_rear == NULL) {
        u->reservations_front = u->reservations_rear = res;
    } else {
        u->reservations_rear->next = res;
        u->reservations_rear = res;
    }
    puts("Reservation created successfully!");
}

// Fungsi untuk menampilkan semua reservasi
// yang dimiliki oleh user
void view_reservation(User *u) {
    puts("=== Your Reservations ===");
    ReservationNode *res = u->reservations_front;
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

// Fungsi untuk membatalkan reservasi
//  User akan memilih nomor reservasi yang ingin dibatalkan
//  Jika nomor tidak valid, akan menampilkan pesan error
void cancel_reservation(User *u) {
    if (u->reservations_front == NULL) {
        puts("No reservations to cancel.");
        return;
    }

    puts("=== Your Reservations ===");
    ReservationNode *res = u->reservations_front;
    int i = 1;
    while (res) {
        printf("%d. Date: %s, Time: %s, Doctor: %s, Notes: %s\n",
               i, res->date, res->time, res->doctor, res->notes);
        res = res->next;
        i++;
    }

    int choice;
    printf("\nMasukkan Nomor Reservasi yang akan di cancel (0 untuk kembali): ");
    if (scanf("%d", &choice) != 1)
    {
        while (getchar() != '\n')
            ;
        puts("Invalid input.");
        return;
    }
    getchar();

    if (choice == 0)
    {
        puts("Cancellation aborted.");
        return;
    }

    if (choice < 1)
    {
        puts("Invalid choice.");
        return;
    }

    ReservationNode *current = u->reservations_front;
    ReservationNode *prev = NULL;
    int count = 1;

    while (current && count < choice)
    {
        prev = current;
        current = current->next;
        count++;
    }

    if (!current)
    {
        puts("Reservation not found.");
        return;
    }

    // Remove current node
    if (prev == NULL)
    { // hapus node depan
        u->reservations_front = current->next;
        if (u->reservations_front == NULL)
        {
            u->reservations_rear = NULL;
        }
    }
    else
    {
        prev->next = current->next;
        if (current == u->reservations_rear)
        {
            u->reservations_rear = prev;
        }
    }

    free(current);
    puts("Reservation canceled successfully.");
}

// Rate Doctor function

void save_rating_to_csv(const char *filename, const char *doctor_name, int rating)
{
    FILE *file = fopen(filename, "a"); // append mode
    if (!file)
    {
        puts("Failed to save rating.");
        return;
    }

    fprintf(file, "%s,%d\n", doctor_name, rating);
    fclose(file);
}

void rate_doctor(User *u, hash_table *ht)
{
    char input[50];
    int rating;

    puts("=== Rate a Doctor ===");
    printf("Enter doctor's name or prefix: ");
    scanf(" %[^\n]", input);
    getchar();
    to_lowercase(input);

    // Traverse Trie
    TrieNode *cur = trie_root;
    for (int i = 0; input[i]; i++)
    {
        int idx = input[i] - 'a';
        if (idx < 0 || idx >= ALPHABET_SIZE || !cur->children[idx])
        {
            puts("No doctor found with that prefix.");
            return;
        }
        cur = cur->children[idx];
    }

    // Exact match? Go ahead and rate
    if (cur->is_end_of_word)
    {
        User *doctor = find_user(ht, cur->username);
        if (!doctor || doctor->role != ROLE_DOCTOR)
        {
            puts("Doctor not found.");
            return;
        }

        printf("Enter rating (1-5) for %s: ", doctor->username);
        if (scanf("%d", &rating) != 1 || rating < 1 || rating > 5)
        {
            puts("Invalid rating.");
            return;
        }
        getchar();

        doctor->total_rating += rating;
        doctor->rating_count++;
        save_rating_to_csv("ratings.csv", doctor->username, rating);

        float avg = (float)doctor->total_rating / doctor->rating_count;
        printf("Thank you! New average for %s: %.2f (%d ratings)\n", doctor->username, avg, doctor->rating_count);
    }
    else
    {
        // Prefix only – show suggestions and ask again
        puts("Doctor not found exactly, but here are suggestions:");
        print_rating_trie(cur, ht);

        char full_name[50];
        printf("\nEnter full doctor's name: ");
        scanf(" %[^\n]", full_name);
        getchar();
        to_lowercase(full_name);

        User *doctor = find_user(ht, full_name);
        if (!doctor || doctor->role != ROLE_DOCTOR)
        {
            puts("Doctor not found.");
            return;
        }

        printf("Enter rating (1-5): ");
        if (scanf("%d", &rating) != 1 || rating < 1 || rating > 5)
        {
            puts("Invalid rating.");
            return;
        }
        getchar();

        doctor->total_rating += rating;
        doctor->rating_count++;
        save_rating_to_csv("ratings.csv", doctor->username, rating);

        float avg = (float)doctor->total_rating / doctor->rating_count;
        printf("Thank you! New average for %s: %.2f (%d ratings)\n", doctor->username, avg, doctor->rating_count);
    }
}

void load_ratings_from_csv(hash_table *ht, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return; // First time run

    char doctor_name[50];
    int rating;

    while (fscanf(file, "%49[^,],%d\n", doctor_name, &rating) == 2)
    {
        to_lowercase(doctor_name);
        User *doctor = find_user(ht, doctor_name);
        if (doctor && doctor->role == ROLE_DOCTOR)
        {
            doctor->total_rating += rating;
            doctor->rating_count++;
        }
    }

    fclose(file);
}

// Fungsi untuk menampilkan daftar dokter
void client_menu(User *u, hash_table *ht)
{
    int choice;
    do
    {
        system("cls");
        printf("=== CLIENT MENU (User: %s) ===\n", u->username);
        puts("1. Create Reservation");
        puts("2. View My Reservations");
        puts("3. Cancel Reservation");
        puts("4. Payment");
        puts("5. Rate Doctor");
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
            create_reservation(u, ht);
            pause_console();
            break;
        case 2:
            view_reservation(u);
            pause_console();
            break;
        case 3:
            cancel_reservation(u);
            pause_console();
            break;
        case 4:
            puts("Payment feature not implemented yet.");
            pause_console();
            break;
        case 5:
            rate_doctor(u, ht);
            pause_console();
            break;
        default:
            puts("Feature not implemented or invalid choice.");
            pause_console();
        }
    } while (choice != 0);
}

// ===============================Dokter=============================================

// Untuk liat list appointment Dokter A
void view_doctor_appointments(hash_table *ht, const char *doctor_name)
{
    puts("=== Doctor's Appointments ===");
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        User *u = ht->table[i];
        while (u)
        {
            ReservationNode *res = u->reservations_front;
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
void toggle_availability(User *u) {
    u->available = !u->available;
    puts(u->available ? "You are now available." : "You are now unavailable.");
}

// Dokter Menu
void doctor_menu(User *u, hash_table *ht) {
    int choice;
    do {
        system("cls");
        printf("=== DOCTOR MENU (User: %s) ===\n", u->username);
        puts("1. View My Appointments");
        puts("2. Toggle Availability");
        puts("0. Logout");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = -1;
        }
        getchar();

        switch (choice) {
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

// ========================== AUTH ============================

// Fungsi untuk membaca data user dari file CSV
void load_users_from_csv(hash_table *ht, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        puts("No user data found. Starting fresh.");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file))
    {
        char username[20], password[20];
        int role;

        // Parse CSV line
        if (sscanf(line, "%19[^,],%19[^,],%d", username, password, &role) == 3)
        {
            // Convert username to lowercase
            char lowered[20];
            strncpy(lowered, username, sizeof(lowered));
            lowered[sizeof(lowered) - 1] = '\0';
            to_lowercase(lowered);

            // Insert into both hash table and Trie
            insert_user(ht, lowered, password, role);
            if (role == ROLE_DOCTOR)
            {
                insert_trie(trie_root, lowered);
            }
        }
    }

    fclose(file);
}
// Fungsi untuk menyimpan data user ke file CSV
void save_user_to_csv(const char *filename, const char *username, const char *password, int role)
{
    FILE *file = fopen(filename, "a"); // Append mode
    if (!file)
    {
        puts("Failed to open user CSV for writing.");
        return;
    }

    fprintf(file, "%s,%s,%d\n", username, password, role);
    fclose(file);
}

// Fungsi untuk login user
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
        if (u->role == ROLE_ADMIN) {
            puts("Login successful as ADMIN.");
            pause_console();
            admin_menu(ht);
        } else if (u->role == ROLE_DOCTOR) {
            puts("Login successful as DOCTOR.");
            pause_console();
            doctor_menu(u, ht);
        } else {
            puts("Login successful as CLIENT.");
            pause_console();
            client_menu(u, ht);
        }
    } else {
        puts("Invalid username or password.");
        pause_console();
    }
}

// Fungsi untuk mendaftarkan user baru sebagai client
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

    insert_user(ht, username, password, ROLE_CLIENT);

    save_user_to_csv("users.csv", username, password, ROLE_CLIENT);
    puts("Registration successful!");
    pause_console();
}

// ====================== MAIN ===========================
int main() {
    hash_table ht = {0};
    trie_root = create_trie_node();

    load_users_from_csv(&ht, "users.csv");

    // Use wrapper to keep hash and Trie in sync
    insert_user_and_trie(&ht, trie_root, "admin", "admin123", ROLE_ADMIN);
    insert_user_and_trie(&ht, trie_root, "drdoom", "kamartaj", ROLE_DOCTOR);
    insert_user_and_trie(&ht, trie_root, "drstrange", "kamartaj", ROLE_DOCTOR);

    load_ratings_from_csv(&ht, "ratings.csv");

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
