#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./libdis.h"
#include "./i386.h"

#ifdef _MSC_VER
        #define snprintf        _snprintf
        #define inline          __inline
#endif


/* =========================================================== INIT/TERM */
struct EXT__ARCH ext_arch = {{0}};
static DISASM_REPORTER reporter_func = NULL;

int x86_init( enum x86_options options, DISASM_REPORTER reporter )
{
        ext_arch.options = (int) options;
        reporter_func = reporter;

        ext_arch_init( &ext_arch );
        return( 1 );
}

void x86_set_options( enum x86_options options ){
        ext_arch.options = (int) options;
}

int x86_get_options( void ) {
        return( ext_arch.options );
}

int x86_cleanup( void )
{
        ext_arch_cleanup( );
        return( 1 );
}

/* =========================================================== ERRORS */
void x86_report_error( enum x86_report_codes code, void *arg ) {
        if ( reporter_func ) {
                (*reporter_func)(code, arg);
        }
}

/* =========================================================== UTILITY */


static __inline void get_reg( int id, x86_reg_t *reg ) {
        strncpy( reg->name, vm_get_reg_name( id ), MAX_REGNAME );
        reg->type = vm_get_reg_type( id );
        reg->size = vm_get_reg_size( id );
        reg->id = id;
}

/* Each addrexp can only be used once; this call removes the addr_exp from
 * the list. */
static int get_ea_from_addrexp( unsigned long id, x86_ea_t *ea ) {
        struct EXPRLIST e;
        if (! ea ) {
                return( 0 );
        }

        memset( ea, 0, sizeof(x86_ea_t) );

        if ( addrexp_get(id, &e) ) {
                ea->disp_sign = ea->disp_size = 0;
                ea->disp = (long) e.disp;
                ea->scale = e.scale ? e.scale : 1;
                if ( AddrExp_IndexType(e.flags) ) {
                        get_reg(e.index, &ea->index);
                }
                if ( AddrExp_BaseType(e.flags) ) {
                        get_reg(e.base, &ea->base);
                }
                /* get size, sign of displacement */
                if ( AddrExp_DispType(e.flags) == ADDREXP_BYTE ) {
                        if ( (char) e.disp < 0 ) {
                                ea->disp_sign = 1;
                        }
                        ea->disp_size = 1;
                } else if ( AddrExp_DispType(e.flags) == ADDREXP_WORD ) {
                        if ( (short) e.disp < 0 ) {
                                ea->disp_sign = 1;
                        }
                        ea->disp_size = 2;
                } else if ( AddrExp_DispType(e.flags) == ADDREXP_DWORD ) {
                        if ( (long) e.disp < 0 ) {
                                ea->disp_sign = 1;
                        }
                        ea->disp_size = 4;
                }
                return(1);
        }

        return(0);
}

static int insn_op_from_code( x86_op_t *op, qword op_val, int op_type ){
        int iop = (int) op_val;

        if (! op ) { return(0); }

        if (! op_type ) {
                op->type = op_unused;
                return(1);
        }
        /* set operand type and value */
        op->access = OP_PERM(op_type);
        op->type = OP_TYPE(op_type) >> 8;
        if (! op->type ) {
                op->type = op_unknown;
        }
        op->flags = OP_MOD(op_type) >> 12;
        op->datatype = OP_SIZE(op_type) >> 24;
        if (! op->datatype ) {
                op->datatype = op_dword;
        }

        /* TODO: handle float, etc types */
        switch ( OP_TYPE(op_type) ) {
                case OP_REG:
                        /* op_val is reg ID */
                        get_reg( iop, &op->data.reg );
                        break;
                case OP_IMM:
                        if ( OP_SIZE(op_type) == OP_BYTE ) {
                                if ( OP_MOD(op_type) & OP_SIGNED )
                                        op->data.sbyte = (char) iop;
                                else
                                        op->data.byte = (unsigned char) iop;
                        } else if (OP_SIZE(op_type) == OP_HWORD ) {
                                if ( OP_MOD(op_type) & OP_SIGNED )
                                        op->data.sword = (short) iop;
                                else
                                        op->data.word = (unsigned short) iop;
                        } else if (OP_SIZE(op_type) == OP_DWORD ) {
                                op->data.sqword = op_val;
                        } else  { /* WORD is default */
                                if ( OP_MOD(op_type) & OP_SIGNED )
                                        op->data.sdword = (long) iop;
                                else
                                        op->data.dword = (unsigned long) iop;
                        }
                        break;
                case OP_REL:
                        /* op_val is a signed something */
                        if ( OP_SIZE(op_type) == OP_BYTE ) {
                                op->data.near_offset = (char) iop;
                        } else {
                                op->data.far_offset = (long) iop;
                        }
                        break;
                case OP_ADDR:
                        /* op_val is an unsigned long */
                        op->data.address = (void *) ((unsigned long) iop);
                        break;
                case OP_EXPR:
                        /* op_val is expr ID */
                        get_ea_from_addrexp( (unsigned long) iop,
                                             &op->data.effective_addr);
                        break;
                case OP_OFF:
                        /* op_val is an unsigned long */
                        op->data.offset = (unsigned long) iop;
                        break;
                default:
                        break;
        }
        return(1);
}

/*
 * concatenation macros.  STRNCATF concatenates a format string, buf
 * only with one argument.
 */
#define STRNCAT( buf, str, len ) do {   				\
	int _i = strlen(str), _blen = strlen(buf), _len = len - 1;  	\
	if ( len ) {							\
        	strncat( buf, str, _len );  				\
		if ( _len <= _i ) {					\
			buf[_blen+_len] = '\0';				\
			len = 0;					\
		} else {						\
			len -= _i;					\
		}							\
	}								\
} while( 0 )

#define STRNCATF( buf, fmt, data, len ) do {        \
        char _tmp[MAX_OP_STRING];                   \
                                                    \
        snprintf( _tmp, sizeof _tmp, fmt, data );   \
        STRNCAT( buf, _tmp, len );                  \
} while( 0 )


#define PRINT_DISPLACEMENT( ea ) do {                            \
        if ( ea->disp ) {                                        \
                if ( ea->disp_size > 1 && ! ea->disp_sign ) {    \
                        STRNCATF( buf, "0x%lX", ea->disp, len );  \
                } else {                                         \
                        STRNCATF( buf, "%ld", ea->disp, len );    \
                }                                                \
        }                                                        \
} while( 0 )


static char *prefix_strings[] = {
        "",     /* no prefix */
        "repz ", /* the trailing spaces make it easy to prepend to mnemonic */
        "repnz ",
        "lock ",
        "branch delay " /* unused in x86 */
};

static int x86insn_from_code( x86_insn_t *insn, struct code *code ) {
        unsigned int flags;

        if ( ! insn || ! code ) {
                return(0);
        }

        insn->group = INS_GROUP(code->mnem_type) >> 12;
        insn->type = INS_TYPE(code->mnem_type);
        insn->prefix = INS_MOD(code->mnem_type) >> 20;
        if ( (unsigned int) insn->prefix > 8 ) {
                insn->prefix = insn_no_prefix;
        }

        /* handle flags effected */
        flags = INS_FLAGS_TEST(code->flags_st);
        /* handle weird OR cases */
        /* these are either JLE (ZF | SF<>OF) or JBE (CF | ZF) */
        if (flags & INS_TEST_OR) {
                flags &= ~INS_TEST_OR;
                if ( flags & INS_TEST_ZERO ) {
                        flags &= ~INS_TEST_ZERO;
                        if ( flags & INS_TEST_CARRY ) {
                                flags &= ~INS_TEST_CARRY ;
                                flags |= (int)insn_carry_or_zero_set;
                        } else if ( flags & INS_TEST_SFNEOF ) {
                                flags &= ~INS_TEST_SFNEOF;
                                flags |= (int)insn_zero_set_or_sign_ne_oflow;
                        }
                }
        }
        insn->flags_tested = flags;

        insn->flags_set = INS_FLAGS_SET(code->flags_st) >> 16;


        /* concat all prefix strings */
        if ( (int)insn->prefix & 1 ) {
                strncat(insn->prefix_string, prefix_strings[1], 32 - 
				strlen(insn->prefix_string));
        } else if ( (int)insn->prefix & 2 ) {
                strncat(insn->prefix_string, prefix_strings[2], 32 - 
				strlen(insn->prefix_string));
        }
        if ( (int)insn->prefix & 4 ) {
                strncat(insn->prefix_string, prefix_strings[3], 32  - 
				strlen(insn->prefix_string));
        }
        if ( (int)insn->prefix & 8 ) {
                strncat(insn->prefix_string, prefix_strings[4], 32 - 
				strlen(insn->prefix_string));
        }

        /* create mnemonic and operands */
        strncpy(insn->mnemonic, code->mnemonic, 7);
        insn_op_from_code( &insn->operands[(int)op_dest], code->dest,
                           code->dest_type );
        insn_op_from_code( &insn->operands[(int)op_src], code->src,
                           code->src_type );
        insn_op_from_code( &insn->operands[(int)op_imm], code->aux,
                           code->aux_type );

        return(1);

}

int x86_disasm( unsigned char *buf, unsigned int buf_len,
                unsigned long buf_rva, unsigned int offset,
                x86_insn_t *insn ){
        int len, size;
        struct code c = { 0 };
        unsigned char disasm_buf[32] = {0};

        if ( ! buf || ! insn || ! buf_len ) {
                /* caller screwed up somehow */
                return(0);
        }

        /* clear insn struct */
        memset(insn, 0, sizeof (x86_insn_t));

        if ( offset >= buf_len ) {
                /* another caller screwup ;) */
                x86_report_error(report_disasm_bounds, (char*)buf_rva+offset);
                return(0);
        }

        /* copy binary code to temporary buffer */
        len = buf_len - offset;
        memcpy( disasm_buf, buf + offset, (len > cpu_inst_size()) ?
                                       cpu_inst_size() : len  );

        /* actually do the disassembly */
        size = disasm_addr(disasm_buf, 0, &c, 0);

        /* check and see if we had an invalid instruction */
        if (! size ) {
                x86_report_error(report_invalid_insn, (char*)buf_rva+offset );
                return(0);
        }

        /* check if we overran the end of the buffer */
        if ( size > len ) {
                x86_report_error( report_insn_bounds, (char*)buf_rva + offset );
                return(0);
        }

        /* fill x86_insn_t with struct code stuff */
        x86insn_from_code( insn, &c );

        /* fill rva, offset, and bytes fields of insn */
        insn->addr = buf_rva + offset;
        insn->offset = offset;
        insn->size = size;
        memcpy( insn->bytes, buf + offset, size );

        return (size);
}

int x86_disasm_range( unsigned char *buf, unsigned long buf_rva,
                      unsigned int offset, unsigned int len,
                      DISASM_CALLBACK func, void *arg ) {
        x86_insn_t insn;
        int buf_len, size, count = 0, bytes = 0;

        /* buf_len is implied by the arguments */
        buf_len = len + offset;

        while ( bytes < len ) {
                size = x86_disasm( buf, buf_len, buf_rva, offset + bytes,
                                   &insn );
                if ( size ) {
                        /* invoke callback if it exists */
                        if ( func ) {
                                (*func)( &insn, arg );
                        }
                        bytes += size;
                        count ++;
                } else {
                        /* error */
                        bytes++;        /* try next byte */
                }
        }

        return( count );
}

static inline int follow_insn_dest( x86_insn_t *insn ) {
        if ( insn->type == insn_jmp || insn->type == insn_jcc ||
             insn->type == insn_call || insn->type == insn_callcc ) {
                return(1);
        }
        return(0);
}

static inline int insn_doesnt_return( x86_insn_t *insn ) {
        return( (insn->type == insn_jmp || insn->type == insn_return) ? 1: 0 );
}

static long internal_resolver( x86_op_t *op, x86_insn_t *insn ){
        long next_addr = -1;
        if ( op->type == op_absolute || op->type == op_offset ) {
                next_addr = op->data.sdword;
        } else if ( op->type == op_relative ){
                /* add offset to current rva+size based on op size */
                if ( op->datatype == op_byte ) {
                        next_addr = insn->addr + insn->size + op->data.sbyte;
                } else if ( op->datatype == op_word ) {
                        next_addr = insn->addr + insn->size + op->data.sword;
                } else if ( op->datatype == op_dword ) {
                        next_addr = insn->addr + insn->size + op->data.sdword;
                }
        }
        return( next_addr );
}

int x86_disasm_forward( unsigned char *buf, unsigned int buf_len,
                        unsigned long buf_rva, unsigned int offset,
                        DISASM_CALLBACK func, void *arg,
                        DISASM_RESOLVER resolver ){
        x86_insn_t insn;
        x86_op_t *op;
        long next_addr;
        unsigned long next_offset;
        int size, count = 0, bytes = 0, cont = 1;

        while ( cont && bytes < buf_len ) {
                size = x86_disasm( buf, buf_len, buf_rva, offset + bytes,
                           &insn );

                if ( size ) {
                        /* invoke callback if it exists */
                        if ( func ) {
                                (*func)( &insn, arg );
                        }
                        bytes += size;
                        count ++;
                } else {
                        /* error */
                        bytes++;        /* try next byte */
                }

                if ( follow_insn_dest(&insn) ) {
                        op = &insn.operands[0];
                        next_addr = -1;

                        /* if caller supplied a resolver, use it to determine
                         * the address to disassemble */
                        if ( resolver ) {
                                next_addr = resolver(op, &insn);
                        } else {
                                next_addr = internal_resolver(op, &insn);
                        }

                        if (next_addr != -1 ) {
                                next_offset = next_addr - buf_rva;
                                /* if offset is in this buffer... */
                                if ( next_offset >= 0 &&
                                     next_offset < buf_len ) {
                                        /* go ahead and disassemble */
                                        count += x86_disasm_forward( buf,
                                                            buf_len,
                                                            buf_rva,
                                                            next_offset,
                                                            func, arg,
                                                            resolver );
                                } else  {
                                        /* report unresolved address */
                                        x86_report_error( report_disasm_bounds,
                                                     (void *) next_addr );
                                }
                        }
                } /* end follow_insn */

                if ( insn_doesnt_return(&insn) ) {
                        /* stop disassembling */
                        cont = 0;
                }
        }
        return( count );
}

static int format_insn_prefix_str( enum x86_insn_prefix prefix, char *buf,
                                   int len ) {

        int len_orig = len;

        /* concat all prefix strings */
        if ( prefix & 1 ) { STRNCAT( buf, prefix_strings[1], len ); }
        if ( prefix & 2 ) { STRNCAT( buf, prefix_strings[2], len ); }
        if ( prefix & 4 ) { STRNCAT( buf, prefix_strings[3], len ); }
        if ( prefix & 8 ) { STRNCAT( buf, prefix_strings[4], len ); }

        /* return the number of characters added */
        return (len_orig - len);
}

/*
 * sprint's an operand's data to string str.
 */
static void get_operand_data_str( x86_op_t *op, char *str, int len ){

        if ( op->flags & op_signed ) {
                switch ( op->datatype ) {
                        case op_byte:
                                snprintf( str, len, "%d", op->data.sbyte );
                                return;
                        case op_word:
                                snprintf( str, len, "%d", op->data.sword );
                                return;
                        case op_qword:
                                snprintf( str, len, "%lld", op->data.sqword );
                                return;
                        default:
                                snprintf( str, len, "%ld", op->data.sdword );
                                return;
                }
        }

        //else
        switch ( op->datatype ) {
                case op_byte:
                        snprintf( str, len, "0x%02X", op->data.byte );
                        return;
                case op_word:
                        snprintf( str, len, "0x%04X", op->data.word );
                        return;
                case op_qword:
                        snprintf( str, len, "0x%08llX",op->data.sqword );
                        return;
                default:
                        snprintf( str, len, "0x%08lX", op->data.dword );
                        return;
        }
}

/*
 * sprints register types to a string.  the register types can be ORed
 * together.
 */
static void get_operand_regtype_str( int regtype, char *str, int len )
{
        static struct {
                char *name;
                int value;
        } operand_regtypes[] = {
                {"reg_gen"    , 0x00001},
                {"reg_in"     , 0x00002},
                {"reg_out"    , 0x00004},
                {"reg_local"  , 0x00008},
                {"reg_fpu"    , 0x00010},
                {"reg_seg"    , 0x00020},
                {"reg_simd"   , 0x00040},
                {"reg_sys"    , 0x00080},
                {"reg_sp"     , 0x00100},
                {"reg_fp"     , 0x00200},
                {"reg_pc"     , 0x00400},
                {"reg_retaddr", 0x00800},
                {"reg_cond"   , 0x01000},
                {"reg_zero"   , 0x02000},
                {"reg_ret"    , 0x04000},
                {"reg_src"    , 0x10000},
                {"reg_dest"   , 0x20000},
                {"reg_count"  , 0x40000},
                {NULL, 0}, //end
        };

        unsigned int i;

        memset( str, 0, len );

        //go thru every type in the enum
        for ( i = 0; operand_regtypes[i].name; i++ ) {
                //skip if type is not set
                if(! (regtype & operand_regtypes[i].value) )
                        continue;

                //not the first time around
                if( str[0] ) {
                        STRNCAT( str, "|", len );
                }

                STRNCAT(str, operand_regtypes[i].name, len );
        }
}

static int format_expr( x86_ea_t *ea, char *buf, int len,
                        enum x86_asm_format format ) {
        char str[MAX_OP_STRING];

        if ( format == att_syntax ) {
                PRINT_DISPLACEMENT(ea);
                STRNCAT( buf, "(", len );

                if ( ea->base.name[0]) {
                        STRNCATF( buf, "%%%s", ea->base.name, len );
                }
                if ( ea->index.name[0]) {
                        STRNCATF( buf, ",%%%s", ea->index.name, len );
                        if ( ea->scale > 1 ) {
                                STRNCATF( buf, ",%d", ea->scale, len );
                        }
                }
                /* handle the syntactic exception */
                if ( ! ea->base.name[0] &&
                     ! ea->index.name[0]   ) {
                        STRNCATF( buf, ",%d", ea->scale, len );
                }

                STRNCAT( buf, ")", len );

        } else if ( format == xml_syntax ){

                if ( ea->base.name[0]) {
                        STRNCAT (buf, "\t\t\t<base>\n", len);

                        get_operand_regtype_str (ea->base.type, str,
                                                 sizeof str);
                        STRNCAT (buf, "\t\t\t\t<register ", len);
                        STRNCATF (buf, "name=\"%s\" ", ea->base.name, len);
                        STRNCATF (buf, "type=\"%s\" ", str, len);
                        STRNCATF (buf, "size=%d/>\n", ea->base.size, len);

                        STRNCAT (buf, "\t\t\t</base>\n", len);
                }

                if ( ea->index.name[0]) {
                        STRNCAT (buf, "\t\t\t<index>\n", len);

                        get_operand_regtype_str (ea->index.type, str,
                                                 sizeof str);

                        STRNCAT (buf, "\t\t\t\t<register ", len);
                        STRNCATF (buf, "name=\"%s\" ", ea->index.name, len);
                        STRNCATF (buf, "type=\"%s\" ", str, len);
                        STRNCATF (buf, "size=%d/>\n", ea->index.size, len);

                        STRNCAT (buf, "\t\t\t</index>\n", len);
                }

                //scale
                STRNCAT (buf, "\t\t\t<scale>\n", len);
                STRNCAT (buf, "\t\t\t\t<immediate ", len);
                STRNCATF (buf, "value=\"%d\"/>\n", ea->scale, len);
                STRNCAT (buf, "\t\t\t</scale>\n", len);

                if ( ea->disp ) {

                        STRNCAT (buf, "\t\t\t<displacement>\n", len);

                        if ( ea->disp_size > 1 && ! ea->disp_sign ) {
                                STRNCAT (buf, "\t\t\t\t<address ", len);
                                STRNCATF (buf, "value=\"0x%lX\"/>\n", ea->disp,
                                          len);
                        } else {
                                STRNCAT (buf, "\t\t\t\t<immediate ", len);
                                STRNCATF (buf, "value=%ld/>\n", ea->disp, len);
                        }

                        STRNCAT (buf, "\t\t\t</displacement>\n", len);
                }

        } else if ( format == raw_syntax ) {

                PRINT_DISPLACEMENT(ea);
                STRNCAT( buf, "(", len );

                STRNCATF( buf, "%s,", ea->base.name, len );
                STRNCATF( buf, "%s", ea->index.name, len );
                STRNCATF( buf, "%d", ea->scale, len );
                STRNCAT( buf, ")", len );

        } else {

                STRNCAT( buf, "[", len );

                if ( ea->base.name[0] ) {
                        STRNCAT( buf, ea->base.name, len );
                        if ( ea->index.name[0] ||
                             (ea->disp && ! ea->disp_sign) ) {
                                STRNCAT( buf, "+", len );
                        }
                }
                if ( ea->index.name[0] ) {
                        STRNCAT( buf, ea->index.name, len );
                        if ( ea->scale > 1 )
                        {
                                STRNCATF( buf, "*%d", ea->scale, len );
                        }
                        if ( ea->disp && ! ea->disp_sign )
                        {
                                STRNCAT( buf, "+", len );
                        }
                }

                if (ea->disp || (! ea->index.name[0] && ! ea->base.name[0]))
                {
                        PRINT_DISPLACEMENT(ea);
                }

                STRNCAT( buf, "]", len );
        }

        return( strlen(buf) );
}

static int format_seg( x86_op_t *op, char *buf, int len,
                       enum x86_asm_format format ) {
        int len_orig = len;
        char *reg = "";

        if (! op || ! buf || ! len || ! op->flags) {
                return(0);
        }
        if ( op->type != op_absolute && op->type != op_offset &&
             op->type != op_expression ){
                return(0);
        }
        if (! (int) op->flags & 0xF00 ) {
                return(0);
        }

        switch (op->flags & 0xF00) {
                case op_es_seg: reg = "es"; break;
                case op_cs_seg: reg = "cs"; break;
                case op_ss_seg: reg = "ss"; break;
                case op_ds_seg: reg = "ds"; break;
                case op_fs_seg: reg = "fs"; break;
                case op_gs_seg: reg = "gs"; break;
                default:
                        break;
        }

        if (! reg[0] ) {
                return( 0 );
        }

        switch( format ) {
                case xml_syntax:
                        STRNCAT( buf, "\t\t\t<segment ", len );
                        STRNCATF( buf, "value=\"%s\"/>\n", reg, len );
                        break;
                case att_syntax:
                        STRNCATF( buf, "%%%s:", reg, len );
                        break;

                default:
                        STRNCATF( buf, "%s:", reg, len );
                        break;
        }

        return( len_orig - len ); /* return length of appended string */
}

static char *get_operand_datatype_str( x86_op_t *op ){

        static char *types[] = {
                "sbyte",
                "sword",
                "sqword",
                "sdword",
                "byte",
                "word",
                "qword",
                "dword",
        };

        if ( op->flags & op_signed ) {
                switch (op->datatype) {
                        case op_byte:  return types[0];
                        case op_word:  return types[1];
                        case op_qword: return types[2];
                        default:       return types[3];
                }
        }

        //else
        switch (op->datatype) {
                case op_byte:  return types[4];
                case op_word:  return types[5];
                case op_qword: return types[6];
                default:       return types[7];
        }
}

static int format_insn_eflags_str( enum x86_flag_status flags, char *buf,
                                   int len) {

        struct {
                char *name;
                int  value;
        } insn_flags[] = {
                { "carry_set ",                 0x0001 },
                { "zero_set ",                  0x0002 },
                { "oflow_set ",                 0x0004 },
                { "dir_set ",                   0x0008 },
                { "sign_set ",                  0x0010 },
                { "parity_set ",                0x0020 },
                { "carry_or_zero_set ",         0x0040 },
                { "zero_set_or_sign_ne_oflow ", 0x0080 },
                { "carry_clear ",               0x0100 },
                { "zero_clear ",                0x0200 },
                { "oflow_clear ",               0x0400 },
                { "dir_clear ",                 0x0800 },
                { "sign_clear ",                0x1000 },
                { "parity_clear ",              0x2000 },
                { "sign_eq_oflow ",             0x4000 },
                { "sign_ne_oflow ",             0x8000 },
                { NULL,                         0x0000 }, //end
        };

        unsigned int i;
        int len_orig = len;

        for (i = 0; insn_flags[i].name; i++) {
                if (! (flags & insn_flags[i].value) )
                        continue;

                STRNCAT( buf, insn_flags[i].name, len );
        }

        return( len_orig - len );
}

static char *get_insn_group_str( enum x86_insn_group gp ) {

        static char *types[] = {
                "",           // 0
                "controlflow",// 1
                "arithmetic", // 2
                "logic",      // 3
                "stack",      // 4
                "comparison", // 5
                "move",       // 6
                "string",     // 7
                "bit_manip",  // 8
                "flag_manip", // 9
                "fpu",        // 10
                "",           // 11
                "",           // 12
                "interrupt",  // 13
                "system",     // 14
                "other",      // 15
        };

        if ( gp > sizeof (types)/sizeof(types[0]) )
                return "";

        return types[gp];
}

static char *get_insn_type_str( enum x86_insn_type type ) {

        static struct {
                char *name;
                int  value;
        } types[] = {
                /* insn_controlflow */
                { "jmp", 0x1001 },
                { "jcc", 0x1002 },
                { "call", 0x1003 },
                { "callcc", 0x1004 },
                { "return", 0x1005 },
                { "loop", 0x1006 },
                /* insn_arithmetic */
                { "add", 0x2001 },
                { "sub", 0x2002 },
                { "mul", 0x2003 },
                { "div", 0x2004 },
                { "inc", 0x2005 },
                { "dec", 0x2006 },
                { "shl", 0x2007 },
                { "shr", 0x2008 },
                { "rol", 0x2009 },
                { "ror", 0x200A },
                /* insn_logic */
                { "and", 0x3001 },
                { "or", 0x3002 },
                { "xor", 0x3003 },
                { "not", 0x3004 },
                { "neg", 0x3005 },
                /* insn_stack */
                { "push", 0x4001 },
                { "pop", 0x4002 },
                { "pushregs", 0x4003 },
                { "popregs", 0x4004 },
                { "pushflags", 0x4005 },
                { "popflags", 0x4006 },
                { "enter", 0x4007 },
                { "leave", 0x4008 },
                /* insn_comparison */
                { "test", 0x5001 },
                { "cmp", 0x5002 },
                /* insn_move */
                { "mov", 0x6001 },      /* move */
                { "movcc", 0x6002 },    /* conditional move */
                { "xchg", 0x6003 },     /* exchange */
                { "xchgcc", 0x6004 },   /* conditional exchange */
                /* insn_string */
                { "strcmp", 0x7001 },
                { "strload", 0x7002 },
                { "strmov", 0x7003 },
                { "strstore", 0x7004 },
                { "translate", 0x7005 },        /* xlat */
                /* insn_bit_manip */
                { "bittest", 0x8001 },
                { "bitset", 0x8002 },
                { "bitclear", 0x8003 },
                /* insn_flag_manip */
                { "clear_carry", 0x9001 },
                { "clear_zero", 0x9002 },
                { "clear_oflow", 0x9003 },
                { "clear_dir", 0x9004 },
                { "clear_sign", 0x9005 },
                { "clear_parity", 0x9006 },
                { "set_carry", 0x9007 },
                { "set_zero", 0x9008 },
                { "set_oflow", 0x9009 },
                { "set_dir", 0x900A },
                { "set_sign", 0x900B },
                { "set_parity", 0x900C },
                { "tog_carry", 0x9010 },
                { "tog_zero", 0x9020 },
                { "tog_oflow", 0x9030 },
                { "tog_dir", 0x9040 },
                { "tog_sign", 0x9050 },
                { "tog_parity", 0x9060 },
                /* insn_fpu */
                { "fmov", 0xA001 },
                { "fmovcc", 0xA002 },
                { "fneg", 0xA003 },
                { "fabs", 0xA004 },
                { "fadd", 0xA005 },
                { "fsub", 0xA006 },
                { "fmul", 0xA007 },
                { "fdiv", 0xA008 },
                { "fsqrt", 0xA009 },
                { "fcmp", 0xA00A },
                { "fcos", 0xA00C },
                { "fldpi", 0xA00D },
                { "fldz", 0xA00E },
                { "ftan", 0xA00F },
                { "fsine", 0xA010 },
                { "fsys", 0xA020 },
                /* insn_interrupt */
                { "int", 0xD001 },
                { "intcc", 0xD002 },    /* not present in x86 ISA */
                { "iret", 0xD003 },
                { "bound", 0xD004 },
                { "debug", 0xD005 },
                { "trace", 0xD006 },
                { "invalid_op", 0xD007 },
                { "oflow", 0xD008 },
                /* insn_system */
                { "halt", 0xE001 },
                { "in", 0xE002 },       /* input from port/bus */
                { "out", 0xE003 },      /* output to port/bus */
                { "cpuid", 0xE004 },
                /* insn_other */
                { "nop", 0xF001 },
                { "bcdconv", 0xF002 },  /* convert to or from BCD */
                { "szconv", 0xF003 },   /* change size of operand */
                { NULL, 0 }, //end
        };

        unsigned int i;

        //go thru every type in the enum
        for ( i = 0; types[i].name; i++ ) {
                if ( types[i].value == type )
                        return types[i].name;
        }

        return "";
}

static int format_operand_att( x86_op_t *op, x86_insn_t *insn, char *buf,
                               int len){

        char str[MAX_OP_STRING];

        memset (str, 0, sizeof str);

        switch ( op->type ) {
                case op_register:
                        STRNCATF( buf, "%%%s", op->data.reg.name, len );
                        break;

                case op_immediate:
                        get_operand_data_str( op, str, sizeof str );
                        STRNCATF( buf, "$%s", str, len );
                        break;

                case op_relative:
                        if (op->datatype == op_byte) {
                                STRNCATF( buf, "0x%02X",
                                          (unsigned char)(op->data.sbyte +
                                          insn->addr + insn->size), len );
                                break;
                        }

                        if (op->datatype == op_word) {
                                STRNCATF( buf, "0x%04X",
                                          (unsigned int)(op->data.sword +
                                          insn->addr + insn->size), len );
                                break;
                        }

                        //else
                        STRNCATF( buf, "0x%08X",
                                  (unsigned int)(op->data.sdword +
                                  insn->addr + insn->size), len );
                        break;

                case op_absolute:
                        /* ATT requires a '*' before absolute JMP/CALL ops */
                        if (insn->type == insn_jmp || insn->type == insn_call)
                                STRNCAT( buf, "*", len );

                        len -= format_seg( op, buf, len, att_syntax );
                        STRNCATF( buf, "0x%08lX", op->data.sdword, len );
                        break;

                case op_expression:
                        len -= format_seg( op, buf, len, att_syntax );
                        len -= format_expr( &op->data.effective_addr, buf, len,
                                     att_syntax );
                        break;

                case op_offset:
                        if ( insn->type == insn_jmp || insn->type == insn_call )
                                STRNCAT( buf, "*", len );

                        len -= format_seg( op, buf, len, att_syntax );

                        if (op->flags & op_pointer) {
                                STRNCATF( buf, "[0x%08lX]", op->data.sdword, len);
                        } else {
                                STRNCATF( buf, "0x%08lX", op->data.sdword, len );
                        }
                        break;

                case op_unused:
                case op_unknown:
                        /* return 0-truncated buffer */
                        break;
        }

        return ( strlen( buf ) );
}

static int format_operand_native( x86_op_t *op, x86_insn_t *insn, char *buf,
                                  int len){

        char str[MAX_OP_STRING];

        switch (op->type) {
                case op_register:
                        STRNCAT( buf, op->data.reg.name, len );
                        break;

                case op_immediate:
                        get_operand_data_str( op, str, sizeof str );
                        STRNCAT( buf, str, len );
                        break;

                case op_relative:
                        if ( op->datatype == op_byte ) {
                                STRNCATF( buf, "0x%02X",
                                          (unsigned char)(op->data.sbyte +
                                          insn->addr + insn->size), len );
                                break;
                        }

                        if ( op->datatype == op_word ) {
                                STRNCATF( buf, "0x%04X",
                                          (unsigned int)(op->data.sword +
                                          insn->addr + insn->size), len );
                                break;
                        }

                        //else
                        STRNCATF( buf, "0x%08lX", op->data.sdword +
                                  insn->addr + insn->size, len );
                        break;

                case op_absolute:
                        len -= format_seg( op, buf, len, native_syntax );
                        STRNCATF( buf, "0x%08lX", op->data.sdword, len );
                        break;

                case op_expression:
                        len -= format_seg( op, buf, len, native_syntax );
                        len -= format_expr( &op->data.effective_addr, buf, len,
                                     native_syntax );
                        break;

                case op_offset:
                        len -= format_seg( op, buf, len, native_syntax );
                        if ( op->flags & op_pointer ) {
                                STRNCATF( buf, "[0x%08lX]", op->data.sdword, len);
                        } else {
                                STRNCATF( buf, "0x%08lX", op->data.sdword, len );
                        }
                        break;
                case op_unused:
                case op_unknown:
                        /* return 0-truncated buffer */
                        break;
        }

        return( strlen( buf ) );
}

static int format_operand_xml( x86_op_t *op, x86_insn_t *insn, char *buf,
                               int len){

        char str[MAX_OP_STRING] = "\0";

        switch (op->type) {
                case op_register:

                        get_operand_regtype_str( op->data.reg.type, str,
                                                 sizeof str );

                        STRNCAT( buf, "\t\t<register ", len );
                        STRNCATF( buf, "name=\"%s\" ", op->data.reg.name, len );
                        STRNCATF( buf, "type=\"%s\" ", str, len );
                        STRNCATF( buf, "size=%d/>\n", op->data.reg.size, len );
                        break;

                case op_immediate:

                        get_operand_data_str( op, str, sizeof str );

                        STRNCAT( buf, "\t\t<immediate ", len );
                        STRNCATF( buf, "type=\"%s\" ",
                                  get_operand_datatype_str (op), len );
                        STRNCATF( buf, "value=\"%s\"/>\n", str, len );
                        break;

                case op_relative:

                        STRNCAT( buf, "\t\t<relative_offset ", len );

                        if (op->datatype == op_byte) {
                                STRNCATF( buf, "value=\"0x%02X\"/>\n",
                                          (unsigned char)(op->data.sbyte +
                                          insn->addr + insn->size), len );
                                break;
                        }

                        if (op->datatype == op_word) {
                                STRNCATF( buf, "value=\"0x%04X\"/>\n",
                                          (unsigned int)(op->data.sword +
                                          insn->addr + insn->size), len);
                                break;
                        }

                        //else
                        STRNCATF( buf, "value=\"0x%08lX\"/>\n",
                                  op->data.sdword + insn->addr + insn->size,
                                  len );
                        break;

                case op_absolute:

                        STRNCATF( buf, "\t\t<absolute_address value=\"0x%08lX\">\n",
                                  op->data.sdword, len );

                        len -= format_seg( op, buf, len, xml_syntax );

                        STRNCAT( buf, "\t\t</absolute_address>\n", len );
                        break;

                case op_expression:
			

                        STRNCAT( buf, "\t\t<address_expression>\n", len );

                        len -= format_seg( op, buf, len, xml_syntax );
                        len -= format_expr( &op->data.effective_addr, buf, len,
                                     xml_syntax );

                        STRNCAT( buf, "\t\t</address_expression>\n", len );
                        break;

                case op_offset:

                        STRNCAT( buf, "\t\t<segment_offset>\n", len );

                        len -= format_seg( op, buf, len, xml_syntax );

                        STRNCAT( buf, "\t\t\t<address ", len);
                        if ( op->flags & op_pointer ) {
                                STRNCATF( buf, "value=\"[0x%08lX]\"/>\n",
                                          op->data.sdword, len );
                        } else {
                                STRNCATF( buf, "value=\"0x%08lX\"/>\n",
                                          op->data.sdword, len );
                        }
                        STRNCAT( buf, "\t\t</segment_offset>\n", len );
                        break;

                case op_unused:
                case op_unknown:
                        /* return 0-truncated buffer */
                        break;
        }

        return( strlen( buf ) );
}

static int format_operand_raw( x86_op_t *op, x86_insn_t *insn, char *buf,
                               int len){

        char str[MAX_OP_STRING];

        switch (op->type) {
                case op_register:
                        get_operand_regtype_str( op->data.reg.type, str,
                                                 sizeof str );

                        STRNCAT( buf, "reg|reg|", len );
                        STRNCATF( buf, "%s:", op->data.reg.name, len );
                        STRNCATF( buf, "%s:", str, len );
                        STRNCATF( buf, "%d|", op->data.reg.size, len );
                        break;

                case op_immediate:

                        get_operand_data_str( op, str, sizeof str );

                        STRNCAT( buf, "immediate|", len );
                        STRNCATF( buf, "%s|",
                                  get_operand_datatype_str (op), len );
                        STRNCATF( buf, "%s|", str, len );
                        break;

                case op_relative:

                        STRNCAT( buf, "relative|", len );

                        if (op->datatype == op_byte) {
                                STRNCATF( buf, "byte|%02X|",
                                          (unsigned char)(op->data.sbyte +
                                          insn->addr + insn->size), len );
                                break;
                        }

                        if (op->datatype == op_word) {
                                STRNCATF( buf, "word|%04X",
                                          (unsigned int)(op->data.sword +
                                          insn->addr + insn->size), len);
                                break;
                        }

                        //else
                        STRNCATF( buf, "dword|%08lX|",
                                  op->data.sdword + insn->addr + insn->size,
                                  len );
                        break;

                case op_absolute:

                        STRNCAT( buf, "absolute_address|dword|", len );

                        len -= format_seg( op, buf, len, native_syntax );

                        STRNCATF( buf, "%08lX|", op->data.sdword, len );

                        break;

                case op_expression:

                        STRNCAT( buf, "address_expression|?|", len );

                        len -= format_seg( op, buf, len, native_syntax );
                        len -= format_expr( &op->data.effective_addr, buf, len,
                                     raw_syntax );

                        STRNCAT( buf, "|", len );
                        break;

                case op_offset:

                        STRNCAT( buf, "segment_offset|?|", len );

                        len -= format_seg( op, buf, len, xml_syntax );

                        if ( op->flags & op_pointer ) {
                                STRNCATF( buf, "[%08lX]|",
                                          op->data.sdword, len );
                        } else {
                                STRNCATF( buf, "%08lX|",
                                          op->data.sdword, len );
                        }
                        break;

                case op_unused:
                case op_unknown:
                        /* return 0-truncated buffer */
                        break;
        }

        return( strlen( buf ) );
}

int x86_format_operand( x86_op_t *op, x86_insn_t *insn, char *buf, int len,
                        enum x86_asm_format format ){

        if ( ! op || ! buf || len < 1 ) {
                return(0);
        }

        memset( buf, 0, len );

        switch ( format ) {
                case att_syntax:
                        return format_operand_att( op, insn, buf, len );
                case xml_syntax:
                        return format_operand_xml( op, insn, buf, len );
                case raw_syntax:
                        return format_operand_raw( op, insn, buf, len );
                case native_syntax:
                case intel_syntax:
                default:
                        return format_operand_native( op, insn, buf, len );
        }
}

#define is_imm_jmp(op)   (op.type == op_absolute   || \
                          op.type == op_immediate  || \
                          op.type == op_offset)
#define is_memory_op(op) (op.type == op_absolute   || \
                          op.type == op_expression || \
                          op.type == op_offset)

static int format_att_mnemonic( x86_insn_t *insn, char *buf, int len) {
        int size = 0;
        char *prefix = "", *suffix;

        if (! insn || ! buf || ! len )
                return(0);

        memset( buf, 0, len );
        /* do long jump/call prefix */
        if ( insn->type == insn_jmp || insn->type == insn_call ) {
                if ( is_imm_jmp(insn->operands[op_dest]) &&
                    insn->operands[op_dest].datatype != op_byte ) {
                        prefix = "l";
                }
        }
        STRNCAT( buf, prefix, len );

        /* do mnemonic */
        STRNCAT( buf, insn->mnemonic, len );

        /* do suffixes for memory operands */
        if ( is_memory_op(insn->operands[op_dest]) ){
                size = x86_operand_size(&insn->operands[op_dest]);
        } else if ( is_memory_op(insn->operands[op_dest]) ) {
                size = x86_operand_size(&insn->operands[op_src]);
        }

        if ( size == 1 ) suffix = "b";
        else if ( size == 2 ) suffix = "w";
        else if ( size == 4 ) suffix = "l";
        else if ( size == 8 ) suffix = "q";
        else suffix = "";

        STRNCAT( buf, suffix, len );
        return ( strlen( buf ) );
}

int x86_format_mnemonic(x86_insn_t *insn, char *buf, int len,
                        enum x86_asm_format format){
        char str[MAX_OP_STRING];

        memset( buf, 0, len );
        STRNCAT( buf, insn->prefix_string, len );
        if ( format == att_syntax ) {
                format_att_mnemonic( insn, str, sizeof str );
                STRNCAT( buf, str, len );
        } else {
                STRNCAT( buf, insn->mnemonic, len );
        }

        return( strlen( buf ) );
}

static int format_raw_insn( x86_insn_t *insn, char *buf, int len ){
        char str[MAX_OP_STRING];
        int i;
        /* RAW style:
         * ADDR|OFFSET|SIZE|BYTES|
         * PREFIX|PREFIX_STRING|GROUP|TYPE|MNEMONIC|
         * FLAGS_SET|FLAGS_TESTED|
         * BLOCK|FUNCTION|TAG|
         * DEST_TYPE|DEST_DATATYPE|DEST_ACCESS|DEST_FLAGS|
         * SRC_TYPE|SRC_DATATYPE|SRC_ACCESS|SRC_FLAGS|
         * IMM_TYPE|IMM_DATATYPE|IMM_ACCESS|IMM_FLAGS
         *
         * Register values are encoded as:
         * NAME:TYPE:SIZE
         *
         * Effective addresses are encoded as:
         * disp(base_reg,index_reg,scale)
         */
        STRNCATF( buf, "0x%08lX|", insn->addr  , len );
        STRNCATF( buf, "0x%08lX|", insn->offset, len );
        STRNCATF( buf, "%d|"    , insn->size  , len );

        /* print bytes */
        for ( i = 0; i < insn->size; i++ ) {
                STRNCATF( buf, "%02X ", insn->bytes[i], len );
        }
        STRNCAT( buf, "|", len );

        len -= format_insn_prefix_str( insn->prefix, buf, len );
        STRNCATF( buf, "|%s|", insn->prefix_string             , len );
        STRNCATF( buf, "%s|", get_insn_group_str( insn->group ), len );
        STRNCATF( buf, "%s|", get_insn_type_str( insn->type )  , len );
        STRNCATF( buf, "%s|", insn->mnemonic                   , len );

        len -= format_insn_eflags_str( insn->flags_set, buf, len );
        STRNCAT( buf, "|", len );
        len -= format_insn_eflags_str( insn->flags_tested, buf, len );
        STRNCAT( buf, "|", len );

        STRNCATF( buf, "0x%08X|", (unsigned int)insn->block   , len );
        STRNCATF( buf, "0x%08X|", (unsigned int)insn->function, len );
        STRNCATF( buf, "0x%08X|", (unsigned int)insn->tag     , len );

        x86_format_operand(&insn->operands[op_dest], insn, str,
                           sizeof str, raw_syntax);
        STRNCAT( buf, str, len );

        x86_format_operand(&insn->operands[op_src], insn, str,
                           sizeof str, raw_syntax);
        STRNCAT( buf, str, len );

        x86_format_operand(&insn->operands[op_imm], insn, str,
                           sizeof str, raw_syntax);
        STRNCAT( buf, str, len );

        return( strlen (buf) );
}

static int format_xml_insn( x86_insn_t *insn, char *buf, int len ) {
        char str[MAX_OP_XML_STRING];
        int i;

        STRNCAT( buf, "<x86_insn>\n", len );

        STRNCATF( buf, "\t<address rva=\"0x%08lX\" ", insn->addr, len );
        STRNCATF( buf, "offset=\"0x%08lX\" ", insn->offset, len );
        STRNCATF( buf, "size=%d bytes=\"", insn->size, len );

        for ( i = 0; i < insn->size; i++ ) {
                STRNCATF( buf, "%02X ", insn->bytes[i], len );
        }
        STRNCAT( buf, "\"/>\n", len );

        STRNCAT( buf, "\t<prefix type=\"", len );
        len -= format_insn_prefix_str( insn->prefix, buf, len );
        STRNCATF( buf, "\" string=\"%s\"/>\n", insn->prefix_string, len );

        STRNCATF( buf, "\t<mnemonic group=\"%s\" ",
                  get_insn_group_str (insn->group), len );
        STRNCATF( buf, "type=\"%s\" ", get_insn_type_str (insn->type), len );
        STRNCATF( buf, "string=\"%s\"/>\n", insn->mnemonic, len );

        STRNCAT( buf, "\t<flags type=set>\n", len );
        STRNCAT( buf, "\t\t<flag name=\"", len );
        len -= format_insn_eflags_str( insn->flags_set, buf, len );
        STRNCAT( buf, "\"/>\n\t</flags>\n", len );


        STRNCAT( buf, "\t<flags type=tested>\n", len );
        STRNCAT( buf, "\t\t<flag name=\"", len );
        len -= format_insn_eflags_str( insn->flags_tested, buf, len );
        STRNCAT( buf, "\"/>\n\t</flags>\n", len );

        x86_format_operand( &insn->operands[op_dest], insn, str,
                            sizeof str, xml_syntax );
        STRNCAT( buf, "\t<operand name=dest>\n", len );
        STRNCAT( buf, str, len );
        STRNCAT( buf, "\t</operand>\n", len );

        x86_format_operand( &insn->operands[op_src], insn, str,
                            sizeof str, xml_syntax );
        STRNCAT( buf, "\t<operand name=src>\n", len );
        STRNCAT( buf, str, len );
        STRNCAT( buf, "\t</operand>\n", len );

        x86_format_operand( &insn->operands[op_imm], insn, str,
                            sizeof str, xml_syntax );
        STRNCAT( buf, "\t<operand name=imm>\n", len );
        STRNCAT( buf, str, len );
        STRNCAT( buf, "\t</operand>\n", len );

        STRNCAT( buf, "</x86_insn>\n", len );

        return strlen (buf);
}

int x86_format_header( char *buf, int len, enum x86_asm_format format ) {
        switch (format) {
                case att_syntax:
                        snprintf( buf, len, "MNEMONIC\tSRC, DEST, IMM" );
                        break;
                case intel_syntax:
                        snprintf( buf, len, "MNEMONIC\tDEST, SRC, IMM" );
                        break;
                case native_syntax:
                        snprintf( buf, len, "ADDRESS\tBYTES\tMNEMONIC\t"
                                            "DEST\tSRC\tIMM" );
                        break;
                case raw_syntax:
                        snprintf( buf, len, "ADDRESS|OFFSET|SIZE|BYTES|"
                               "PREFIX|PREFIX_STRING|GROUP|TYPE|MNEMONIC|"
                               "FLAGS_SET|FLAGS_TESTED|BLOCK|FUNCTION|TAG|"
                               "DEST_TYPE|DEST_DATATYPE|DEST_ACCESS|DEST_FLAGS|"
                               "SRC_TYPE|SRC_DATATYPE|SRC_ACCESS|SRC_FLAGS|"
                               "IMM_TYPE|IMM_DATATYPE|IMM_ACCESS|IMM_FLAGS"
                                );
                        break;
                case xml_syntax:
                        snprintf( buf, len,
                                  "<x86_insn>"
                                      "<address rva= offset= size= bytes=/>"
                                      "<prefix type= string=/>"
                                      "<mnemonic group= type= string=/>"
                                      "<flags type=set>"
                                          "<flag name=>"
                                      "</flags>"
                                      "<flags type=tested>"
                                          "<flag name=>"
                                      "</flags>"
                                      "<operand name=dest>"
                                          "<register name= type= size=/>"
                                          "<immediate type= value=/>"
                                          "<relative_offset value=/>"
                                          "<absolute_address value=>"
                                              "<segment value=/>"
                                          "</absolute_address>"
                                          "<address_expression>"
                                              "<segment value=/>"
                                              "<base>"
                                                  "<register name= type= size=/>"
                                              "</base>"
                                              "<index>"
                                                  "<register name= type= size=/>"
                                              "</index>"
                                              "<scale>"
                                                  "<immediate value=/>"
                                              "</scale>"
                                              "<displacement>"
                                                  "<immediate value=/>"
                                                  "<address value=/>"
                                              "</displacement>"
                                          "</address_expression>"
                                          "<segment_offset>"
                                              "<address value=/>"
                                          "</segment_offset>"
                                      "</operand>"
                                      "<operand name=src>"
                                          "<register name= type= size=/>"
                                          "<immediate type= value=/>"
                                          "<relative_offset value=/>"
                                          "<absolute_address value=>"
                                              "<segment value=/>"
                                          "</absolute_address>"
                                          "<address_expression>"
                                              "<segment value=/>"
                                              "<base>"
                                                  "<register name= type= size=/>"
                                              "</base>"
                                              "<index>"
                                                  "<register name= type= size=/>"
                                              "</index>"
                                              "<scale>"
                                                  "<immediate value=/>"
                                              "</scale>"
                                              "<displacement>"
                                                  "<immediate value=/>"
                                                  "<address value=/>"
                                              "</displacement>"
                                          "</address_expression>"
                                          "<segment_offset>"
                                              "<address value=/>"
                                          "</segment_offset>"
                                      "</operand>"
                                      "<operand name=imm>"
                                           "<register name= type= size=/>"
                                          "<immediate type= value=/>"
                                          "<relative_offset value=/>"
                                          "<absolute_address value=>"
                                              "<segment value=/>"
                                          "</absolute_address>"
                                          "<address_expression>"
                                              "<segment value=/>"
                                              "<base>"
                                                  "<register name= type= size=/>"
                                              "</base>"
                                              "<index>"
                                                  "<register name= type= size=/>"
                                              "</index>"
                                              "<scale>"
                                                  "<immediate value=/>"
                                              "</scale>"
                                              "<displacement>"
                                                  "<immediate value=/>"
                                                  "<address value=/>"
                                              "</displacement>"
                                          "</address_expression>"
                                          "<segment_offset>"
                                              "<address value=/>"
                                          "</segment_offset>"
                                     "</operand>"
                                  "</x86_insn>"
                                );
                        break;
		case unknown_syntax:
			if ( len ) {
				buf[0] = '\0';
			}
			break;
        }

        return( strlen(buf) );
}

int x86_format_insn( x86_insn_t *insn, char *buf, int len,
                     enum x86_asm_format format ){
        char str[MAX_OP_STRING];
        int i;

        memset(buf, 0, len);
        if ( format == intel_syntax ) {
                /* INTEL STYLE: mnemonic dest, src, imm */
                STRNCAT( buf, insn->prefix_string, len );
                STRNCAT( buf, insn->mnemonic, len );
                STRNCAT( buf, "\t", len );

                /* dest */
                x86_format_operand(&insn->operands[op_dest], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );

                /* src */
                if ( insn->operands[op_src].type != op_unused ) {
                        STRNCAT( buf, ", ", len );
                }
                x86_format_operand(&insn->operands[op_src], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );

                /* imm */
                if ( insn->operands[op_imm].type != op_unused ) {
                        STRNCAT( buf, ", ", len );
                }
                x86_format_operand(&insn->operands[op_imm], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );

        } else if ( format == att_syntax ) {
                /* ATT STYLE: mnemonic src, dest, imm */
                STRNCAT( buf, insn->prefix_string, len );
                format_att_mnemonic(insn, str, MAX_OP_STRING);
                STRNCATF( buf, "%s\t", str, len);

                /* src */
                x86_format_operand(&insn->operands[op_src], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );

                /* dest */
                if ( insn->operands[op_src].type != op_unused &&
                     insn->operands[op_dest].type != op_unused ) {
                        STRNCAT( buf, ", ", len );
                }

                x86_format_operand(&insn->operands[op_dest], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );

                /* imm */
                if ( insn->operands[op_imm].type != op_unused ) {
                        STRNCAT( buf, ", ", len );
                }
                x86_format_operand(&insn->operands[op_imm], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );

        } else if ( format == raw_syntax ) {
                format_raw_insn( insn, buf, len );
        } else if ( format == xml_syntax ) {
                format_xml_insn( insn, buf, len );
        } else { /* default to native */
                /* NATIVE style: RVA\tBYTES\tMNEMONIC\tOPERANDS */
                /* print address */
                STRNCATF( buf, "%08lX\t", insn->addr, len );

                /* print bytes */
                for ( i = 0; i < insn->size; i++ ) {
                        STRNCATF( buf, "%02X ", insn->bytes[i], len );
                }

                STRNCAT( buf, "\t", len );

                /* print mnemonic */
                STRNCAT( buf, insn->prefix_string, len );
                STRNCAT( buf, insn->mnemonic, len );
                STRNCAT( buf, "\t", len );

                /* print operands */
                /* dest */
                x86_format_operand(&insn->operands[op_dest], insn, str,
                                   MAX_OP_STRING, format);
                STRNCATF( buf, "%s\t", str, len );

                /* src */
                x86_format_operand(&insn->operands[op_src], insn, str,
                                   MAX_OP_STRING, format);
                STRNCATF( buf, "%s\t", str, len );

                /* imm */
                x86_format_operand(&insn->operands[op_imm], insn, str,
                                   MAX_OP_STRING, format);
                STRNCAT( buf, str, len );
        }

        return( strlen( buf ) );
}

x86_op_t * x86_get_operand( x86_insn_t *insn, enum x86_operand_id id ){
        if ( insn ) return( &insn->operands[(int)id] );
        return( NULL );
}

x86_op_t * x86_get_dest_operand( x86_insn_t *insn ) {
        if ( insn ) return( &insn->operands[(int)op_dest] );
        return( NULL );
}

x86_op_t * x86_get_src_operand( x86_insn_t *insn ) {
        if ( insn ) return( &insn->operands[(int)op_src] );
        return( NULL );
}

x86_op_t * x86_get_imm_operand( x86_insn_t *insn ) {
        if ( insn ) return( &insn->operands[(int)op_imm] );
        return( NULL );
}

unsigned char * x86_get_raw_imm( x86_insn_t *insn ) {
        int size, offset;
        x86_op_t *op;

        if ( insn ) {
                if ( insn->operands[0].type == op_immediate ) {
                        op = &insn->operands[0];
                } else if ( insn->operands[1].type == op_immediate ) {
                        op = &insn->operands[1];
                } else if ( insn->operands[2].type == op_immediate ) {
                        op = &insn->operands[2];
                } else {
                        return( NULL );
                }
                /* immediate data is at the end of the insn */
                size = x86_operand_size( op );
                offset = insn->size - size;
                return( &insn->bytes[offset] );
        }
        return(NULL);
}


int x86_operand_size( x86_op_t *op ) {
        switch (op->datatype ) {
                case op_byte:    return 1;
                case op_word:    return 2;
                case op_dword:   return 4;
                case op_qword:   return 8;
                case op_dqword:  return 16;
                case op_sreal:   return 4;
                case op_dreal:   return 8;
                case op_extreal: return 10;
                case op_bcd:     return 10;
                case op_simd:    return 16;
                case op_fpuenv:  return 28;
        }
        return(4);      /* default size */
}

void x86_set_insn_addr( x86_insn_t *insn, unsigned long addr ) {
        if ( insn ) insn->addr = addr;
}

void x86_set_insn_offset( x86_insn_t *insn, unsigned int offset ){
        if ( insn ) insn->offset = offset;
}

void x86_set_insn_function( x86_insn_t *insn, void * func ){
        if ( insn ) insn->function = func;
}

void x86_set_insn_block( x86_insn_t *insn, void * block ){
        if ( insn ) insn->block = block;
}

void x86_tag_insn( x86_insn_t *insn ){
        if ( insn ) insn->tag = (void *) 1;
}

void x86_untag_insn( x86_insn_t *insn ){
        if ( insn ) insn->tag = (void *) 0;
}

int x86_insn_is_tagged( x86_insn_t *insn ){
        return( insn && insn->tag ? 1 : 0 );
}

/* accessor functions for the private 'ext_arch' structure */
int x86_endian(void)        { return( ext_arch.endian );  }
int x86_addr_size(void)     { return( ext_arch.sz_addr ); }
int x86_op_size(void)       { return( ext_arch.sz_oper ); }
int x86_word_size(void)     { return( ext_arch.sz_word ); }
int x86_max_inst_size(void) { return( ext_arch.sz_inst ); }
int x86_sp_reg(void)        { return( ext_arch.SP );      }
int x86_fp_reg(void)        { return( ext_arch.FP );      }
int x86_ip_reg(void)        { return( ext_arch.IP );      }

static int invariant_op_from_optype( int op_type, x86_invariant_op_t *inv_op ) {
	if (! inv_op ) {
		return(0);
	}
        if (! op_type ) {
                inv_op->type = op_unused;
                return(1);
        }
        /* set operand type and value */
        inv_op->access = OP_PERM(op_type);
        inv_op->type = OP_TYPE(op_type) >> 8;
        if (! inv_op->type ) {
                inv_op->type = op_unknown;
        }
        inv_op->flags = OP_MOD(op_type) >> 12;
        inv_op->datatype = OP_SIZE(op_type) >> 24;
        if (! inv_op->datatype ) {
                inv_op->datatype = op_dword;
        }

	return(1);
}

/* invariant instruction representation */
int x86_invariant_insn( unsigned char *buf, int buf_len, x86_invariant_t *inv ){
	struct ARCH_INVARIANT ainv = {{0}, 0};
	int size;

	if (! buf || ! buf_len || ! inv  ) {
		return(0);
	}

	size = disasm_invariant(buf, buf_len, &ainv);

	if ( size ) {
		memcpy(inv->bytes, ainv.buf, 64);
		inv->size = size;

		/* translate i386 types to libdis types */
        	inv->group = INS_GROUP(ainv.insn_type) >> 12;
        	inv->type = INS_TYPE(ainv.insn_type);

		invariant_op_from_optype( ainv.dest_type, &inv->operands[0] );
		invariant_op_from_optype( ainv.src_type, &inv->operands[1] );
		invariant_op_from_optype( ainv.aux_type, &inv->operands[2] );
	}


	return( size );
}


/* OLD API
 * ------------------------------------------------------------------------ */
static enum x86_asm_format assembler_format = native_syntax;
/* From here down are the routines in the old legacy api -- used in
 * libdisasm .16 and under. Obviously big fans of the library will want
 * these to stick around, even though the API sucks like a jet intake */

static void fix_op( qword * op, unsigned int type ) {
        struct addr_exp *e;
        struct EXPRLIST expl;

        if ( OP_TYPE(type) == OP_REG ) {
                *op = (qword) ((long) vm_get_reg_name((int)*op));
        } else if ( OP_TYPE(type) == OP_EXPR ) {
                e = calloc( sizeof(struct addr_exp), 1 );
                addrexp_get((int) *op, &expl);
                e->scale = expl.scale;
                e->index = expl.index;
                e->base = expl.base;
                e->disp = expl.disp;
                e->flags = expl.flags;
                e->used = expl.used;
                *op = (qword) ((long) e);
        }
        return;
}

int x86old_disasm_addr_raw(char *buf, int buf_len, struct code *c){
        int size;
        unsigned char disasm_buf[32] = {0};

        /* copy binary code to temporary buffer */
        memcpy( disasm_buf, buf, ( buf_len > cpu_inst_size() ) ?
                                       cpu_inst_size() : buf_len     );

        /* actually do the disassembly  -- note we do not zero 'c' */
        size = disasm_addr(disasm_buf, 0, c, 0);

        /* check if we overran the end of the buffer */
        if ( size > buf_len ) {
                return(0);
        }

        fix_op( &c->dest, c->dest_type );
        fix_op( &c->src, c->src_type );
        fix_op( &c->aux, c->aux_type );

        return(size);
}

/* format an address expression */
static int fmt_expr_op(long operand, int flags, char *buf, int len)
{
        if (!operand && flags != ADDREXP_REG) {
                buf[0] = '\0';
                return (0);
        }

        switch (flags) {
        case ADDREXP_REG:
                if (assembler_format == ATT_SYNTAX)
                        snprintf(buf, len, "%%%s", vm_get_reg_name(operand));
                else
                        strncpy(buf, vm_get_reg_name(operand), len);
                break;
        case ADDREXP_WORD:
                if (operand)
                        snprintf(buf, len, "%04X", (short) operand);
                break;
        case ADDREXP_DWORD:
                if (operand)
                        snprintf(buf, len, "%08lX", operand);
                break;
        case ADDREXP_QWORD:
                if (operand)
                        snprintf(buf, len, "%012lX", operand);
                break;
        case ADDREXP_BYTE:
        default:
                if (operand)
                        snprintf(buf, len, "%02X", (char) operand);
        }

        return (strlen(buf));
}

int x86_old_sprint_addexp(char *str, int len, struct addr_exp *e)
{
        char scale[32] = { 0 }, index[32] = {0},
                        base[32] = {0}, disp[32] = {0};
        char sd, idx[16] = { 0 }, tmp[32] = {0};

        /* normalize negatives */
        if (e->disp < 0) {
                sd = '-';
                e->disp *= -1;
        } else if ( assembler_format == ATT_SYNTAX) {
                sd = ' ';
        } else {
                sd = '+';
        }

        /* do scale */
        fmt_expr_op(e->scale, AddrExp_ScaleType(e->flags), scale, 32);
        /* do index */
        fmt_expr_op(e->index, AddrExp_IndexType(e->flags), index, 32);
        /* do byte */
        fmt_expr_op(e->base, AddrExp_BaseType(e->flags), base, 32);
        /* do disp */
        fmt_expr_op((long)e->disp, AddrExp_DispType(e->flags), disp, 32);

        str[0] = '\0';

        switch (assembler_format) {
        case ATT_SYNTAX:
                if (disp[0]) {
                        snprintf(str, len - strlen(str), "%c%s", sd, disp);
                }

                if (base[0]) {
                        strncat(tmp, base, 32 - strlen(tmp));
                }
                if (index[0]) {
                        strncat(tmp, ", ", 32 - strlen(tmp));
                        strncat(tmp, index, 32 - strlen(tmp));
                } else if (scale[0]) {
                        strncat(tmp, ",", 32 - strlen(tmp));
                }
                if (scale[0]) {
                        strncat(tmp, ",", 32 - strlen(tmp));
                        strncat(tmp, scale, len - strlen(tmp));
                }
                if (tmp[0]) {
                        strncat(str, "(", len - strlen(str));
                        strncat(str, tmp, len - strlen(str));
                        strncat(str, ")", len - strlen(str));
                }
                break;
        case INTEL_SYNTAX:
        case NATIVE_SYNTAX:
        default:
                if (scale[0] && index[0])
                        snprintf(idx, 16, "(%s*%s)", scale, index);
                else if (index[0])
                        snprintf(idx, 16, "%s", index);

                if (base[0]) {
                	snprintf(str, len, "[%s", base);
                        if (idx[0]) {
                                strncat(str, "+", len - strlen(str));
                                strncat(str, idx, len - strlen(str));
                        }
                        if (disp[0]) {
                                snprintf(tmp, 32, "%c%s", sd, disp);
                                strncat(str, tmp, len - strlen(str));
                        }
                } else if (idx[0]) {
                        snprintf(str, len, "[%s%c%s", idx, sd, disp);
                } else {
                        if ( sd == '-' ) {
				strncat( str, "[-", len - strlen(str) );
			} else {
				strncat( str, "[", len - strlen(str) );
			}
                        strncat(str, disp, len - strlen(str));
                }
                strncat(str, "]", len - strlen(str));

        }
        return (strlen(str));
}


static int sprint_seg(char *str, int len, int seg)
{
        seg = seg >> 16;
        if (assembler_format == ATT_SYNTAX)
                snprintf(str, len, "%%%s:",
                         vm_get_reg_name(ext_arch.reg_seg + seg - 1));
        else
                snprintf(str, len, "%s:",
                         vm_get_reg_name(ext_arch.reg_seg + seg - 1));
        return (strlen(str));
}

static int sprint_op(char *str, int len, qword op, int type)
{
        int diff, seg, iop;

        if (!type) {
                memset(str, 0, len);
                return (0);
        }

        seg = type & OP_SEG_MASK;       /* segment override for operand */
        iop = (int) op;

        switch (type & OP_TYPE_MASK) {
                case OP_PTR:
                case OP_ADDR:
                        if (assembler_format == ATT_SYNTAX && len) {
                                strcat( str, "*" );
                                str++;
                                len--;
                        }
                        snprintf(str, len, "0x%08lX", (long)iop);
                        break;
                case OP_REG:
                        if (assembler_format == ATT_SYNTAX){
                                strncat(str, "%s", len);
                                str++;
                                len--;
                        } else if (seg) {
                                diff = sprint_seg(str, len, seg);
                                str += diff;
                                len -= diff;
                        }
                        snprintf(str, len, "%s", vm_get_reg_name(iop));
                        break;
                case OP_EXPR:
                        if (assembler_format != ATT_SYNTAX && seg) {
                                diff = sprint_seg(str, len, seg);
                                str += diff;
                                len -= diff;
                        }
                        x86_old_sprint_addexp(str, len, (struct addr_exp *)iop);
                        break;
                case OP_REL:
                        if (op < 0) {
                                op *= -1;
                                strncat(str, "-", len);
                        } else {
                                strncat(str, "+", len);
                        }
                        str++;
                        len--;
                        snprintf(str, len, "0x%X", iop);
                        break;
                case OP_OFF:
                        if (assembler_format != ATT_SYNTAX && seg) {
                                diff = sprint_seg(str, len, seg);
                                str += diff;
                                len -= diff;
                        }
                        snprintf(str, len, "0x%08lX", (long)iop);
                        break;
                case OP_IMM:
                default:
                        if (assembler_format == ATT_SYNTAX && len) {
                                strcat( str, "$" );
                                str++;
                                len--;
                        }
                        if (  type & OP_SIGNED ) {
                                if (op < 0) {
                                        strncat(str, "-", len);
                                        len--;
                                        str++;
                                        op *= -1;
                                }
                                snprintf( str, len, "0x%lX", *(long *)&iop );
                        } else {
                                snprintf( str, len, "0x%lX",
                                                *(unsigned long *)&iop );
                        }
                        break;
        }
        return (strlen(str));
}

int x86_old_disasm_addr(char *buf, int buf_len, struct x86_old_instr *i) {
        int size;
        struct code c = { 0 };
        unsigned char disasm_buf[32] = {0};

        /* clear addr_exp */
        memset(i, 0, sizeof (struct x86_old_instr));
        memcpy( disasm_buf, buf, ( buf_len > cpu_inst_size() ) ?
                        cpu_inst_size() : buf_len     );

        size = disasm_addr(disasm_buf, 0, &c, 0);
        if ( size ) {
                fix_op( &c.dest, c.dest_type );
                fix_op( &c.src, c.src_type );
                fix_op( &c.aux, c.aux_type );
                strncpy(i->mnemonic, c.mnemonic, 16);
                sprint_op(i->dest, 32, c.dest, c.dest_type);
                sprint_op(i->src, 32, c.src, c.src_type);
                sprint_op(i->aux, 32, c.aux, c.aux_type);
                i->mnemType = c.mnem_type;
                i->destType = c.dest_type;
                i->srcType = c.src_type;
                i->auxType = c.aux_type;
                i->size = size;
        }
        return (size);
}

int x86_old_sprint_addr(char *str, int len, char *buf, int buf_len) {
        struct x86_old_instr i;
        int size;

        size = x86_old_disasm_addr(buf, buf_len, &i);
        if (! size) {
                snprintf(str, len, "invalid instruction: %02X\n", *buf);
                return(0);
        }
        snprintf(str, len, "%s\t%s", i.mnemonic, i.dest);
        if (i.src[0])
                snprintf(str, len - strlen(str), "%s, %s", str, i.src);
        if (i.aux[0])
                snprintf(str, len - strlen(str), "%s, %s", str, i.aux);
        return (size);
}

int x86_old_init(int options, int format){
        assembler_format = format;
        return( x86_init(options, NULL) );
}



