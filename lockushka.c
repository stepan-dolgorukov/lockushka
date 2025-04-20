#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/file.h>

static volatile sig_atomic_t to_work = true;
static char name_file_lock[256] = ".lck";

static void handle_interrupt(int number_signal)
{
  if (number_signal != SIGINT)
  {
    return;
  }

  if (!to_work)
  {
    return;
  }

  to_work = false;
}

int lock()
{
  int descriptor_lock = -1;

  while (descriptor_lock == -1)
  {
    descriptor_lock = open(name_file_lock, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  }

  pid_t identifier = getpid();
  char identifier_text[16] = {'\0'};

  sprintf(identifier_text, "%i", identifier);

  ssize_t amount = write(descriptor_lock, identifier_text, strlen(identifier_text));

  if (amount <= 0)
  {
    fprintf(stderr, "%i: fail to write to lock\n", identifier);
    close(descriptor_lock);

    return 1;
  }

  close(descriptor_lock);

  return 0;
}

int unlock()
{
  pid_t identifier = getpid();

  if (access(name_file_lock, F_OK) != 0)
  {
    fprintf(stderr, "%i: no lock file\n", identifier);

    return 1;
  }

  int descriptor_lock = open(name_file_lock, O_RDONLY, S_IRUSR | S_IWUSR);

  if (descriptor_lock < 0)
  {
    fprintf(stderr, "%i: fail to open lock file\n", identifier);

    return 1;
  }

  char identifier_text[16] = {'\0'};
  ssize_t amount = read(descriptor_lock, identifier_text, sizeof identifier_text);

  if (amount <= 0)
  {
    fprintf(stderr, "%i: fail to read lock\n", identifier);
    close(descriptor_lock);

    return 1;
  }

  pid_t identifier_read = -1;

  sscanf(identifier_text, "%i", &identifier_read);

  if (identifier_read != identifier)
  {
    fprintf(stderr, "%i: identifier_read != identifier, identifier_read=%i\n", identifier, identifier_read);
    close(descriptor_lock);

    return 1;
  }

  close(descriptor_lock);

  if (remove(name_file_lock) < 0)
  {
    fprintf(stderr, "%i: fail to remove lock file\n", identifier);

    return 1;
  }
}

int write_report(int counter)
{
  char line_report[32] = {'\0'};

  sprintf(line_report, "%i, %i\n", getpid(), counter);

  int descriptor = open("report.text", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

  if (descriptor < 0)
  {
    return 1;
  }

  if (flock(descriptor, LOCK_EX) < 0)
  {
    close(descriptor);

    return 1;
  }

  if (write(descriptor, line_report, strlen(line_report)) <= 0)
  {
    flock(descriptor, LOCK_UN);
    close(descriptor);

    return 1;
  }

  if (flock(descriptor, LOCK_UN) == -1)
  {
    close(descriptor);

    return 1;
  }

  close(descriptor);

  return 0;
}

int main(int amount_arguments, char* arguments[])
{
  if (signal(SIGINT, handle_interrupt) == SIG_ERR)
  {
    return 1;
  }

  int counter_success_locks = 0;

  while (to_work)
  {
    if (lock() != 0)
    {
      fprintf(stderr, "%i: fail to lock\n", getpid());

      return 1;
    }

    sleep(2);

    if (unlock() != 0)
    {
      fprintf(stderr, "%i: fail to unlock\n", getpid());

      return 1;
    }

    ++counter_success_locks;
  }

  if (write_report(counter_success_locks) != 0)
  {
    fprintf(stderr, "%i: fail to write report\n", getpid());
  }

  return 0;
}
