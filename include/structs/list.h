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
  for (pos = CONTAINEROF((head)->next, TYPEOF(*pos), link); \
       &pos->link != (head);                                \
       pos = CONTAINEROF(pos->link.next, TYPEOF(*pos), link))
#define list_for_each_reverse(pos, head, link)              \
  for (pos = CONTAINEROF((head)->prev, TYPEOF(*pos), link); \
       &pos->link != (head);                                \
       pos = CONTAINEROF(pos->link.prev, TYPEOF(*pos), link))
#define list_is_empty(list) ((list) == (list)->next)

static inline void
list_join(struct list* list, struct list* other) {
  if (list_is_empty(other))
    return;
  other->next->prev = list;
  other->prev->next = list->next;
  list->next->prev = other->prev;
  list->next = other->next;
}

#endif
