#ifndef SYM_H
#define SYM_H

typedef struct sym {
  unsigned long addr;
  char   *name;
} sym_t;


int load_symbols(const char *fname);
void format_symbols(void);
void free_symbols(void);
void debug_dump_symbols(void);
char *lookup_symbol(unsigned long addr, unsigned long *offset);
char *check_dis_address(unsigned long addr, int warn);

#endif /* end of include guard: SYM_H */
