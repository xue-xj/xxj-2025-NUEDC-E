#include <stddef.h>
void *memset(void *p,int value,size_t size){unsigned char *out=p;while(size--)*out++=(unsigned char)value;return p;}
void *memcpy(void *p,const void *source,size_t size){unsigned char *out=p;const unsigned char *in=source;while(size--)*out++=*in++;return p;}
void *memmove(void *p,const void *source,size_t size){unsigned char *out=p;const unsigned char *in=source;if(out<in){while(size--)*out++=*in++;}else{out+=size;in+=size;while(size--)*--out=*--in;}return p;}
void __aeabi_memclr4(void *p,size_t size){memset(p,0,size);}
void __aeabi_memclr(void *p,size_t size){memset(p,0,size);}
void __aeabi_memcpy(void *p,const void *source,size_t size){memcpy(p,source,size);}
void __aeabi_memcpy4(void *p,const void *source,size_t size){memcpy(p,source,size);}
