
# 🏥 Clinic Reservation System (C Language Project)

Sistem Reservasi Klinik berbasis C yang mendukung peran Admin, Dokter, dan Pengguna (Pasien), dengan berbagai fitur seperti antrian reservasi, AVL tree untuk penyortiran tanggal, Trie untuk pencarian dokter, dan Heap untuk laporan.

## 📦 Fitur Utama

### 👨‍⚕️ Admin
- Melihat daftar semua pengguna
- Menghapus pengguna dan data reservasinya
- Menampilkan laporan janji mendatang (menggunakan Heap)
- Melihat rekap penilaian dokter (menggunakan Trie)

### 🧑‍💻 Pengguna (Pasien)
- Registrasi dan login
- Melihat daftar dokter yang tersedia
- Membuat reservasi ke dokter
- Melihat dan membatalkan reservasi (dengan AVL Tree)
- Memberi penilaian kepada dokter
- Data disimpan otomatis ke file `.csv`

### 🩺 Dokter
- Melihat daftar janji dari pasien (dengan AVL Tree)
- Mengatur ketersediaan online/offline

## 🛠 Struktur Data yang Digunakan

- **Hash Table**: Menyimpan dan mencari pengguna berdasarkan username
- **Queue (Linked List)**: Menyimpan daftar reservasi tiap pengguna
- **AVL Tree**: Menyortir janji berdasarkan tanggal & waktu
- **Heap**: Menghasilkan laporan janji terdekat
- **Trie**: Mencari dokter berdasarkan prefix nama
- **Linked List** : Digunakan dalam hashmap ketika colission terjadi, maka akan langsung chaining

## 📁 Struktur File

- `clinic.cpp` — File utama proyek
- `users.csv` — Data login pengguna
- `reservations.csv` — Data reservasi pasien
- `ratings.csv` — Data penilaian untuk dokter

## 💻 Cara Menjalankan

1. Compile:
   ```bash
   gcc clinic_final_indonesian.cpp -o clinic -std=c99
   ```

2. Jalankan:
   ```bash
   ./clinic
   ```

> 💡 **Catatan**: Pastikan file `users.csv` dan `reservations.csv` tersedia di direktori saat program berjalan. Jika tidak, sistem akan mulai dari nol.

## 🗃 Contoh Akun Bawaan

| Username   | Password   | Role     |
|------------|------------|----------|
| admin      | admin123   | Admin    |
| drdoom     | dok123     | Dokter   |
| drstrange  | 123dok     | Dokter   |
| alice      | 1234       | Client   |

## 📌 Catatan Tambahan

- Program menggunakan Bahasa Indonesia untuk seluruh antarmuka pengguna.
- Semua data disimpan secara **persistent** menggunakan file `.csv`.
- Mendukung konsol Windows (`conio.h`, `windows.h`). Untuk Linux, sesuaikan bagian `getch()`.

## 📚 Pembelajaran

Proyek ini cocok untuk memahami implementasi:
- Struktur data kompleks di dunia nyata
- Pemrosesan file
- Simulasi peran pengguna dalam sistem informasi

---

🧠 Dibuat sebagai bagian dari Proyek Akhir Struktur Data.  
Selamat datang di Klinik Digital Anda! 💉
