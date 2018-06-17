#include "cpu/exec.h"

void cross_check_skip_qemu();
void cross_check_skip_nemu();

make_EHelper(lidt) {
  cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
  cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4);

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef CROSS_CHECK
  cross_check_skip_qemu();
#endif
}

make_EHelper(int) {
  void raise_intr(uint8_t, vaddr_t);
  raise_intr(id_dest->val, *eip);

  print_asm("int %s", id_dest->str);

#ifdef CROSS_CHECK
  cross_check_skip_nemu();
#endif
}

make_EHelper(iret) {
  /*rtl_pop(&t0);
  decoding.jmp_eip = t0;
  decoding.is_jmp = 1;

  rtl_pop(&t0);
  cpu.cs = t0;

  rtl_pop(&t0);
  cpu.eflags.val = t0*/
  printf("Nothing here\n");

  print_asm("iret");
  }

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  rtl_li(&t0, pio_read(id_src->val, id_dest->len));
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#ifdef CROSS_CHECK
  cross_check_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->len, id_src->val);
  print_asm_template2(out);

  print_asm_template2(out);

#ifdef CROSS_CHECK
  cross_check_skip_qemu();
#endif
}
