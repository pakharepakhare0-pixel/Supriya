# Hybrid Inventory Manager

A console-based inventory manager that demonstrates C/C++ interoperability:
the **data layer** is written in **C** (structs + binary file I/O), and the
**UI layer** is written in **C++** (classes + STL).

---

## Project Structure

```
.
├── include/
│   ├── inventory.h          # C struct (Item) + extern "C" API declarations
│   └── InventoryManager.h   # C++ class declaration
├── src/
│   ├── inventory.c          # C backend: fread/fwrite/fseek binary storage
│   ├── InventoryManager.cpp # C++ UI: menu, validation, STL vector + sort
│   └── main.cpp             # Entry point
├── Makefile                 # Primary build (gcc + g++)
├── CMakeLists.txt           # Alternative build (CMake ≥ 3.12)
└── README.md
```

### Key design points

| Layer | Language | Responsibilities |
|-------|----------|-----------------|
| `inventory.c` | C (std=c11) | Binary file CRUD, `fseek`-based in-place update/delete, soft-delete flag |
| `InventoryManager.cpp` | C++ (std=c++17) | Menu loop, input validation, `std::vector` for buffering, `std::sort` for listing |
| `extern "C"` block | — | Lets C++ call the C functions without name-mangling |

---

## Build & Run

### Option A — Make (recommended)

```bash
make          # compiles and links
make run      # build + launch
make clean    # remove build artifacts and inventory.dat
```

### Option B — CMake

```bash
mkdir cmake-build && cd cmake-build
cmake ..
cmake --build .
./inventory_manager
```

> **Requirements:** GCC/G++ with C11 and C++17 support (GCC 7+ or Clang 5+).

---

## Menu

```
1. Add item
2. View item
3. Update item
4. Delete item
5. List all items
6. Exit
```

All input is validated before any file operation:
- ID must be a positive integer
- Quantity must be ≥ 0
- Price must be ≥ 0.0
- Name must not be empty
- Duplicate IDs are rejected

---

## Test Cases

### Test 1 — Add items and verify persistence
**Steps:** Add items with IDs 1, 2, 3. Choose Exit. Re-run the program. Choose List all.  
**Expected:** All three items appear in the listing sorted by ID.  
**Result:** ✅ Pass — binary file retains all records across restarts.

### Test 2 — Duplicate ID rejected
**Steps:** With item ID 2 already in the file, attempt to add a second item with ID 2.  
**Expected:** `[FAIL] Could not add item (duplicate ID or I/O error).`  
**Result:** ✅ Pass — `find_offset` detects the collision before writing.

### Test 3 — Update persists after restart
**Steps:** Update item 2 (new name "Gadget B UPDATED", qty 99, price 49.95). Exit. Re-run. View item 2.  
**Expected:** Updated values are displayed.  
**Result:** ✅ Pass — `fseek` + `fwrite` overwrites the correct record in-place.

### Test 4 — Soft-delete hides item from list and view
**Steps:** Delete item 1. Choose List all. Choose View item → enter ID 1.  
**Expected:** Item 1 does not appear in the list; View returns "not found or deleted".  
**Result:** ✅ Pass — `is_deleted = 1` flag is checked in both `list_items` and `get_item`.

### Test 5 — Invalid input does not crash
**Steps:** At the "Enter ID" prompt type `-5`, then `abc`, then `0`, then finally `4`.  
**Expected:** Each bad entry prints an error and re-asks; the program continues normally after a valid ID is entered.  
**Result:** ✅ Pass — the `prompt_positive_id` helper loops until valid input is received, using `cin.clear()` + `ignore()` to flush the stream.

---

## Storage Format

Data is stored in `inventory.dat` as a flat array of fixed-size `Item` records
(no padding surprises — all fields are native types):

```c
typedef struct {
    int   id;           // 4 bytes
    char  name[40];     // 40 bytes
    int   quantity;     // 4 bytes
    float price;        // 4 bytes
    int   is_deleted;   // 4 bytes  (0=active, 1=deleted)
} Item;               // 56 bytes total per record
```

Record at index *i* is always at file offset `i × sizeof(Item)`, making
`fseek`-based random access O(1).
