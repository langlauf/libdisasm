#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i386.h"

/* This can be set by an init routine, so that 16 and 32 bit disassembly
 * will be supported. For now, it is always 32 :) */
int default_addr_mode = 32;



int GetSizedOperand( char *str, BYTE *buf, int size, int flag) {
	int i;
	short s;
	char c;
	
	switch (size) {
		case 1:
			c = *(char *)&buf[0];
			if (flag == OP_SIGNED && c < 0 ) {
					strcat(str, "-");
					c *= -1;
			}
			sprintf(str, "%s%02X", str, c);
			break;
		case 2:
			s = *(short *)&buf[0];
			if (flag == OP_SIGNED && s < 0){
					strcat(str, "-");
					s *= -1;
			}
			sprintf(str, "%s%04X", str, s);
			break;
		case 4:
		case 6:
		case 8:
		default:
			i = *(int *)&buf[0];
			if (flag == OP_SIGNED && i < 0 ){
					strcat(str, "-");
					i *= -1;
			}
			sprintf(str, "%s%08X", str, i);
			break;
	}
}

int DecodeByte(BYTE b, struct modRM_byte *modrm){	
	/* generic bitfield-packing routine */
	
	modrm->mod = b >> 6;
	modrm->reg = ( b & 56 ) >> 3;
	modrm->rm  = b & 7;
	
	return(0);
}



	
int DecodeSIB(BYTE *b, int disp, char *operand) {
	/* Rewrite to handle 16 vs 32-bit registers and such */
	int count = 1;		/* start at 1 for SIB byte */
	char tmp[32];
	struct SIB_byte sib;
	
	DecodeByte(*b, (struct modRM_byte *) &sib);

	/* full scale and index setup [index != 100] */
	if ( sib.index != SIB_INDEX_NONE ){ 
		strncat( operand, reg_dword[sib.index], 32);

		/* don't print the useless "* 1" [ss 01-11] scale = 2^ss */
		if ( sib.scale > 0) {
			strncpy(tmp, operand, 16);
			sprintf( operand, "(%s * %02X)", tmp, (1 << sib.scale) );
		}

		/* add any displacement from modR/M */
		if ( disp ) {
			if ( disp < 0)
				strcat(operand, " -");
			else
				strcat(operand, " +");
			sprintf( operand, "%s %X", operand, disp);
		}


	/* displacement, but no scale or index */
	} else if ( disp ) {
		if ( disp < 0)
				strcat(operand, " -");
	   sprintf( operand, "%s %X ", operand, disp );
	}


	/* Intel docs cite some weirdness in the area of the EBP base reg */
	if ( sib.base == SIB_BASE_EBP && sib.scale == SIB_SCALE_NOBASE ){
		/* what to do here? implied disp32 ? */
		sprintf( operand, "%s %X", operand, *(DWORD *)&b[1] );
		count += sizeof(DWORD);

	/* do normal handling of base register */
	} else {
		sprintf( operand, "%s + %s", operand, reg_dword[sib.base]);
	}

	/* enclose in brackets ;) ] */
	strncpy(tmp, operand, 31);
	sprintf(operand, "[%s]", tmp);
	return(count);
}



/* TODO : Mark index modes
          Use addressing mode flags to imply arrays (index), structure (disp),
			 two-dimensional arrays [disp + index], classes [ea reg], and so on.
			 Don't forget to flag string (*SB, *SW) instructions
*/
/* returns number of bytes it decoded */
int DecodeModRM(BYTE *b, char *operand, int *op_flags, char **reg_table, 
					 int size, int flags){
	int count=1; 	/* # of bytes decoded -- start with 1 for the modR/M byte */
	int disp = 0;
	struct modRM_byte modrm;
	
	DecodeByte(*b, &modrm);

	if ( flags == MODRM_EA) {
	/* This is the 'EA' operand */
			  
		/* if op1 is a register [mod 11] */
		if ( modrm.mod == MODRM_MOD_NOEA ) {
			*op_flags |= OP_REG;
			strncpy( operand, reg_table[modrm.rm], 32); /* mmx, debug, etc */

		/* if op1 is a 32-bit displacement [mod 00 rm 101] */
		} else if ((modrm.mod == MODRM_MOD_NODISP) && 
					  (modrm.rm == MODRM_RM_NOREG)) {
			/* unsure about this one */
			*op_flags |= OP_IND | OP_UNSIGNED;
			GetSizedOperand(operand, b + 1, size, OP_UNSIGNED);
			count += size;
			/* or is an SIB implied? goddamn intel... */

		/* if operand uses an SIB [rm 100] */
		} else if (modrm.rm == MODRM_RM_SIB){

			/* if 8-bit displacement, sign-extend add it */
	   	if ( modrm.mod == MODRM_MOD_DISP8) {
				disp = b[2];                 /* b[1] == SIB, b[2] == disp */
				count += sizeof(BYTE);

			/* if 32-bit displacement, add it */
			} else { //if (modrm.mod == MODRM_MOD_DISP32) {
				disp = (DWORD)b[2];                /* b[1] == SIB, b[2] == disp */
				count += sizeof(DWORD);
			}
			*op_flags |= OP_EXPR;
			count += DecodeSIB(&b[1], disp, operand);	

		/* operand is a register + displacement combo */
		} else {
			/* get register based on operand size and opcode flags */
			sprintf( operand, "[%s", reg_table[modrm.rm]);
			
			/* if 8-bit displacement, add it [mod == 01 ]*/
	   	if ( modrm.mod == MODRM_MOD_DISP8) {
				sprintf( operand, "%s + %02X", operand, b[1]); /* no SIB */
				count += sizeof(BYTE);
				
				/* if 32-bit displacement, add it [mod == 10 ]*/
			} else if (modrm.mod == MODRM_MOD_DISP32) {
				sprintf( operand, "%s + %04X", operand, (DWORD) b[1]); /* no SIB */
				count += sizeof(DWORD);
		 	} 

			strcat( operand, "]");
			*op_flags |= OP_EXPR;
		}
	
	

	} else {
	/* This is the 'reg' operand */
			  
		/* fill operand with register */
		strncpy( operand, reg_table[modrm.reg], 32); /* mmx, debug, etc */
		*op_flags |= OP_REG;
	}
	
	return(count);
}


int InstDecode( instr *t, BYTE *buf, struct code *c, DWORD rva){
	int x, flags, bytes=0, size=0;
	int addr_size, op_size, op_notes; /* for override prefixes */
	unsigned int addr_meth, op_type;
	char **genRegs;
	/* tables used to address each operands with the for loop */
	char *operands[3] = {    t->dest,       t->src,       t->aux       };
	int   op_flags[3] = {    t->destFlg,    t->srcFlg,    t->auxFlg    };
	/* destination buffers in the CODE struct */
	char *dest_buf[3] = {    c->dest,      c->src,      c->aux       };
	int  *dest_flg[3] = {    &c->destType, &c->srcType, &c->auxType   };
	

	/*  ++++   1. Copy mnemonic and mnemonic-flags to CODE struct */
	if ( t->mnemonic)   
			  strncpy( c->mnemonic, t->mnemonic, 15);
	c->mnemType |= t->mnemFlg;

	/*  ++++   2. Handle opcode prefixes */
	addr_size = default_addr_mode;
	if ( t->mnemFlg & PREFIX_ADDR_SIZE) {
		if ( addr_size == 32 ) addr_size = 16; /* that's right, it's a toggle */
		else addr_size = 32;
	}
	/* TODO : do something w/ addr size :) */
	op_size = default_addr_mode;
	if ( t->mnemFlg & PREFIX_OP_SIZE) {
		if ( op_size == 32 ) op_size = 16; /* this one too */
		else op_size = 32;
	}

	/*  ++++   3. Fill operands and operand-flags in CODE struct */
	for (x=0; x < 3; x++ ) {
		/* set default register set */
		if ( op_size == 32) genRegs = reg_dword;
		else                genRegs = reg_word;
	
	   /* ++ Yank optype and addr mode out of operand flags */
		addr_meth = op_flags[x] & ADDRMETH_MASK;
		op_type   = op_flags[x] & OPTYPE_MASK;
		op_notes  = op_flags[x] & OPFLAGS_MASK; /* these are passed to bastard */
		/* ++ Copy flags to CODE struct */
		*dest_flg[x] |= op_notes;

		/* ++ Handle operands hard-coded in the opcode [e.g. "dec eax"] */
		if ( operands[x] ) {
			strncpy(dest_buf[x], operands[x], 31);
			continue; /* next instructuin */
		}

		/* ++ Do Operand Type ++ */
		switch ( op_type) {
			/* ------------------------ Operand Type ----------------- */
			case OPTYPE_c  :	/* byte or word [op size attr] */
				size = ( op_size == 32 ) ? 2 : 1;
				break;
			case OPTYPE_a  :	/* 2 word or 2 DWORD [op size attr ] */
				/* when is this used? */
				size = ( op_size == 32 ) ? 4 : 2;
				break;
			case OPTYPE_v  :	/* word or dword [op size attr] */
				size = ( op_size == 32 ) ? 4 : 2;
				break;
			case OPTYPE_p  :	/* 32/48-bit ptr [op size attr] */
				size = ( op_size == 32 ) ? 6 : 4;
				break;
			case OPTYPE_b  :	/* byte, ignore op-size */
				size = 1;
				break;
			case OPTYPE_w  :	/* word, ignore op-size */
				size = 2;
				break;
			case OPTYPE_d  :	/* dword , ignore op-size*/
				size = 4;
				break;
			case OPTYPE_s  :	/* 6-byte psuedo-descriptor */
				size = 6;
				break;
			case OPTYPE_q  :	/* qword, ignore op-size */
				size = 8;
				break;
			case OPTYPE_dq  :	/* d-qword, ignore op-size */
			case OPTYPE_ps  :	/* 128-bit FP data */
			case OPTYPE_ss  :	/* Scalar elem of 128-bit FP data */
				size = 16;
				break;
			case OPTYPE_pi  :	/* qword mmx register */
				break;
			case OPTYPE_si  :	/* dword integer register */
				break;
			case 0:
			default:
				/* ignore -- operand not used in this intruction */
				break;
		}
		/* override default register set */
		if      ( size == 1 ) genRegs = reg_byte;
		else if ( size == 2 ) genRegs = reg_word;
		else if ( size == 4 ) genRegs = reg_dword;

		/* ++ Do Operand Addressing Method / Decode operand ++ */
		switch ( addr_meth ) {
			/* ---------------------- Addressing Method -------------- */
			/* Note that decoding mod ModR/M operand adjusts the size of
			 * the instruction, but decoding the reg operand does not. 
			 * This should not cause any problems, as every 'reg' operand 
			 * has an associated 'mod' operand. */
			/* TODO: pass dest_flgs to DecodeModRM */
			case ADDRMETH_E :	/* ModR/M present, Gen reg or memory  */
				bytes += DecodeModRM(buf,dest_buf[x], dest_flg[x], genRegs, size, MODRM_EA);
				break;
			case ADDRMETH_M :	/* ModR/M only refers to memory */
				bytes += DecodeModRM(buf,dest_buf[x], dest_flg[x], genRegs, size, MODRM_EA);
				break;
			case ADDRMETH_Q :	/* ModR/M present, MMX or Memory */
				bytes += DecodeModRM(buf,dest_buf[x], dest_flg[x], reg_mmx, size, MODRM_EA);
				break;
			case ADDRMETH_R  :	/* ModR/M mod == gen reg */
				bytes += DecodeModRM(buf,dest_buf[x], dest_flg[x], genRegs, size, MODRM_EA);
				break;
			case ADDRMETH_W  :	/* ModR/M present, mem or FP reg */
				bytes += DecodeModRM(buf,dest_buf[x], dest_flg[x], reg_fpu, size, MODRM_EA);
				break;

			/* MODRM -- reg operand */
			/* TODO: replace OP_REG with register type flags?? */
			case ADDRMETH_C  :	/* ModR/M reg == control reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], reg_control, size, MODRM_reg);
				break;
			case ADDRMETH_D  :	/* ModR/M reg == debug reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], reg_debug, size, MODRM_reg);
				break;
			case ADDRMETH_G  :	/* ModR/M reg == gen-purpose reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], genRegs, size, MODRM_reg);
				break;
			case ADDRMETH_P  :	/* ModR/M reg == qword MMX reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], reg_mmx, size, MODRM_reg);
				break;
			case ADDRMETH_S  :	/* ModR/M reg == segment reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], reg_seg, size, MODRM_reg);
				break;
			case ADDRMETH_T  :	/* ModR/M reg == test reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], reg_test, size, MODRM_reg);
				break;
			case ADDRMETH_V  :	/* ModR/M reg == FP reg */
				*dest_flg[x] |=OP_REG;
				DecodeModRM(buf, dest_buf[x], dest_flg[x], reg_fpu, size, MODRM_reg);
				break;

			/* No MODRM */
			case ADDRMETH_A  :	/* No modR/M -- direct addr */
				/* sprintf absolute addr, set flag */
				*dest_flg[x] |= OP_DPTR | OP_UNSIGNED;
				GetSizedOperand(dest_buf[x], buf + bytes, size, OP_UNSIGNED);
				bytes += size;
				break;
			case ADDRMETH_F  :	/* EFLAGS register */
				/* sprintf flags reg */
				*dest_flg[x] |= OP_REG ;
				strcpy(dest_buf[x], "EFLAGS");
				break;
			case ADDRMETH_I  :	/* Immediate val */
				/* sprintf imm val , set flag */
				*dest_flg[x] |= OP_IMM | OP_SIGNED;
				GetSizedOperand(dest_buf[x], buf + bytes, size, OP_SIGNED);
				bytes += size;
				break;
			case ADDRMETH_J  :	/* Rel offset to add to IP [jmp] */
				/* sprintf rel addr , set flag */
				*dest_flg[x] |= OP_IND | OP_SIGNED;
				GetSizedOperand(dest_buf[x], buf + bytes, size, OP_SIGNED);
				bytes += size;
				break;
			case ADDRMETH_O  :	/* No ModR/M;operand is word/dword offset */
				/* sprintf rel addr , set flag */
				*dest_flg[x] |= OP_IND | OP_SIGNED;
				GetSizedOperand(dest_buf[x], buf + bytes, size, OP_SIGNED);
				bytes += size;
				break;
			case ADDRMETH_X  :	/* Memory addressed by DS:SI [string!] */
				/* sprintf ds si , set flag */
				*dest_flg[x] |=OP_STRPTR;
				strcpy(dest_buf[x], "DS:ESI");
				break;
			case ADDRMETH_Y  :	/* Memory addressed by ES:DI [string ] */
				/* sprintf es di , set flag */
				*dest_flg[x] |= OP_STRPTR;
				strcpy(dest_buf[x], "ES:EDI");
				break;

			case 0:				/* Operand is not used */
			default:
				/* ignore -- operand not used in this intruction */
				break;
		}
	}	/* end foreach operand */


	return(bytes);
	/* CODE struct should be full now -- printf it to be sure */
}



/* this function is called by the controlling disassembler, so its name and
 * calling convention cannot be changed */
int DisasmAddress( BYTE *buf, BYTE tbl, struct code *c, DWORD rva){
	BYTE op = buf[0];	/* byte value -- 'opcode' */
	int x, size = 1;		/* Automatically advance position 1 byte */
	instr *t;			/* table in i386.opcode.map */

	/* Make magic adjustments for skewed tables  */
	t = tables86[tbl].table;
	if (tables86[tbl].divisor) {op /=  tables86[tbl].divisor; tbl--;};
	if (tables86[tbl].mask) {
			  /* modR/M byte: reduce size to allow decoding of byte */
			  if ( (unsigned char) tables86[tbl].mask != 0xFF) size --;
			  op &= tables86[tbl].mask;
	}
//printf("DEBUG: op %x table %d\n", op, tbl);
	
	/* switch based on flags for this byte */
	if ( t[op].mnemFlg & INSTR_PREFIX){
		for ( x = 0; prefix_table[x][0] != 0; x++) {
			/* cheat by storing flag for prefix in the CODE struct.
			 * this enables it to be passed to instr decode w/o using
			 * any global vars or ruing the recursion of this function */
			if (prefix_table[x][0] == op )
				c->flags |= prefix_table[x][1];
		}
		if (t[op].mnemonic[0])		/* some prefixs have no mnemonics */
			strcat(c->mnemonic, t[op].mnemonic);
		/* Continue Processing INstruction */
		size += DisasmAddress(&buf[1], tbl, c, rva);
	} else if ( t[op].table &&  ! t[op].mnemonic[0] ) {
		/* This is an escape to an additional opcode table */
		size += DisasmAddress(&buf[1], t[op].table, c, rva);
	} else if ( ! t[op].mnemonic[0]  ) {
		/* This is not a valid opcode */
		strcpy(c->mnemonic, "invalid"); 
	} else {
		/* A valid opcode cas been found; disassemble it */
		size += InstDecode( &t[op], &buf[size], c, rva); 
	}
	
	return size;
}
