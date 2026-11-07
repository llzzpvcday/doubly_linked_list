#include <stdio.h>
#include <time.h>

#define DLLIST_LOGGING
#define DLLIST_DEBUG
// #define DLLIST_LOGFILE_TIMESTAMP
// #define DLLIST_LOG_DARK_THEME
// #define DLLIST_TABLE_DUMP

// ------------------LOGGING-MACRO-------------------------------------------

#ifdef DLLIST_LOGGING

void get_cur_time_str(char *buf, size_t size, const char *format) {
  time_t now   = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buf, size, format, t);
}

FILE *__dllist_get_logging_file() {
  static FILE *log_file = NULL;

  if (log_file == NULL) {
    char log_filename_buf[64] = "dllist_log.html";

    #ifdef DLLIST_LOGFILE_TIMESTAMP
    get_cur_time_str(log_filename_buf, 64, "%H:%M-%d-%m-%Y-log.html");
    #endif
    
    log_file = fopen(log_filename_buf, "w");
    setvbuf(log_file, NULL, _IONBF, 0);

    #ifdef DLLIST_LOG_DARK_THEME
    fputs("<pre><meta name    = \"color-scheme\" content = \"light dark\" /> ", log_file);
    #endif
    
    fputs("<pre>\n", log_file);
    fputs("<style> h1 { margin: 0px } tr:nth-child(odd) { background-color: rgba(236, 253, 245, 0.8); } th:nth-child(odd),td:nth-child(odd) { background-color: rgba(236, 253, 245, 0.8); } table { table-layout:fixed; } td {width:4em; font-size: 1.5em; text-align: center;}</style>", log_file);
  }

  return log_file;
}

#define $log(...)                                                              \
  {                                                                            \
    FILE *log_file = __dllist_get_logging_file();                              \
    fprintf(log_file, __VA_ARGS__);                                            \
    fflush(log_file);                                                          \
  }

#define $error(...)                                                            \
  {                                                                            \
    FILE *log_file         = __dllist_get_logging_file();                      \
    char timestamp_buf[32] = "";                                               \
    get_cur_time_str(                                                          \
        timestamp_buf, sizeof(timestamp_buf), "%d-%m-%Y %H:%M:%S");            \
    fprintf(log_file,                                                          \
        "<h1><font color=\"red\">%s - [ERROR] - [%3ld] - ",                    \
        timestamp_buf,                                                         \
        dllist->dllist_id);                                                    \
    fprintf(log_file, __VA_ARGS__);                                            \
    fprintf(log_file, "</font></h1>\n");                                       \
    dllist_dump_to_html_table(dllist, log_file);                               \
    dllist_dump(dllist, log_file);                                             \
    fflush(log_file);                                                          \
  }

#else

#define $log(...)
#define $DLLIST_DEBUG_DUMP(...)
#define $error(...)

#endif

// ------------------DEBUG-MACRO---------------------------------------------

#if defined(DLLIST_DEBUG) && defined(DLLIST_LOGGING)

#define $DLLIST_DEBUG_DUMP(...)                                                \
  {                                                                            \
    FILE *log_file = __dllist_get_logging_file();                              \
    fprintf(log_file, "<h1>");                                                 \
    fprintf(log_file, __VA_ARGS__);                                            \
    fprintf(log_file,                                                          \
        "</h1>                                                               " \
        "                                                              <br>"); \
    dllist_dump_to_html_table(dllist, log_file);                               \
    dllist_dump(dllist, log_file);                                             \
    fflush(log_file);                                                          \
  }

#define $debug(...)                                                            \
  {                                                                            \
    FILE *log_file         = __dllist_get_logging_file();                      \
    char timestamp_buf[32] = "";                                               \
    get_cur_time_str(                                                          \
        timestamp_buf, sizeof(timestamp_buf), "%d-%m-%Y %H:%M:%S");            \
    fprintf(log_file,                                                          \
        "<h1><font color=\"green\">%s - [DEBUG] - [%3ld] - ",                 \
        timestamp_buf,                                                         \
        dllist->dllist_id);                                                    \
    fprintf(log_file, __VA_ARGS__);                                            \
    fprintf(log_file, "</font></h1>\n");                                       \
    fflush(log_file);                                                          \
  }

#else 

#define $DLLIST_DEBUG_DUMP(...)
#define $debug(...)

#endif
