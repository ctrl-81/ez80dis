#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcode.h"
#include "sym.h"

static const char hex_table[16] = "0123456789ABCDEF";

static void print_lst_line(FILE *file, const char *p, int len,
  unsigned long org, int adl){
  if(adl)
    fprintf(file, "%06lX ", org);
  else
    fprintf(file, "%04lX ", org);
  if(len == 1)
    fprintf(file, "%02X -- -- -- -- ", (unsigned char)p[0]);
  else if(len == 2)
    fprintf(file, "%02X %02X -- -- -- ", (unsigned char)p[0], (unsigned char)p[1]);
  else if(len == 3)
    fprintf(file, "%02X %02X %02X -- -- ", (unsigned char)p[0],
      (unsigned char)p[1], (unsigned char)p[2]);
  else if(len == 4)
    fprintf(file, "%02X %02X %02X %02X -- ", (unsigned char)p[0],
      (unsigned char)p[1], (unsigned char)p[2], (unsigned char)p[3]);
  else if(len == 5)
  fprintf(file, "%02X %02X %02X %02X %02X ", (unsigned char)p[0],
    (unsigned char)p[1], (unsigned char)p[2], (unsigned char)p[3],
    (unsigned char)p[4]);
}

static void print_lst_few_bytes(FILE *file, const char *p,
  int len, unsigned long org, int adl){
  print_lst_line(file, p, len, org, adl);
  if(len == 1){
    fprintf(file, ".db 0%02Xh\n", (unsigned char)p[0]);
  }else if(len == 2){
    fprintf(file, ".db 0%02Xh, 0%02Xh\n", (unsigned char)p[0], (unsigned char)p[1]);
  }else if(len == 3){
    fprintf(file, ".db 0%02Xh, 0%02Xh, 0%02Xh\n", (unsigned char)p[0],
      (unsigned char)p[1], (unsigned char)p[2]);
  }else if(len == 4){
    fprintf(file, ".db 0%02Xh, 0%02Xh, 0%02Xh, 0%02Xh\n", (unsigned char)p[0],
      (unsigned char)p[1], (unsigned char)p[2], (unsigned char)p[3]);
  }
}

static int print_lst_word(FILE *file, const char **_p, const char *end,
  int adl, unsigned long org, opcode_t *op, int base_len, int bonus_inc){
  const char *p = *_p;
  const char *label = NULL;
  if(adl){
    char buf[12];
    int to_go = end - p;
    unsigned long num;
    if(to_go < 4) /* check this isn't off by one */
      return to_go;
    print_lst_line(file, p, base_len + 3 + bonus_inc, org, adl);
    p += base_len;
    num = (unsigned char)p[0] + (((unsigned char)p[1]) << 8)
      + (((unsigned char)p[2]) << 16);
    label = lookup_symbol(num, NULL);
    if(label == NULL){
      sprintf(buf, "0%06lXh", num);
      fprintf(file, op->disp, buf);
    }else{
      fprintf(file, op->disp, label);
    }
    (*_p) += 3;
  }else{
    char buf[12];
    int to_go = end - p;
    unsigned long num;
    if(to_go < 3) /* check this isn't off by one */
      return to_go;
    print_lst_line(file, p, base_len + 2, org, adl);
    p += base_len;
    num = (unsigned char)p[0] + (((unsigned char)p[1]) << 8);
    label = lookup_symbol(num, NULL);
    if(label == NULL){
      sprintf(buf, "0%04lXh", num);
      fprintf(file, op->disp, buf);
    }else{
      fprintf(file, op->disp, label);
    }
    (*_p) += 2;
  }
  return 0;
}

static int print_lst_byte(FILE *file, const char **_p, const char *end,
  int adl, unsigned long org, opcode_t *op, int base_len, int bonus_inc){
  const char *p = *_p;
  if(p + 1 == end)
    return 1;
  char tmp_s[5] = {'0', '0', '0', 'h', 0};
  print_lst_line(file, p, base_len + 1 + bonus_inc, org, adl);
  p += base_len;
  tmp_s[2] = hex_table[(unsigned char)p[0] & 0x0F];
  tmp_s[1] = hex_table[((unsigned char)(p[0] & 0xF0) >> 4)];
  fprintf(file, op->disp, tmp_s);
  ++(*_p);
  return 0;
}

int z80_disassemble(
  const char *filename,
  const char *assem,
  unsigned long asm_len,
  FILE *file,
  int adl,
  unsigned long oorg,
  int warn)
{
  const char *p, *end_addr = assem + asm_len;
  fprintf(file, "; Assembly listing of ``%s''\n"
    "; Produced by ez80dis\n"
    ".ASSUME ADL=%c\n", filename, adl ? '1' : '0');
  int base_len = 1, bonus_inc = 0;
  for(p = assem; p < end_addr; p += base_len + bonus_inc){
    const char *label;
    opcode_t *op;
    unsigned long org = oorg + (p - assem);
    base_len = 1;
    bonus_inc = 0;
    while((label = check_dis_address(org, warn)) != NULL)
      fprintf(file, "%s:\n", label);
    if((unsigned char)p[0] == 0xCB){
      op = cb_table + (unsigned char)p[1];
      ++base_len;
    }else if((unsigned char)p[0] == 0xED){
      op = ed_table + (unsigned char)p[1];
      ++base_len;
    }else if((unsigned char)p[0] == 0xDD){
      ++base_len;
      if(p + 1 == end_addr){
        op = NULL;
        --base_len;
      }else if((unsigned char)p[1] == 0xCB){
        if(p + 3 >= end_addr)
          op = NULL;
        else {
          op = dd_cb_table + (unsigned char)p[3];
          bonus_inc = 1;
        }
      }else{
        op = dd_table + (unsigned char)p[1];
      }
    }else if((unsigned char)p[0] == 0xFD){
      ++base_len;
      if(p + 1 == end_addr)
        op = NULL;
      else if((unsigned char)p[1] == 0xCB){
        if(p + 3 >= end_addr)
          op = NULL;
        else {
          op = fd_cb_table + (unsigned char)p[3];
          bonus_inc = 1;
        }
      }else
        op = fd_table + (unsigned char)p[1];
    }else
      op = op_table + (unsigned char)p[0];
    /* etc. */
    if(op != NULL && op->disp != NULL){
      if(op->arg_type == ARG_NONE){
        print_lst_line(file, p, base_len, org, adl);
        fputs(op->disp, file);
      }else if(op->arg_type == ARG_BYTE){
        int to_go = print_lst_byte(file, &p, end_addr, adl, org, op,
          base_len, bonus_inc);
        if(to_go)
          print_lst_few_bytes(file, p, to_go, org, adl);
      }else if(op->arg_type == ARG_WORD){
        int to_go = print_lst_word(file, &p, end_addr, adl, org, op,
          base_len, bonus_inc);
        if(to_go)
          print_lst_few_bytes(file, p, to_go, org, adl);
      }else if(op->arg_type == ARG_D_IMM){
        if(p + 3 >= end_addr){
          print_lst_few_bytes(file, p, end_addr - p, org, adl);
        }else{
          char buf1[8], buf2[8];
          print_lst_line(file, p, base_len + 2, org, adl);
          fflush(stdout);
          sprintf(buf1, "0%02Xh", (unsigned char)p[2]);
          sprintf(buf2, "0%02Xh", (unsigned char)p[3]);
          fprintf(file, op->disp, buf1, buf2);
        }
        p += 2;
      }
    }else if(op == NULL){
        print_lst_few_bytes(file, p, end_addr - p, org, adl);
    }else{
      print_lst_few_bytes(file, p, 1, org, adl);
    }
    fputc('\n', file);
  }
  return 0;
}
