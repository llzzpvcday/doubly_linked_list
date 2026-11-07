#include "dlinked_list.h"
#include "dllist_logging.c"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

void __dllist_check(dllist_t *dllist);
void __dllist_status_handler(dllist_t *dllist);
bool __dllist_check_index(dllist_t *dllist, ssize_t index);
void __dllist_resize(dllist_t *dllist);
ssize_t __dllist_get_free(dllist_t *dllist);
void __popen2(const char *__command, FILE **stdin, FILE **stdout);

#define __next(index__) dllist->buf[index__].next
#define __prev(index__) dllist->buf[index__].prev

void __dllist_check(dllist_t *dllist) {
  #define ret_status(status__) {dllist->status = status__; free(mark); return;}

  if (dllist->size > dllist->capacity) {
    dllist->status = DLLIST_SIZE_GREATER_CAPACITY;
    return;
  }

  int8_t *mark = (int8_t *)calloc((size_t)dllist->capacity, sizeof(int8_t));
  ssize_t i = 0;

  do {
    if (mark[i] > 0)
      ret_status(DLLIST_ELEM_LOOP_DETECTED);

    mark[i] += 1;
    
    ssize_t next = __next(i);
    if (__dllist_check_index(dllist, next)) {
      if (__prev(next) == -1){
        ret_status(DLLIST_ONE_WAY_LINK_DETECTED);
      }
      if (__dllist_check_index(dllist, __prev(next))) {
        if (__prev(next) != i)
        ret_status(DLLIST_ONE_WAY_LINK_DETECTED);
      } else {
        ret_status(DLLIST_INVALID_LINK_DETECTED);
      }
    } else {
      ret_status(DLLIST_INVALID_LINK_DETECTED);
    }     

    i = next;

  } while (i != 0);

  i = dllist->free;
  while (i != 0) {
    if (mark[i] < 0) {
      ret_status(DLLIST_FREE_LOOP_DETECTED);
    } else if (mark[i] > 0) {
      ret_status(DLLIST_NOT_FREE_ELEM_IN_FREE_LIST);
    }
    mark[i] -= 1;

    ssize_t next = __next(i);
    if (!__dllist_check_index(dllist, next)) {
      ret_status(DLLIST_INVALID_LINK_DETECTED);
    }
    i = next;
  }

  for (ssize_t j = 0; j < dllist->capacity; j += 1) {
    if (mark[j] == 0) {
      ret_status(DLLIST_NOT_LINKED_NODE_DETECTED);
    }
  }

  ret_status(DLLIST_OK);

  #undef ret_status
}

void __dllist_status_handler(dllist_t *dllist) {
  int ret = -1;
  if (dllist->status_handler != NULL) {
    ret = dllist->status_handler(dllist);
  }

  if (ret == 0)
    return;

  switch (dllist->status) {

  case DLLIST_OK:
    $debug("Dllist check ok");
    break;
  case DLLIST_NOT_INITIALIZED:
    $error("Operation with not initialized dllist (exiting program)");
    exit(DLLIST_NOT_INITIALIZED);
  case DLLIST_BUF_ALLOC_FAILED:
    $error("Allocation of buffer for dllist failed (exiting program)");
    exit(DLLIST_BUF_ALLOC_FAILED);
  case DLLIST_INVALID_INDEX_OPERATION:
    $error("Attempt to perform operation with invalid index (exiting program)");
    exit(DLLIST_INVALID_INDEX_OPERATION);
  case DLLIST_ATTEMPT_TO_REMOVE_ZERO:
    $error("Attempt to remove sentinel element (exiting program)");
    exit(DLLIST_ATTEMPT_TO_REMOVE_ZERO);
  case DLLIST_ELEM_LOOP_DETECTED:
    $error("Dllist checker detected a loop (exiting program)");
    exit(DLLIST_ELEM_LOOP_DETECTED);
  case DLLIST_FREE_LOOP_DETECTED:
    $error("Dllist checker detected a loop of free elements (exiting program)");
    exit(DLLIST_FREE_LOOP_DETECTED);
  case DLLIST_NOT_LINKED_NODE_DETECTED:
    $error("Dllist checker found a node that is not connected to any other one. (exiting program)");
    exit(DLLIST_NOT_LINKED_NODE_DETECTED);
  case DLLIST_ONE_WAY_LINK_DETECTED:
    $error("Dllist checker found a one way link (exiting program)");
    exit(DLLIST_ONE_WAY_LINK_DETECTED);
  case DLLIST_INVALID_LINK_DETECTED:
    $error("Dllist checker found a link with invalid index (exiting program)");
    exit(DLLIST_INVALID_LINK_DETECTED);
  case DLLIST_NOT_FREE_ELEM_IN_FREE_LIST:
    $error("Dllist checker found a not free node in free list (exiting program)");
    exit(DLLIST_NOT_FREE_ELEM_IN_FREE_LIST);
  case DLLIST_SIZE_GREATER_CAPACITY:
    $error("Dllist size is greater than capacity (exiting program)");
    exit(DLLIST_SIZE_GREATER_CAPACITY);
    break;
  default:
    break;
  }
}

bool __dllist_check_index(dllist_t *dllist, ssize_t index) {
  return ((ssize_t)index < dllist->capacity) && (index >= 0);
}

#define DLLIST_CHECK {__dllist_check(dllist); __dllist_status_handler(dllist);}
#define DLLIST_SET_STATUS(status__) {dllist->status = status__; __dllist_status_handler(dllist);}

#define DLLIST_CHECK_OPERATION_INDEX(index__) {if (!__dllist_check_index(dllist, index__) || (__next(index) == -1 || __prev(index) == -1)) DLLIST_SET_STATUS(DLLIST_INVALID_INDEX_OPERATION);}

// Source - https://stackoverflow.com/a/64359731
// Posted by Hank
// Retrieved 2026-10-31, License - CC BY-SA 4.0
// Like popen(), but returns two FILE*: child's stdin and stdout, respectively.
void __popen2(const char *__command, FILE **stdin, FILE **stdout) {
  // pipes[0]: parent writes, child reads (child's stdin)
  // pipes[1]: child writes, parent reads (child's stdout)
  int pipes[2][2];

  pipe(pipes[0]);
  pipe(pipes[1]);

  if (fork() > 0) {
    // parent
    close(pipes[0][0]);
    close(pipes[1][1]);

    *stdin = fdopen(pipes[0][1], "w");
    *stdout = fdopen(pipes[1][0], "r");
    return;
  } else {
    // child
    close(pipes[0][1]);
    close(pipes[1][0]);

    dup2(pipes[0][0], STDIN_FILENO);
    dup2(pipes[1][1], STDOUT_FILENO);

    execl("/bin/sh", "/bin/sh", "-c", __command, NULL);

    exit(1);
  }
}

void dllist_dump(dllist_t *dllist, FILE *out) {
  FILE *child_in = {};
  FILE *child_out = {};
  __popen2("/usr/bin/dot -Tsvg", &child_in, &child_out);

  fprintf(child_in, "digraph G{\nrankdir=LR;\n");
  fprintf(child_in, "bgcolor=\"transparent\";\n");
  fprintf(child_in, "graph[splines=ortho];\n");
  fprintf(child_in, "graph[fontname=\"Helvetica, Arial, sans-serif\"];\n");
  fprintf(child_in, "node[style=\"filled,rounded\"; fontname=\"Helvetica, Arial, sans-serif\"; fontcolor=\"#1E293B\"; penwidth=\"2.0\"];\n");
;
  fprintf(child_in, "edge[color=\"#64748B\"; penwidth=\"1.5\";];\n");
  for (ssize_t i = 0; i < dllist->capacity; i += 1) {


#define print_node(color1, color2)                                             \
  {                                                                            \
    fprintf(child_in,                                                          \
        "node%1$ld[shape=\"Mrecord\", color=\"" color1 "\", "                  \
        "style=\"filled\", fillcolor=\"" color2 "\", "                         \
        "label=\" index=%1$ld | { prev=%2$ld | next=%3$ld } | data=%4$d "      \
        "\"];\n",                                                              \
        i, cur.prev, cur.next, cur.data);                                      \
  }

    dllist_node cur = dllist->buf[i];
    if (i == 0) {
      print_node("#4F46E5", "#EEF2FF");
    } else if (__dllist_check_index(dllist, cur.next) && __dllist_check_index(dllist, cur.prev)) {
      print_node("#059669", "#ECFDF5");
    } else if (__dllist_check_index(dllist, cur.next) && cur.prev == -1) {
      print_node("#D97706", "#FFFBEB");
    } else {
      print_node("#E11D48", "#FFF1F2");
    }

    if (!__dllist_check_index(dllist, cur.next)) {
      fprintf(child_in, "node%ld -> node%ld[color=\"#F43F5E\", style=\"bold,dashed\"];", i, cur.next);
    } else if (cur.prev == -1) {
       fprintf(child_in, "node%ld -> node%ld[color=\"#F59E0B\", style=\"dashed\"];", i, cur.next);
    } else if (__prev(cur.next) == i) {
      fprintf(child_in, "node%ld -> node%ld[dir=\"both\"];", i, cur.next);
    } else {
      fprintf(child_in, "node%ld -> node%ld[color=\"#F43F5E\", style=\"bold,dashed\"];", i, cur.next);
    }

    if (cur.prev != -1) {
      if (!__dllist_check_index(dllist, cur.prev) || __next(cur.prev) != i) {
        fprintf(child_in, "node%ld -> node%ld[color=\"#F43F5E\", style=\"bold,dashed\"];", i, cur.prev);
      }
    }

  }

  fprintf(child_in, "free -> node%ld;", dllist->free);
  fprintf(child_in, "head -> node%ld;", __next(0));
  fprintf(child_in, "tail -> node%ld;", __prev(0));

  fprintf(child_in, "}");
  fclose(child_in);

  int c = '\0';
  while ((c = fgetc(child_out)) != EOF) {
    if (c == '\n') {
      fputc(' ', out);
    } else {
      fputc(c, out);
    }
  }
  fputc('\n', out);
}

ssize_t dllist_init(dllist_t *dllist, ssize_t base_capacity) {
  static ssize_t next_id = 0;
  if (base_capacity < DLLIST_MIN_CAP)
    base_capacity = DLLIST_MIN_CAP;

  base_capacity += 1;

  dllist->status    = DLLIST_OK;
  dllist->size      = 0;
  dllist->capacity  = base_capacity;
  dllist->buf       = (dllist_node *)calloc((size_t)base_capacity, sizeof(dllist_node));
  dllist->dllist_id = next_id;

  for (ssize_t i = 1; i < dllist->capacity - 1; i += 1) {
    __next(i) = i + 1;
    __prev(i) = -1;
  }
  __next(dllist->capacity - 1) = 0;
  __prev(dllist->capacity - 1) = -1;

  dllist->free = 1;

  __next(0) = 0;
  __prev(0) = 0;

  $debug("Dllist initialized");
  DLLIST_CHECK;
  $DLLIST_DEBUG_DUMP("Dump after <font color='gray'>INIT</font>");
  return next_id++;
}

void dllist_destruct(dllist_t *dllist) {
  free(dllist->buf);
  dllist->buf      = NULL;
  dllist->size     = 0;
  dllist->capacity = 0;
  dllist->status   = DLLIST_NOT_INITIALIZED;
}

void __dllist_resize(dllist_t *dllist) {
  DLLIST_CHECK;

  dllist->buf = (dllist_node *)realloc(dllist->buf, (size_t)dllist->capacity * sizeof(dllist_node) * 2 );
  if (dllist->buf == NULL)
    DLLIST_SET_STATUS(DLLIST_BUF_ALLOC_FAILED);

  for (ssize_t i = dllist->capacity; i < dllist->capacity * 2; i += 1) {
    __next(i) = i + 1;
    __prev(i) = -1;
    dllist->buf[i].data = 0;
  }


  __next(dllist->capacity * 2 - 1) = dllist->free;
  __prev(dllist->capacity * 2 - 1) = -1;

  dllist->free = dllist->capacity;

  dllist->capacity *= 2;

  DLLIST_CHECK;
  $DLLIST_DEBUG_DUMP("Dump after <font color='orange'>RESIZE</font>");
}

ssize_t __dllist_get_free(dllist_t *dllist) {
  if (dllist->free == 0) 
    __dllist_resize(dllist);
  
  
  ssize_t ret = dllist->free;
  dllist->free = __next(ret); 

  return ret;
}

ssize_t dllist_insert(dllist_t *dllist, ssize_t index, dllist_elem_t elem) {
  DLLIST_CHECK;

  DLLIST_CHECK_OPERATION_INDEX(index);
  
  ssize_t next = __next(index);
  ssize_t free  = __dllist_get_free(dllist);

  __prev(free) = index;
  __next(free) = next;
  __prev(next) = free;
  __next(index) = free;

  dllist->buf[free].data = elem;

  DLLIST_CHECK;
  $DLLIST_DEBUG_DUMP("Dump after <font color='blue'>INSERT</font> index: %ld", index);

  return free;
}

void dllist_remove(dllist_t *dllist, ssize_t index) {
  DLLIST_CHECK;
   
  DLLIST_CHECK_OPERATION_INDEX(index);
  if (index == 0) 
    DLLIST_SET_STATUS(DLLIST_ATTEMPT_TO_REMOVE_ZERO);

  ssize_t next = __next(index);
  ssize_t prev = __prev(index);
  __prev(index) = -1;
  __next(index) = dllist->free;
  dllist->buf[index].data = 0;

  dllist->free = index;

  __prev(next) = prev;
  __next(prev) = next;

  DLLIST_CHECK;
  $DLLIST_DEBUG_DUMP("Dump after <font color='deeppink'>REMOVE</font> index: %ld", index);
}

ssize_t dllist_get_head(dllist_t *dllist) {
  DLLIST_CHECK;
  return __next(0);
}

ssize_t dllist_get_tail(dllist_t *dllist){
  DLLIST_CHECK;
  return __prev(0);
}

dllist_elem_t dllist_get(dllist_t *dllist, ssize_t index) {
  DLLIST_CHECK;

  DLLIST_CHECK_OPERATION_INDEX(index);

  return dllist->buf[index].data;
}

ssize_t dllist_get_next(dllist_t *dllist, ssize_t index) {
  return __next(index);
}

ssize_t dllist_get_prev(dllist_t *dllist, ssize_t index) {
  return __prev(index);
}

ssize_t dllist_insert_before(dllist_t *dllist, ssize_t index, dllist_elem_t elem) {
  return dllist_insert(dllist, __prev(index), elem);
}


void dllist_set_status_handler(dllist_t *dllist, int (*status_handler)(struct dllist_t *dllist)) {
  dllist->status_handler = status_handler;
}
