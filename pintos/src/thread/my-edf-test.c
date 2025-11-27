#include <stdio.h>
#include "threads/thread.h"
#include "threads/synch.h"

static struct semaphore step_sem;

static void
edf_worker (void *aux)
{
  int id = (int) aux;

  /* round 1 */
  printf ("T%d round 1, dl=%lld\n", id, thread_get_deadline ());

  if (id == 3)
    {
      printf ("T3 sets dl=10\n");
      thread_set_deadline (10);
    }

  /* wait for main to start round 2 */
  sema_down (&step_sem);

  /* round 2 */
  printf ("T%d round 2, dl=%lld\n", id, thread_get_deadline ());

  thread_yield ();
}

void
my_edf_test (void)
{
  printf ("EDF test: change deadline between rounds\n");

  sema_init (&step_sem, 0);

  /* initial deadlines */
  thread_create_with_deadline ("T1", 31, 50, edf_worker, (void *) 1);
  thread_create_with_deadline ("T2", 31, 20, edf_worker, (void *) 2);
  thread_create_with_deadline ("T3", 31, 70, edf_worker, (void *) 3);
  thread_create_with_deadline ("T4", 31, 40, edf_worker, (void *) 4);

  /* let all threads run round 1 and block on sema_down */
  thread_yield ();

  /* wake all four for round 2 (they re-enter ready_list together) */
  sema_up (&step_sem);
  sema_up (&step_sem);
  sema_up (&step_sem);
  sema_up (&step_sem);

  /* run round 2 in EDF order after T3 changed deadline */
  thread_yield ();

  printf ("EDF test done\n");
}
