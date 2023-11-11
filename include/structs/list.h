#ifndef MIMIK_LIST_H
#define MIMIK_LIST_H

#include <util/struct.h>

struct list {
  struct list* prev;
  struct list* next;
};

void list_init(struct list* list);
void list_insert(struct list* list, struct list* elem);
void list_remove(struct list* elem);
size_t list_length(struct list* list);

#define list_for_each(pos, head, link)                      \
  for (pos = containerof((head)->next, typeof(*pos), link); \
       &pos->link != (head);                                \
       pos = containerof(pos->link.next, typeof(*pos), link))
#define list_for_each_reverse(pos, head, link)              \
  for (pos = containerof((head)->prev, typeof(*pos), link); \
       &pos->link != (head);                                \
       pos = containerof(pos->link.prev, typeof(*pos), link))

#endif
