/* this code is adapted from showkey(1) from console-tools 0.2.3
 * original Copyright (C) 1993 Risto Kankkunen.
 * modifications for MatPLC (C) 2000 Jiri Baum.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sysexits.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <sys/ioctl.h>

/* #include <lct/local.h> What is this supposed to do? --Jiri  */
/* #include <lct/utils.h> What is this supposed to do? --Mario */
#include <lct/console.h> 

/* The above file is not found in default installs of Debian and Red Hat  */
/* For debian run as root:                                                */
/*  #apt-get install console-tools-dev                                    */
/* For Red Hat, run as root:                                              */
/*  #???? ????????????                                                    */

#include <plc.h>

static int fd;
static int oldkbmode;
static struct termios old;
static int quiet = 0;

inline const char *_(const char *s)
{
  return quiet ? "" : s;
}

static void get_mode(void)
{
  const char *m;

  if (ioctl(fd, KDGKBMODE, &oldkbmode)) {
    perror("KDGKBMODE");
    exit(1);
  }
  switch (oldkbmode) {
  case K_RAW:
    m = "RAW";
    break;
  case K_XLATE:
    m = "XLATE";
    break;
  case K_MEDIUMRAW:
    m = "MEDIUMRAW";
    break;
  case K_UNICODE:
    m = "UNICODE";
    break;
  default:
    m = _("?UNKNOWN?");
    break;
  }
  printf(_("kb mode was %s\n"), m);
  if (oldkbmode != K_XLATE) {
    printf(_("[ if you are trying this under X, it might not work\n"));
    printf(_("since the X server is also reading /dev/console ]\n"));
  }
  printf(_("\n"));
}

static void clean_quit(int x)
{
  if (ioctl(fd, KDSKBMODE, oldkbmode)) {
    perror("KDSKBMODE");
    exit(1);
  }
  if (tcsetattr(fd, 0, &old) == -1)
    perror("tcsetattr");
  close(fd);
  plc_done();
  exit(x);
}

static void die(int x)
{
  printf(_("caught signal %d, cleaning up...\n"), x);
  clean_quit(1);
}

static void read_cfg(plc_pt_t pts[128])
{
  int i, k;
  char *key, *point, *err;
  for (k = 0; k < 128; k++)
    pts[k] = plc_pt_null();

  conffile_get_value_i32("quiet", &quiet, 0, 1, quiet);

  for (i = conffile_get_table_rows("key")-1; i >= 0; i--) {
    key = conffile_get_table("key", i, 0);
    point = conffile_get_table("key", i, 1);

    if (conffile_get_table("key", i, 2))
      plc_log_wrnmsg(1, "extra stuff after `key %s %s'", key, point);

    k = strtol(key, &err, 0);
    if ((*key == 0) || (*err != 0)) {
      /* TODO: accept string keycodes as well */
      plc_log_errmsg(1, "unknown key: `%s'", key);
      continue;
    }

    if (plc_pt_len(pts[k]) > 0) {
      plc_log_errmsg(1, "Cannot map key %d twice, mapping to %s.\n", k,
		     point);
    }

    pts[k] = plc_pt_by_name(point);

    if (!pts[k].valid) {
      plc_log_errmsg(1, "Could not get valid handle to %s.\n", point);
      pts[k] = plc_pt_null();
      continue;
    }
    printf(_("Key %d mapped to %s\n"), k, point);
  }
}

int main(int argc, char *argv[])
{
  struct termios new;
  unsigned char buf[1];	/* we don't really want buffering */
  plc_pt_t pts[128];
  int i, n;

  if (plc_init("keyboard", argc, argv) < 0) {
    printf("Error initializing PLC\n");
    return -1;
  }

  read_cfg(pts);

  if (-1 == (fd = get_console_fd(NULL)))
    exit(1);

  /*
     if we receive a signal, we want to exit nicely, in
     order not to leave the keyboard in an unusable mode
   */
  signal(SIGHUP, die);
  signal(SIGINT, die);
  signal(SIGQUIT, die);
  signal(SIGILL, die);
  signal(SIGTRAP, die);
  signal(SIGABRT, die);
  signal(SIGIOT, die);
  signal(SIGFPE, die);
  signal(SIGKILL, die);
  signal(SIGUSR1, die);
  signal(SIGSEGV, die);
  signal(SIGUSR2, die);
  signal(SIGPIPE, die);
  signal(SIGTERM, die);
#ifdef SIGSTKFLT
  signal(SIGSTKFLT, die);
#endif
  signal(SIGCHLD, die);
  signal(SIGCONT, die);
  signal(SIGSTOP, die);
  signal(SIGTSTP, die);
  signal(SIGTTIN, die);
  signal(SIGTTOU, die);

  get_mode();
  if (tcgetattr(fd, &old) == -1)
    perror("tcgetattr = %d\n");
  if (tcgetattr(fd, &new) == -1)
    perror("tcgetattr = %d\n");

  new.c_lflag &= ~(ICANON | ECHO | ISIG);
  new.c_iflag = 0;
  new.c_cc[VMIN] = sizeof(buf);
  new.c_cc[VTIME] = 1;		/* 0.1 sec intercharacter timeout */

  if (tcsetattr(fd, TCSAFLUSH, &new) == -1)
    perror("tcsetattr = %d\n");
  if (ioctl(fd, KDSKBMODE, K_MEDIUMRAW)) {
    perror("KDSKBMODE");
    exit(1);
  }

  printf(_("press Esc to exit...\n"));

  while (1) {
    plc_scan_beg();
    /* plc_update(); (not required) */

    n = read(fd, buf, sizeof(buf));

    for (i = 0; i < n; i++) {
      printf(_("keycode %3d %s\n"),
	     buf[i] & 0x7f, buf[i] & 0x80 ? _("release") : _("press"));
      plc_set(pts[buf[i] & 0x7f], buf[i] & 0x80 ? 0 : 1);
      if (buf[i]==1) {
        printf(_("Exiting (Esc pressed)...\n"));
	clean_quit(0);
      }
    }
    plc_update();
    plc_scan_end();
  }
}
