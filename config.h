/*
 * config.h for Raspberry Pi Pico (RP2040)
 */

#ifndef CONFIG_H
#define CONFIG_H

/* C99 / Standard C */
#define STDC_HEADERS 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_MEMSET 1
#define HAVE_STRCHR 1
#define HAVE_STRERROR 1
#define HAVE_VPRINTF 1
#define HAVE__BOOL 1

/* Curses provided by Pico Curses layer */
#define HAVE_CURSES_H 1

/* Disable Unix/POSIX specific headers and syscalls */
#undef HAVE_PWD_H
#undef HAVE_TERMIOS_H
#undef HAVE_TERM_H
#undef HAVE_NCURSES_TERM_H
#undef HAVE_SYS_UTSNAME_H
#undef HAVE_ARPA_INET_H
#undef HAVE_UNISTD_H
#undef HAVE_SYS_IOCTL_H
#undef HAVE_UTMP_H
#undef HAVE_NLIST_H
#undef HAVE_PROCESS_H

#undef HAVE_FORK
#undef HAVE_VFORK
#undef HAVE_WORKING_FORK
#undef HAVE_WORKING_VFORK
#undef HAVE_SPAWNL
#undef HAVE__SPAWNL

#undef HAVE_GETUID
#undef HAVE_GETGID
#undef HAVE_SETUID
#undef HAVE_SETGID
#undef HAVE_SETREUID
#undef HAVE_SETREGID
#undef HAVE_GETPASS
#undef HAVE_GETPWUID
#undef HAVE_GETLOADAVG
#undef HAVE_ALARM
#undef HAVE_ESCDELAY
#undef HAVE_KILLCHAR
#undef HAVE_ERASECHAR

#undef CHECKTIME
#undef SCOREFILE
#undef LOCKFILE
#undef MASTER
#undef ALLSCORES

#define NUMNAME "Ten"
#define NUMSCORES 10
#define PACKAGE_NAME "Rogue"
#define PACKAGE_STRING "Rogue 5.4.4 (Raspberry Pi Pico)"
#define PACKAGE_VERSION "5.4.4"

#endif /* CONFIG_H */
