#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "dis.h"
#include "sym.h"

const char *help =
"USAGE: %s [options] <input>\n\n"
"OPTIONS:\n"
"-help      Print this message\n"
"-A         Assume ADL mode\n"
"-org <a>   Set the base address of the file\n"
"-sym <f>   Add specified symbol file\n"
"-start <s> Specify the position at which to start disassembling\n"
"-end <a>   Specify the point at which to stop disassembling\n"
"-wno       Supress warnings\n"
"-o <f>     Specify the output file\n\n";

int main(int argc, char **argv){
  const char *in_file = NULL;
  const char *out_file = NULL;
  unsigned long addr = 0;
  int addr_given = 0;
  int adl = 0, close_output = 0;
  FILE *file;
  char *buf;
  unsigned long bufsize;
  unsigned long fend = 0;
  unsigned long fstart = 0;
  int warn = 1;
  for(char **s = argv + 1; *s != NULL; ++s){
    if(**s != '-'){
      if(in_file != NULL){
        fputs("z80dis: error: multiple inputs specified\n", stderr);
        return 1;
      }
      in_file = *s;
    }else if(!strcmp(*s, "-A")){
      if(adl){
        fputs("z80dis: error: multiple options specified for ADL mode\n",
          stderr);
        return 1;
      }
      adl = 1;
    }else if(!strcmp(*s, "-o")){
      ++s;
      if(*s == NULL){
        fputs("z80dis: error: no output given\n", stderr);
        return 1;
      }
      out_file = *s;
    }else if(!strcmp(*s, "-sym")){
      ++s;
      if(*s == NULL){
        fputs("z80dis: error: no symbols given\n", stderr);
        return 1;
      }
      load_symbols(*s);
    }else if(!strcmp(*s, "-help")){
      fprintf(stderr, help, argv[0]);
      return 0;
    }else if(!strcmp(*s, "-org")){
      ++s;
      if(*s == NULL){
        fputs("z80dis: error: no base address given\n", stderr);
        return 1;
      }
      if(!isdigit(**s)){
        fputs("z80dis: error: illegal address given\n", stderr);
        return 1;
      }
      if(addr_given){
        fputs("z80dis: error: address already given\n", stderr);
        return 1;
      }
      addr_given = 1;
      addr = strtoul(*s, NULL, 0);
    }else if(!strcmp(*s, "-end")){
      ++s;
      if(*s == NULL){
        fputs("z80dis: error: no end location given\n", stderr);
        return 1;
      }
      if(fend){
        fputs("z80dis: error: multiple end locations specified\n", stderr);
        return 1;
      }
      fend = strtoul(*s, NULL, 0);
      if(fend == 0){
        fprintf(stderr, "z80dis: error: illegal end location ``%s'' given\n",
          *s);
        return 1;
      }
    }else if(!strcmp(*s, "-start")) {
      ++s;
      if(*s == NULL){
        fputs("z80dis: error: no start location given\n", stderr);
        return 1;
      }
      if(fstart){
        fputs("z80dis: error: multiple start location specified\n", stderr);
        return 1;
      }
      fstart = strtoul(*s, NULL, 0);
      if(fstart == 0){
        fprintf(stderr, "z80dis: error: illegal end location ``%s'' given\n",
          *s);
        return 1;  
      }
    }else if(!strcmp(*s, "-wno")){
      warn = 0;
    }else if(!strcmp(*s, "--help")){
      fprintf(stderr, help, argv[0]);
      return 1;
    }else{
      fprintf(stderr, "z80dis: error: unknown argument ``%s''\n", *s);
      return 1;
    }
  }
  if(in_file == NULL){
    fputs("z80dis: error: no input\n", stderr);
    return 1;
  }

  file = fopen(in_file, "rb");
  if(file == NULL){
    fprintf(stderr, "z80dis: error: could not open input ``%s'': %s\n",
      in_file, strerror(errno));
    return 1;
  }
  fseek(file, 0, SEEK_END);
  bufsize = ftell(file);
  if(fend != 0){
    if(bufsize < fend && warn)
      fputs("z80dis: warning: file is smaller than the end location\n", stderr);
    else if(bufsize > fend)
      bufsize = fend;
  }
  
  if(fstart > bufsize) {
      fputs("z80dis: error: start location exceeds the end location\n", stderr);
      return 1;
  }
  fseek(file, fstart, SEEK_SET);
  buf = malloc(bufsize);
  if(buf == NULL){
    fprintf(stderr, "z80dis: internal error (%d): %s\n", errno, strerror(errno));
    return 1;
  }
  bufsize -= fstart;
  addr += fstart;
  fread(buf, bufsize, 1, file);
  fclose(file);

  if(out_file == NULL)
    file = stdout;
  else{
    file = fopen(out_file, "wb");
    if(file == NULL){
      fprintf(stderr, "z80dis: error: could not open output ``%s'': %s\n",
        out_file, strerror(errno));
      return 1;
    }
    close_output = 1;
  }
  format_symbols();
  z80_disassemble(in_file, buf, bufsize, file, adl, addr, warn);
  if(close_output)
    fclose(file);
  free_symbols();
  return 0;
}
