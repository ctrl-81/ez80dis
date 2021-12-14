#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "sym.h"

#define MAX_LINE 128

sym_t *sym_table = NULL;
unsigned long sym_table_size = 0;
unsigned long sym_index = 0;

static unsigned long asm_strtoul(const char *p){
  int base = 0;
  if(!isdigit(p[0]) && p[0] != -1)
    return 0;
  for(int i = 0; p[i] != '\0'; ++i){
    if(p[i] == 'h')
      base = 16;
    else if(!isdigit(p[i]) && !(p[i] >= 'A' && p[i] <= 'F'))
      break;
  }
  return strtoul(p, NULL, base);
}

int load_symbols(const char *fname){
  FILE *file = fopen(fname, "rb");
  char buf[MAX_LINE];
  if(file == NULL){
    fprintf(stderr, "z80dis: warning: could not open symbol file ``%s'': %s\n",
      fname, strerror(errno));
    return 1;
  }
  while(fgets(buf, sizeof(buf), file) != NULL){
    int sect = 0, in_arg = 0;
    char *name = NULL;
    unsigned long addr = 0;
    for(char *p = (char*)&buf; *p != '\0'; ++p){
      if(sect == 0){
        if(!in_arg && !isspace(*p)){
          if(p[0] == ';' || p[0] == '#')
            break;
          name = p;
          in_arg = 1;
        }else if(in_arg && isspace(*p)){
          p[0] = '\0';
          ++sect;
        }
      }else if(sect == 1 && !isspace(*p)){
        if((p[0] == 'e' || p[0] == 'E') && (p[1] == 'q' || p[1] == 'Q') &&
          (p[2] == 'u' || p[2] == 'U')){
            p += 2;
            ++sect;
        }else{
          name = NULL;
          break;
        }
      }else if(sect == 2 && !isspace(*p)){
        addr = asm_strtoul(p);
        break;
      }
    }
    if(name == NULL || addr == 0)
      continue;
    sym_table = realloc(sym_table, (sym_table_size + 1) * sizeof(sym_t));
    sym_table[sym_table_size].addr = addr;
    sym_table[sym_table_size++].name = strdup(name);
  }
  fclose(file);
  return 0;
}

int sym_cmp(const void *a, const void *b){
  return ((sym_t*)a)->addr - ((sym_t*)b)->addr;
}

void format_symbols(void){
  if(sym_table == NULL)
    return;
  qsort(sym_table, sym_table_size, sizeof(sym_t), sym_cmp);
}

void debug_dump_symbols(void){
  puts("Loaded Symbols:");
  for(unsigned long i = 0; i < sym_table_size; ++i)
    printf("%32s  0x%lX\n", sym_table[i].name, sym_table[i].addr);
}

void free_symbols(void){
  for(unsigned long i = 0; i < sym_table_size; ++i)
    free(sym_table[i].name);
  free(sym_table);
  sym_table_size = 0;
}

char *lookup_symbol(unsigned long addr, unsigned long *offset){
/* use for memory accesses and jumps */
  unsigned long upper = sym_table_size - 1;
  unsigned long lower = 0;
  if(offset != NULL)
    *offset = 0;
  if(sym_table_size == 0)
    return NULL;
  do {
    unsigned long tmp;
    if(sym_table[lower].addr == addr)
      return sym_table[lower].name;
    tmp = (upper + lower) / 2;
    if(sym_table[tmp].addr == addr)
      return sym_table[tmp].name;
    else if(sym_table[tmp].addr < addr)
      lower = tmp;
    else
      upper = tmp;
    if(sym_table[upper].addr == addr)
      return sym_table[upper].name;
  } while(upper != lower && upper != lower + 1);
  if(offset != NULL){
    *offset = addr - sym_table[lower].addr;
    return sym_table[lower].name;
  }else
    return NULL;
}

char *check_dis_address(unsigned long addr, int warn){
/* check whether the current disassembly location corresponds to a symbol,
   if there is a return value, then it should be called again, to check if
   there are more symbols with the same address. Use for labels. UNTESTED. */
  while(sym_index < sym_table_size && sym_table[sym_index].addr <= addr){
    if(sym_table[sym_index].addr < addr){
      if(warn)
        fprintf(stderr, "z80dis: warning: symbol ``%s'' (0x%lX) skipped\n",
          sym_table[sym_index].name, sym_table[sym_index].addr);
      ++sym_index;
    }else if(sym_table[sym_index].addr == addr){
      return sym_table[sym_index++].name;
    }
  }
  return NULL;
}
