#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
  size_t len = 0;
  while (*s++)
    len++;
  return len;
}

char *strcpy(char *dst, const char *src)
{
  assert(dst != NULL);
  char *tmp = dst;
  while (*src)
    *dst++ = *src++;
  *dst = '\0';
  return tmp;
}

char *strncpy(char *dst, const char *src, size_t n)
{
  assert(dst != NULL);
  char *tmp = dst;
  while (*src && n--)
    *dst++ = *src++;
  *dst = '\0';
  return tmp;
}

char *strcat(char *dst, const char *src)
{
  assert(dst != NULL);
  char *tmp = dst + strlen(dst);
  while (*src)
    *tmp++ = *src++;
  *tmp = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1)
  {
    if (*s1 != *s2)
      break;
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  while (*s1 && --n)
  {
    if (*s1 != *s2)
      break;
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void *memset(void *s, int c, size_t n)
{
  char *tmp = s;
  while (n--)
    *tmp++ = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
  assert(dst != NULL);
  for (int i = n-1; i >= 0; i--)
    *((char *)dst + i) = *((char *)src + i);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n)
{
  assert(out != NULL);
  for (int i = 0; i < n; i++)
    *((char *)out + i) = *((char *)in + i);
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  char *tmp1 = (char *)s1;
  char *tmp2 = (char *)s2;
  while (*tmp1 && --n)
  {
    if (*tmp1 != *tmp2)
      break;
    tmp1++;
    tmp2++;
  }
  return *tmp1 - *tmp2;
}

#endif
