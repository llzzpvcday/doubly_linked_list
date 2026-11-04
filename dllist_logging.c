#include <stdio.h>
#include <time.h>

#define DLLIST_DEBUG

#ifdef DLLIST_DEBUG

FILE *__dllist_get_logging_file() {
  static FILE *log_file = NULL;
  
  if (log_file == NULL) {
    char log_filename_buf[64] = "dllist_log.html";
    time_t now   = time(NULL);
    struct tm *t = localtime(&now);
    strftime(log_filename_buf, sizeof(log_filename_buf), "%H:%M-%d-%m-%Y-log.html", t);
    log_file = fopen(log_filename_buf, "w");
    setvbuf(log_file, NULL, _IONBF, 0);
    // fputs("<pre><meta name    = \"color-scheme\" content = \"light dark\" /> ", log_file);
    fputs("<pre>", log_file);
  }

  return log_file;
}

#define $log(...)                                                              \
  {                                                                            \
    FILE *log_file = __dllist_get_logging_file();                              \
    fprintf(log_file, __VA_ARGS__);                                            \
    fflush(log_file);                                                          \
  }

#define $DLLIST_DEBUG_DUMP(...)                                                \
  {                                                                            \
    FILE *log_file = __dllist_get_logging_file();                              \
    fprintf(log_file, "<h1>");                                                 \
    fprintf(log_file, __VA_ARGS__);                                            \
    fprintf(log_file, "</h1>");                                                \
    dllist_dump(dllist, log_file);                                             \
    fflush(log_file);                                                          \
  }

#else

#endif
