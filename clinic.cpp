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
// ======================= [DATA STRUCTURES] =======================
#define MAX_HEAP 1000

// Node untuk menyimpan data reservasi
//  yang akan disimpan dalam queue

typedef struct reservation_node
{
    char date[20];
    char time[10];
    char doctor[50];
    char notes[100];
    struct reservation_node *next;
    char patient_username[20];
} ReservationNode;

// Struktur untuk menyimpan data user
// yang akan disimpan dalam hash table
// dengan chaining untuk mengatasi collision (linked list)
typedef struct user
{
    char username[20];
    char password[20];
    int role;      // 0 = client, 1 = admin, 2 = doctor
    int available; // 0 = not available, 1 = available
    int total_rating;
    int rating_count;
    ReservationNode *reservations_front; // queue front
    ReservationNode *reservations_rear;  // queue rear
    struct user *next;
} User;

typedef struct
{
    User *table[TABLE_SIZE];
} hash_table;

// Struct untuk heap
typedef struct heap_node
{
    ReservationNode *res; // Pointer to reservation node
} HeapNode;

typedef struct ReservationHeap
{
    HeapNode *data[MAX_HEAP];
    int size;
} ReservationHeap;

// Struct untuk Tries
typedef struct trie_node
{
    struct trie_node *children[ALPHABET_SIZE];
    int is_end_of_word; //
    char username[20];  //
} TrieNode;

typedef struct avl_node
{
    struct avl_node *left;
    struct avl_node *right;
    int height;
    ReservationNode *res; // Pointer to reservation node
} AVLNode;

TrieNode *trie_root = NULL;

// ======================= [UTILITY FUNCTIONS] =======================
// Fungsi untuk membuat hash dari username
// Menggunakan metode hash sederhana dengan perkalian
int hash_function(const char *username)
{
    int hash = 0;
    for (int i = 0; username[i]; i++)
    {
        hash = (hash * 31 + username[i]) % TABLE_SIZE;
    }
    return hash;
}

void swap_heap_nodes(HeapNode **a, HeapNode **b)
{
    HeapNode *temp = *a;
    *a = *b;
    *b = temp;
}

int compare_reservations(ReservationNode *a, ReservationNode *b)
{
    int cmp = strcmp(a->date, b->date);
    if (cmp == 0)
        cmp = strcmp(a->time, b->time);
    return cmp;
}

void heapify_up(ReservationHeap *heap, int idx)
{
    if (idx == 0)
        return;
    int parent = (idx - 1) / 2;
    if (compare_reservations(heap->data[idx]->res, heap->data[parent]->res) < 0)
    {
        swap_heap_nodes(&heap->data[idx], &heap->data[parent]);
        heapify_up(heap, parent);
    }
}

void heapify_down(ReservationHeap *heap, int idx)
{
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size &&
        compare_reservations(heap->data[left]->res, heap->data[smallest]->res) < 0)
        smallest = left;

    if (right < heap->size &&
        compare_reservations(heap->data[right]->res, heap->data[smallest]->res) < 0)
        smallest = right;

    if (smallest != idx)
    {
        swap_heap_nodes(&heap->data[idx], &heap->data[smallest]);
        heapify_down(heap, smallest);
    }
}

void insert_heap(ReservationHeap *heap, ReservationNode *res)
{
    if (heap->size >= MAX_HEAP)
        return;
    HeapNode *node = (HeapNode *)malloc(sizeof(HeapNode));
    node->res = res;
    heap->data[heap->size] = node;
    heapify_up(heap, heap->size);
    heap->size++;
}

HeapNode *extract_min(ReservationHeap *heap)
{
    if (heap->size == 0)
        return NULL;
    HeapNode *min = heap->data[0];
    heap->data[0] = heap->data[--heap->size];
    heapify_down(heap, 0);
    return min;
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
void insert_user(hash_table *ht, const char *username, const char *password, int role)
{
    int idx = hash_function(username);
    User *u = (User *)malloc(sizeof(User));

    if (!u)
    {
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
User *find_user(hash_table *ht, const char *username)
{
    int idx = hash_function(username);
    for (User *cur = ht->table[idx]; cur; cur = cur->next)
    {
        if (strcmp(cur->username, username) == 0)
            return cur;
    }
    return NULL;
}

void save_reservations_to_csv(hash_table *ht, const char *filename)
{
    FILE *file = fopen(filename, "w"); // overwrite
    if (!file)
    {
        puts("Failed to open file to save reservations.");
        return;
    }

    fprintf(file, "username,date,time,doctor,notes\n"); // CSV header

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        User *u = ht->table[i];
        while (u)
        {
            ReservationNode *res = u->reservations_front;
            while (res)
            {
                fprintf(file, "%s,%s,%s,%s,%s\n",
                        u->username,
                        res->date,
                        res->time,
                        res->doctor,
                        res->notes);
                res = res->next;
            }
            u = u->next;
        }
    }

    fclose(file);
    puts("Reservations saved to file successfully.");
}

void load_reservations_from_csv(hash_table *ht, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        puts("No existing reservation data found.");
        return;
    }

    char line[300];
    fgets(line, sizeof(line), file); // skip header

    while (fgets(line, sizeof(line), file))
    {
        char username[20], date[20], time[10], doctor[50], notes[100];
        if (sscanf(line, "%19[^,],%19[^,],%9[^,],%49[^,],%99[^\n]",
                   username, date, time, doctor, notes) == 5)
        {

            User *u = find_user(ht, username);
            if (!u)
                continue; // skip if user not found

            ReservationNode *res = (ReservationNode *)malloc(sizeof(ReservationNode));
            if (!res)
                continue;

            strcpy(res->patient_username, username);
            strcpy(res->date, date);
            strcpy(res->time, time);
            strcpy(res->doctor, doctor);
            strcpy(res->notes, notes);
            res->next = NULL;

            if (u->reservations_rear == NULL)
            {
                u->reservations_front = u->reservations_rear = res;
            }
            else
            {
                u->reservations_rear->next = res;
                u->reservations_rear = res;
            }
        }
    }

    fclose(file);
    puts("Reservations loaded from file.");
}

// Fungsi untuk AVL

int height(AVLNode *node)
{
    return node ? node->height : 0;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int get_balance(AVLNode *node)
{
    if (!node)
        return 0;
    return height(node->left) - height(node->right);
}

// Rotation
AVLNode *right_rotate(AVLNode *y)
{
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

AVLNode *left_rotate(AVLNode *x)
{
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

int compare_date(const char *d1, const char *d2)
{
    return strcmp(d1, d2); // since format is YYYY-MM-DD, lexicographic comparison works!
}

AVLNode *insert_avl(AVLNode *node, ReservationNode *res)
{
    if (!node)
    {
        AVLNode *new_node = (AVLNode *)malloc(sizeof(AVLNode));
        new_node->res = res;
        new_node->left = new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    int cmp = compare_date(res->date, node->res->date);

    if (cmp == 0)
        cmp = strcmp(res->time, node->res->time);
    if (cmp == 0)
        cmp = strcmp(res->doctor, node->res->doctor);
    if (cmp < 0)
        node->left = insert_avl(node->left, res);
    else if (cmp > 0)
        node->right = insert_avl(node->right, res);
    else
        return node; // duplicate date (optional)

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && compare_date(res->date, node->left->res->date) < 0)
        return right_rotate(node);
    if (balance < -1 && compare_date(res->date, node->right->res->date) > 0)
        return left_rotate(node);
    if (balance > 1 && compare_date(res->date, node->left->res->date) > 0)
    {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
    if (balance < -1 && compare_date(res->date, node->right->res->date) < 0)
    {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

void inorder_traversal_avl(AVLNode *node, int is_doctor_view)
{
    if (!node)
        return;

    inorder_traversal_avl(node->left, is_doctor_view);

    ReservationNode *res = node->res;
    if (is_doctor_view)
    {
        printf("Patient: %s\n", res->patient_username);
    }

    printf("Date: %s\nTime: %s\nDoctor: %s\nNotes: %s\n\n",
           res->date, res->time, res->doctor, res->notes);

    inorder_traversal_avl(node->right, is_doctor_view);
}

// Pause console untuk menunggu input dari user
void pause_console(void)
{
    printf("Press any key to continue...");
    _getch();
    printf("\n");
}

// ======================= [ADMIN FUNCTIONS] =======================

// Fungsi untuk menampilkan semua user
// yang ada di hash table
void view_all_users(hash_table *ht)
{
    puts("=== List of Users ===");
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        User *cur = ht->table[i];
        while (cur)
        {
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
void delete_user(hash_table *ht, const char *username)
{
    int idx = hash_function(username);
    User *cur = ht->table[idx];
    User *prev = NULL;

    while (cur)
    {
        if (strcmp(cur->username, username) == 0)
        {
            if (prev == NULL)
            {
                ht->table[idx] = cur->next;
            }
            else
            {
                prev->next = cur->next;
            }
            // Free all reservations
            ReservationNode *res = cur->reservations_front;
            while (res)
            {
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

void generate_report_with_heap(hash_table *ht)
{
    ReservationHeap heap;
    heap.size = 0;

    // Collect all reservations
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        User *u = ht->table[i];
        while (u)
        {
            ReservationNode *res = u->reservations_front;
            while (res)
            {
                insert_heap(&heap, res);
                res = res->next;
            }
            u = u->next;
        }
    }

    if (heap.size == 0)
    {
        puts("No reservations to report.");
        return;
    }

    puts("=== Upcoming Appointments Report ===");
    while (heap.size > 0)
    {
        HeapNode *node = extract_min(&heap);
        printf("Date: %s | Time: %s | Doctor: %s | Patient: %s | Notes: %s\n",
               node->res->date,
               node->res->time,
               node->res->doctor,
               node->res->patient_username,
               node->res->notes);
        free(node);
    }
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
void admin_menu(hash_table *ht)
{
    int choice;
    do
    {
        system("cls");
        puts("=== ADMIN MENU ===");
        puts("1. View Users");
        puts("2. Delete User");
        puts("3. Generate Report");
        puts("4. Rating Summary");
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
            view_all_users(ht);
            pause_console();
            break;
        case 2:
        {
            char uname[20];
            printf("Enter username to delete: ");
            scanf("%19s", uname);
            getchar();
            delete_user(ht, uname);
            pause_console();
            break;
        }
        case 3:
            generate_report_with_heap(ht);
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

// ======================= [CLIENT FUNCTIONS] =======================

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

void create_reservation(User *u, hash_table *ht)
{
    view_doctors_list(ht);
    ReservationNode *res = (ReservationNode *)malloc(sizeof(ReservationNode));
    if (!res)
    {
        puts("Memory allocation failed.");
        return;
    }

    puts("=== Create Reservation ===");

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

    res->next = NULL;
    strcpy(res->patient_username, u->username);
    if (u->reservations_rear == NULL)
    {
        u->reservations_front = u->reservations_rear = res;
    }
    else
    {
        u->reservations_rear->next = res;
        u->reservations_rear = res;
    }
    save_reservations_to_csv(ht, "reservations.csv");
    puts("Reservation created successfully!");
}

// Fungsi untuk menampilkan semua reservasi
// yang dimiliki oleh user
void view_reservation(User *u)
{
    AVLNode *root = NULL;
    ReservationNode *cur = u->reservations_front;

    while (cur)
    {
        root = insert_avl(root, cur);
        cur = cur->next;
    }

    if (!root)
    {
        puts("No reservations found.");
        return;
    }

    puts("=== Your Reservations (Sorted by Date) ===");
    inorder_traversal_avl(root, 0);
}

// Fungsi untuk membatalkan reservasi
//  User akan memilih nomor reservasi yang ingin dibatalkan
//  Jika nomor tidak valid, akan menampilkan pesan error
void cancel_reservation(User *u)
{
    if (u->reservations_front == NULL)
    {
        puts("No reservations to cancel.");
        return;
    }

    puts("=== Your Reservations ===");
    ReservationNode *res = u->reservations_front;
    int i = 1;
    while (res)
    {
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

// ======================= [DOCTOR FUNCTIONS] =======================

// Untuk liat list appointment Dokter A
void view_doctor_appointments(hash_table *ht, const char *doctor_name)
{
    AVLNode *root = NULL;

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
                    // Patient name is already filled when created
                    root = insert_avl(root, res);
                }
                res = res->next;
            }
            u = u->next;
        }
    }

    if (!root)
    {
        puts("No appointments found.");
        return;
    }

    puts("=== Appointments (Sorted by Date) ===");
    inorder_traversal_avl(root, 1);
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

// ======================= [AUTH FUNCTIONS] ========================

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
void login(hash_table *ht)
{
    char username[20], password[20];

    printf("Username: ");
    scanf("%19s", username);
    getchar();

    printf("Password: ");
    scanf("%19s", password);
    getchar();

    User *u = find_user(ht, username);
    if (u && strcmp(u->password, password) == 0)
    {
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
            doctor_menu(u, ht);
        }
        else
        {
            puts("Login successful as CLIENT.");
            pause_console();
            client_menu(u, ht);
        }
    }
    else
    {
        puts("Invalid username or password.");
        pause_console();
    }
}

// Fungsi untuk mendaftarkan user baru sebagai client
void register_client(hash_table *ht)
{
    char username[20], password[20];

    puts("=== Client Registration ===");
    printf("Choose username: ");
    scanf("%19s", username);
    getchar();

    if (find_user(ht, username))
    {
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

// ======================= [MAIN FUNCTION] =======================
int main()
{
    hash_table ht = {0};
    trie_root = create_trie_node();

    load_users_from_csv(&ht, "users.csv");
    load_reservations_from_csv(&ht, "reservations.csv");
    load_ratings_from_csv(&ht, "ratings.csv");

    // Use wrapper to keep hash and Trie in sync
    insert_user_and_trie(&ht, trie_root, "admin", "admin123", ROLE_ADMIN);
    insert_user_and_trie(&ht, trie_root, "drdoom", "dok123", ROLE_DOCTOR);
    insert_user_and_trie(&ht, trie_root, "drstrange", "123dok", ROLE_DOCTOR);
    insert_user_and_trie(&ht, trie_root, "alice", "1234", ROLE_CLIENT);

    int choice;
    do
    {
        system("cls");
        puts("=== Clinic System ===");
        puts("1. Register (Client)");
        puts("2. Login");
        puts("0. Exit");
        printf("Choose: ");
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
