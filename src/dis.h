#ifndef DIS_H
#define DIS_H

int z80_disassemble(
  const char *filename,
  const char *assem,
  unsigned long asm_len,
  FILE *file,
  int adl,
  unsigned long oorg,
  int warn);


#endif /* end of include guard: DIS_H */
