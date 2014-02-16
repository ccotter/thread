
#include <stdlib.h>
#include <stdio.h>
#include <threadlib.h>

#include <llist.h>

int worker(void *data)
{
    int id = *(int*)data;
    int i = 0;
    printf("In Worker %d once\n", id);
    while (i++ < 500000);
    thread_yield();
    printf("In Worker %d twice\n", id);
    return 0;
}

int main(int argc, char **argv)
{
    /*
    llist l;
    llist_init(&l);
    llist_add_first(&l, 10);
    llist_add_first(&l, 11);
    llist_add_first(&l, 12);
    printf("size: %d\n", llist_size(&l));
    int i3 = llist_remove_first(&l);
    int i2 = llist_remove_first(&l);
    int i1 = llist_remove_first(&l);
    printf("we have %d %d %d\n", i1, i2, i3);
    return 0;*/
    thread *t1, *t2, *t3;
    int id1 = 0, id2 = 1, id3 = 2;
    // Initialize the threads.
    int rc = thread_init(&t1, worker, &id1);
    if (rc != 0)
    {
        fprintf(stderr, "Unable to create thread t1, rc=%d\n", rc);
        exit(1);
    }
    rc = thread_init(&t2, worker, &id2);
    if (rc != 0)
    {
        fprintf(stderr, "Unable to create thread t1, rc=%d\n", rc);
        exit(1);
    }
    rc = thread_init(&t3, worker, &id3);
    if (rc != 0)
    {
        fprintf(stderr, "Unable to create thread t1, rc=%d\n", rc);
        exit(1);
    }
    // Start execution of the threads.
    thread_run(t1);
    thread_run(t2);
    thread_run(t3);
    thread_join(t1);
    thread_join(t2);
    thread_join(t3);
    printf("DONE\n");
    return 0;
}

