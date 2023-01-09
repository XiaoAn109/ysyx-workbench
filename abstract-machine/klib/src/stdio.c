#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void num2str(int n, char *str, int radix, int lowercase, int is_unsigned)
{
	int i, j, remain, minus = 0;
	uint64_t m = n;
	char tmp;
	 
	i = 0;
	if(!is_unsigned && n < 0) {
		minus = 1;
		m = -m;
	}
	if(is_unsigned) {
		uint32_t un = m;
		do {
			remain = un%radix;
			if(remain > 9) {
				if(lowercase)
					str[i] = remain - 10 + 'a';
				else
					str[i] = remain - 10 + 'A';
			}
			else 
				str[i] = remain + '0';
			i++;
		} while(un /= radix);
	}
	else {
		do {
			remain = m%radix;
			if(remain > 9) {
				if(lowercase)
					str[i] = remain - 10 + 'a';
				else
					str[i] = remain - 10 + 'A';
			}
			else 
				str[i] = remain + '0';
			i++;
		} while(m /= radix);
	}
	if(minus) {
		str[i] = '-';
		i++;
	}
	str[i] = '\0';

	for(i--, j=0; j<=i; j++, i--) {
		tmp = str[j];
		str[j] = str[i];
		str[i] = tmp;
	}
}

void double2str(double f, char *str) {
	int n = f;
	char tail[10];
	num2str(n, str, 10, 0, 0);
	strcat(str, ".");
	double bias = n>0 ? 0.0000005 : -0.0000005;
	n = (f-n+bias)*1000000;
	if(n<0) n = -n;
	num2str(n, tail, 10, 0, 0);
	for(int i=6; i>strlen(tail); i--)
		strcat(str, "0");
	strcat(str, tail);
}
 
int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    int len = 0;
    while (*fmt != '\0') {
        /* no % */
        if (*fmt != '%') {
            *out = *fmt;
            out++;
            fmt++;
            len++;
            continue;
        }
        /* if % fmt++ & get type */
        fmt++;
        switch (*fmt) {
			case 'c': {
				char c = (char)va_arg(ap, int);
				if(c != '\0') {
					*out = c;
					out++;
					len++;
				}
				break;
			}
            case 's': {
                char* sp = va_arg(ap, char*);
                while (*sp != '\0') {
                    *out = *sp;
                    out++;
                    sp++;
                    len++;
                }
                break;
            }
			case 'd': case 'i': {
                int num = va_arg(ap, int);
				int i=0;
				char str[12];
				num2str(num, str, 10, 0, 0);
                while (str[i]) {
                    *out = str[i++];
                    out++;
                    len++;
                }
                break;
            }
			case 'u': {
                uint32_t num = (uint32_t)va_arg(ap, int);
				int i=0;
				char str[12];
				num2str(num, str, 10, 0, 1);
                while (str[i]) {
                    *out = str[i++];
                    out++;
                    len++;
                }
                break;
            }
			case 'x': case 'X': {
                uint32_t num = (uint32_t)va_arg(ap, int);
				int i=0;
				char str[12];
				int lowercase = (*fmt == 'x');
				num2str(num, str, 16, lowercase, 1);
                while (str[i]) {
                    *out = str[i++];
                    out++;
                    len++;
                }
                break;
			}
			case 'o': {
                uint32_t num = (uint32_t)va_arg(ap, int);
				int i=0;
				char str[12];
				num2str(num, str, 8, 0, 1);
                while (str[i]) {
                    *out = str[i++];
                    out++;
                    len++;
                }
                break;
            }
			case 'f': {
                double f = va_arg(ap, double);
				int i=0;
				char str[12];
				double2str(f, str);
                while (str[i]) {
                    *out = str[i++];
                    out++;
                    len++;
                }
                break;
            }
			default: panic("Not implemented");
        }
        fmt++;
    }
    *out = '\0';
    //len++;
    return len;
}

int sprintf(char *out, const char *fmt, ...) {
    uint32_t ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsprintf(out, fmt, ap);
    va_end(ap);
    return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
