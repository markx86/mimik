#include <structs/list.h>
#include <assert.h>

void
list_init(struct list* list) {
  ASSERT(list != NULL);
  list->next = list;
  list->prev = list;
}

void
list_insert(struct list* list, struct list* elem) {
  ASSERT(list != NULL);
  ASSERT(elem != NULL);
  elem->prev = list;
  elem->next = list->next;
  list->next = elem;
  elem->next->prev = elem;
}

void
list_remove(struct list* elem) {
  ASSERT(elem != NULL);
  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;
  elem->prev = NULL;
  elem->next = NULL;
}

size_t
list_length(struct list* list) {
  struct list* elem;
  size_t len = 0;
  ASSERT(list != NULL);
  for (elem = list->next; elem != list; elem = elem->next)
    ++len;
  return len;
}

void
list_join(struct list* list, struct list* other) {
  ASSERT(list != NULL);
  ASSERT(list != other);
  if (list_is_empty(other))
    return;
  other->next->prev = list;
  other->prev->next = list->next;
  list->next->prev = other->prev;
  list->next = other->next;
}
