#include <mem/str.h>
#include <assert.h>

size_t
str_length(const char* str) {
  size_t len;
  ASSERT(str != NULL);
  for (len = 0; *(str++) != '\0'; ++len)
    ;
  return len;
}

bool_t
str_equal(const char* this, const char* other) {
  size_t n1, n2;
  n1 = str_length(this);
  n2 = str_length(other);
  if (n1 != n2)
    return FALSE;
  return str_nequal(this, other, n1);
}

bool_t
str_nequal(const char* this, const char* other, size_t n) {
  ASSERT(n > 0);
  ASSERT(this != NULL);
  ASSERT(other != NULL);
  for (; n > 0; n--) {
    if (*(this ++) != *(other++))
      return FALSE;
  }
  return TRUE;
}
