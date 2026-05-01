#include "inventory.h"

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ *
 * Internal helpers                                                     *
 * ------------------------------------------------------------------ */

/* Return the total number of records (including deleted) in the file. */
static long record_count(FILE *fp)
{
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    return size / (long)sizeof(Item);
}

/*
 * Find the file offset of the record with the given id.
 * Returns the offset (>= 0) on success, or -1 if not found.
 * Sets *out_item when out_item != NULL.
 */
static long find_offset(FILE *fp, int id, Item *out_item)
{
    long count = record_count(fp);
    for (long i = 0; i < count; i++) {
        long offset = i * (long)sizeof(Item);
        fseek(fp, offset, SEEK_SET);
        Item tmp;
        if (fread(&tmp, sizeof(Item), 1, fp) != 1) continue;
        if (tmp.id == id) {
            if (out_item) *out_item = tmp;
            return offset;
        }
    }
    return -1L;
}

/* ------------------------------------------------------------------ *
 * Public C API                                                         *
 * ------------------------------------------------------------------ */

int add_item(const Item *item)
{
    if (!item) return 0;

    /* Check for duplicate id */
    FILE *fp = fopen(DB_FILENAME, "rb");
    if (fp) {
        Item dummy;
        if (find_offset(fp, item->id, &dummy) >= 0) {
            fclose(fp);
            return 0;   /* duplicate */
        }
        fclose(fp);
    }

    /* Append the new record */
    fp = fopen(DB_FILENAME, "ab");
    if (!fp) return 0;
    int ok = (fwrite(item, sizeof(Item), 1, fp) == 1);
    fclose(fp);
    return ok;
}

int get_item(int id, Item *out)
{
    if (!out) return 0;
    FILE *fp = fopen(DB_FILENAME, "rb");
    if (!fp) return 0;

    Item tmp;
    long offset = find_offset(fp, id, &tmp);
    fclose(fp);

    if (offset < 0 || tmp.is_deleted) return 0;
    *out = tmp;
    return 1;
}

int update_item(int id, const Item *updated)
{
    if (!updated) return 0;
    FILE *fp = fopen(DB_FILENAME, "r+b");
    if (!fp) return 0;

    Item existing;
    long offset = find_offset(fp, id, &existing);
    if (offset < 0 || existing.is_deleted) {
        fclose(fp);
        return 0;
    }

    /* Write the updated record back at the same offset */
    fseek(fp, offset, SEEK_SET);
    int ok = (fwrite(updated, sizeof(Item), 1, fp) == 1);
    fflush(fp);
    fclose(fp);
    return ok;
}

int delete_item(int id)
{
    FILE *fp = fopen(DB_FILENAME, "r+b");
    if (!fp) return 0;

    Item tmp;
    long offset = find_offset(fp, id, &tmp);
    if (offset < 0 || tmp.is_deleted) {
        fclose(fp);
        return 0;
    }

    tmp.is_deleted = 1;
    fseek(fp, offset, SEEK_SET);
    int ok = (fwrite(&tmp, sizeof(Item), 1, fp) == 1);
    fflush(fp);
    fclose(fp);
    return ok;
}

int list_items(Item *buffer, int max_items)
{
    if (!buffer || max_items <= 0) return 0;
    FILE *fp = fopen(DB_FILENAME, "rb");
    if (!fp) return 0;

    long count  = record_count(fp);
    int  copied = 0;

    for (long i = 0; i < count && copied < max_items; i++) {
        fseek(fp, i * (long)sizeof(Item), SEEK_SET);
        Item tmp;
        if (fread(&tmp, sizeof(Item), 1, fp) != 1) continue;
        if (!tmp.is_deleted) {
            buffer[copied++] = tmp;
        }
    }
    fclose(fp);
    return copied;
}
