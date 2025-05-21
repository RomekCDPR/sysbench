#ifndef SB_GAMESERVER_H
#define SB_GAMESERVER_H

/* Threads request definition */

typedef struct
{
  unsigned int lock_num;
} sb_gameserver_request_t;

int register_test_gameserver(sb_list_t *tests);

#endif

