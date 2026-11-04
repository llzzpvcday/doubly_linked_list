#include <stddef.h>
#include <stdio.h>

#define dllist_elem_t  int
#define DLLIST_MIN_CAP 6

typedef enum dllist_status {
  DLLIST_OK                         =  1,
  DLLIST_NOT_INITIALIZED            =  0,
  DLLIST_BUF_ALLOC_FAILED           = -1,
  DLLIST_INVALID_INDEX_OPERATION    = -2,
  DLLIST_ATTEMPT_TO_REMOVE_ZERO     = -3,
  DLLIST_ELEM_LOOP_DETECTED         = -4,
  DLLIST_FREE_LOOP_DETECTED         = -5,
  DLLIST_NOT_LINKED_NODE_DETECTED   = -6, 
  DLLIST_ONE_WAY_LINK_DETECTED      = -7,
  DLLIST_INVALID_LINK_DETECTED      = -8,
  DLLIST_NOT_FREE_ELEM_IN_FREE_LIST = -9,
  DLLIST_SIZE_GREATER_CAPACITY      = 10,
} dllist_status;

typedef struct dllist_node {
  dllist_elem_t data;
  ssize_t next;
  ssize_t prev;
} dllist_node;

typedef struct dllist_t {
  dllist_node *buf;
  ssize_t size;
  ssize_t capacity;
  dllist_status status;
  int (*status_handler)(struct dllist_t *dllist); // If handler returns zero,
                                                  // default handler will not be executed
  ssize_t free;
} dllist_t;

void dllist_init(dllist_t *dllist, ssize_t base_capacity);
void dllist_destruct(dllist_t *dllist);
void dllist_insert(dllist_t *dllist, ssize_t index, dllist_elem_t elem);
void dllist_remove(dllist_t *dllist, ssize_t index);
ssize_t dllist_get_head(dllist_t *dllist);
ssize_t dllist_get_tail(dllist_t *dllist);
dllist_elem_t dllist_get(dllist_t *dllist, ssize_t index);
void dllist_dump(dllist_t *dllist, FILE *out);

#define get_next(index__) dllist->buf[index__].next
#define get_prev(index__) dllist->buf[index__].prev
