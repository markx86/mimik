#include <structs/list.h>

void
list_init(struct list* list) {
  list->next = list;
  list->prev = list;
}

void
list_insert(struct list* list, struct list* elem) {
  elem->prev = list;
  elem->next = list->next;
  list->next = elem;
  elem->next->prev = elem;
}

void
list_remove(struct list* elem) {
  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;
  elem->prev = NULL;
  elem->next = NULL;
}

size_t
list_length(struct list* list) {
  size_t len = 0;
  for (struct list* elem = list->next; elem != list; elem = elem->next)
    ++len;
  return len;
}
