#ifndef OPCODE_H
#define OPCODE_H


enum opcode_arg_type {
  ARG_NONE = 0,
  ARG_BYTE,
  ARG_MEM,  /* 2 bytes in Z80 mode, 3 in ADL mode */
  ARG_WORD,
  ARG_D_IMM,/* one byte displacement, one byte data */
  ARG_FIX   /* if there is another lookup table */
};

typedef struct opcode {
  const char    *disp;
  unsigned int arg_type;
} opcode_t;

extern opcode_t op_table[256];
extern opcode_t cb_table[256];
extern opcode_t ed_table[256];
extern opcode_t dd_table[256];
extern opcode_t fd_table[256];
extern opcode_t fd_cb_table[256];
extern opcode_t dd_cb_table[256];

#endif /* end of include guard: OPCODE_H */
