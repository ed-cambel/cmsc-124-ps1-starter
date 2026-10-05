/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

#define DT_MAP_NUM_BUCKETS 16

typedef struct dt_entry{
    char *key;
    dt_value v;
    struct dt_entry *next;
} dt_entry;

struct dt_map {
    dt_entry *buckets[DT_MAP_NUM_BUCKETS];
    char **order;
    size_t len;
    size_t capacity;
};

/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    dt_map *m = calloc(1, sizeof(dt_map));
    if(!m){
        return NULL;
    }
    m->capacity = 8;
    m->order = malloc(m->capacity * sizeof(char*));
    if (!m->order){
        free(m);
        return NULL;
    }
    m->len = 0;
    return m;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    if(!m){
        return;
    }
    for (size_t i = 0; i < DT_MAP_NUM_BUCKETS; i++){
        dt_entry *curr = m->buckets[i];
        while(curr){
            dt_entry *next = curr->next;
            free(curr->key);
            free(curr);
            curr = next;
        }
    }
    free(m->order);
    free(m);
}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
   if(!m){
    return 0;
   }
   return m->len;
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    if(!m || !key){
        return DT_ERR_CAPACITY;
    }

    unsigned long long h = 14695981039346656037ULL;
    for(const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++){
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    size_t bucket = (size_t)(h%DT_MAP_NUM_BUCKETS);

    /* Search bucket for existing key */
    for (dt_entry *curr = m->buckets[bucket]; curr != NULL; curr = curr->next) {
        if (strcmp(curr->key, key) == 0) {
            curr->v = v;
            return DT_OK;
        }
    }

    /* Expand insertion order array if capacity is reached */
    if (m->len >= m->capacity) {
        size_t new_cap = m->capacity == 0 ? 8 : m->capacity * 2;
        char **new_order = realloc(m->order, new_cap * sizeof(char *));
        if (!new_order) {
            return DT_ERR_CAPACITY;
        }
        m->order = new_order;
        m->capacity = new_cap;
    }

    /* Duplicate key string */
    size_t key_len = strlen(key) + 1;
    char *key_copy = malloc(key_len);
    if (!key_copy) {
        return DT_ERR_CAPACITY;
    }
    memcpy(key_copy, key, key_len);

    /* Allocate entry node */
    dt_entry *entry = malloc(sizeof(dt_entry));
    if (!entry) {
        free(key_copy);
        return DT_ERR_CAPACITY;
    }

    entry->key = key_copy;
    entry->v = v;
    entry->next = m->buckets[bucket];
    m->buckets[bucket] = entry;

    m->order[m->len] = key_copy;
    m->len++;

    return DT_OK;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    if (!m || !key || !out) {
        return DT_ERR_KEY;
    }

    /* 64-bit FNV-1a Hash */
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    size_t bucket = (size_t)(h % DT_MAP_NUM_BUCKETS);

    for (const dt_entry *curr = m->buckets[bucket]; curr != NULL; curr = curr->next) {
        if (strcmp(curr->key, key) == 0) {
            *out = curr->v;
            return DT_OK;
        }
    }

    return DT_ERR_KEY;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    if (!m || !key) {
        return DT_ERR_KEY;
    }

    /* 64-bit FNV-1a Hash */
    unsigned long long h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= (unsigned long long)*p;
        h *= 1099511628211ULL;
    }
    size_t bucket = (size_t)(h % DT_MAP_NUM_BUCKETS);

    dt_entry **prev = &m->buckets[bucket];
    dt_entry *curr = *prev;

    while (curr && strcmp(curr->key, key) != 0) {
        prev = &curr->next;
        curr = *prev;
    }

    if (!curr) {
        return DT_ERR_KEY;
    }

    *prev = curr->next;

    /* Remove key reference from order array */
    size_t ord_idx = 0;
    for (; ord_idx < m->len; ord_idx++) {
        if (m->order[ord_idx] == curr->key) {
            break;
        }
    }

    if (ord_idx < m->len) {
        memmove(&m->order[ord_idx], &m->order[ord_idx + 1], (m->len - ord_idx - 1) * sizeof(char *));
    }

    m->len--;
    free(curr->key);
    free(curr);

    return DT_OK;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    if (!m || !out || index >= m->len) {
        return DT_ERR_RANGE;
    }

    *out = m->order[index];
    return DT_OK;
}
