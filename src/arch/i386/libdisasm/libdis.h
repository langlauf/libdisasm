#include <bastard.h>
#include <extension.h>
#include <vm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LIBDISASM_H
#define LIBDISASM_H


/* formats : */
#define NATIVE_SYNTAX 0x00
#define INTEL_SYNTAX  0x01
#define ATT_SYNTAX    0x02


struct instr {
    char    mnemonic[16];
    char    dest[32];
    char    src[32];
    char    aux[32];
    int     mnemType;
    int     destType;
    int     srcType;
    int     auxType;
    int     size;
};


/* OK, this is a horrible API as there is no rhyme or reason
 * to function naming or parameters. Deal. */
int disassemble_init(int options, int format);
int disassemble_cleanup(void);
int sprint_addrexp(char *str, int len, struct addr_exp *e);
int disassemble_address(char *buf, int buf_len, struct instr *i);
int sprint_address(char *str, int len, char *buf, int buf_len);

/* access to disassembler internal settings */
/* ext_arch is intended for internal use only */
struct EXT__ARCH ext_arch;

/* Endianness of CPU */
#define cpu_endian() ext_arch.endian
/* Default address and operand size in bytes */
#define cpu_addr_size() ext_arch.sz_addr
#define cpu_op_size() ext_arch.sz_oper
/* Size of a machine word in bytes */
#define cpu_word_size() ext_arch.sz_word
/* maximum size of a code instruction */
#define cpu_inst_size() ext_arch.sz_inst
/* register IDs of Stack, Frame, and Instruction pointer */
#define cpu_sp() ext_arch.SP
#define cpu_fp() ext_arch.FP
#define cpu_ip() ext_arch.IP

/* These all take operand type as a parameter */
/* operand type, size, permissions, modifiers */
#define op_type(x) (x & OP_TYPE_MASK)
#define op_size(x) (x & OP_SIZE_MASK)
#define op_perm(x) (x & OP_PERM_MASK)
#define op_mod(x) (x & OP_MOD_MASK)
/* operand segment override */
#define op_seg(x) (x & OP_SEG_MASK)
/* is operand a register */
#define op_isreg(x) (x & OP_REG)

/* These take instruction type as a parameter */
#define insn_type(x)  (x & INSN_TYPE_MASK)
#define insn_group(x) (x & INSN_GROUP_MASK)
#define insn_mod(x) (x & INSN_MOD_MASK)
#define insn_size(x) (x & INSN_SIZE_MASK)

#endif
