#include "cpu/exec.h"

static inline void and_internal() {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t2, id_dest->len);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
}

make_EHelper(test) {
  and_internal();
  print_asm_template2(test);
}

make_EHelper(and) {
  and_internal();
  operand_write(id_dest, &t2);
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->len);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->len);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sext(&t2, &id_dest->val, id_dest->len);
  rtl_sar(&t2, &t2, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->len);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->len);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->len);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shr);
}

make_EHelper(rol) {
  rtl_shl(&t1, &id_dest->val, &id_src->val);
  rtl_li(&t2, id_dest->len * 8);
  rtl_sub(&t2, &t2, &id_src->val);
  rtl_shr(&t2, &id_dest->val, &t2);
  rtl_or(&t2, &t1, &t2);

  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->len);
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(rol);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}
