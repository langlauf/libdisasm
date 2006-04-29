#include <stdio.h>

int main( void ) {
	printf( "ATT: %d\n", strlen("MNEMONIC\tSRC, DEST, IMM") );
        printf( "INTEL : %d\n", strlen("MNEMONIC\tDEST, SRC, IMM" ) );
        printf( "NATIVE : %d\n", strlen("ADDRESS\tBYTES\tMNEMONIC\t"
                                            "DEST\tSRC\tIMM" ) );
        printf( "RAW : %d\n", strlen("ADDRESS|OFFSET|SIZE|BYTES|"
                               "PREFIX|PREFIX_STRING|GROUP|TYPE|NOTES|"
			       "MNEMONIC|CPU|ISA|FLAGS_SET|FLAGS_TESTED|"
			       "STACK_MOD|STACK_MOD_VAL"
			       "[|OP_TYPE|OP_DATATYPE|OP_ACCESS|OP_FLAGS|OP]*"
                                ) );
        printf( "XML : %d\n", strlen(
                                  "<x86_insn>"
                                      "<address rva= offset= size= bytes=/>"
                                      "<prefix type= string=/>"
                                      "<mnemonic group= type= string= "
				      "cpu= isa= note= />"
                                      "<flags type=set>"
                                          "<flag name=>"
                                      "</flags>"
				      "<stack_mod val= >"
                                      "<flags type=tested>"
                                          "<flag name=>"
                                      "</flags>"
                                      "<operand name=>"
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
                                ) );
        return 0;
}
