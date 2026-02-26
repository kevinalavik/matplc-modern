#!/usr/bin/perl -w

# This program turns simple instruction list into C, which can then be
# compiled and executed as a module in the MatPLC.

# This is an interim version of this program; it will be re-implemented in
# C at some stage (or more likely flex/lex, bison/yacc and C).

# When editing, make sure you edit il2c.pl (the `source' code).



# (c) 2000 Jiri Baum
#
# Offered to the public under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# This code is made available on the understanding that it will not be used
# in safety-critical situations without a full and competent review.



#here come the recognised stepladder commands
%cmds = (
	LD => 'push; acc = master ? plc_get($pt) : 0;',
	LDI => 'push; acc = master ? !plc_get($pt) : 0;',

	#constant (note: this identifier is special below)
	K => 'push; acc = ${pt}UL;',

	AND => 'acc = acc && plc_get($pt);',
	ANI => 'acc = acc && !plc_get($pt);',

	OR => 'acc = acc || master && plc_get($pt);',
	ORI => 'acc = acc || master && !plc_get($pt);',

	'LT' => 'acc = acc < plc_get($pt);',
	'LE' => 'acc = acc <= plc_get($pt);',
	'GT' => 'acc = acc > plc_get($pt);',
	'GE' => 'acc = acc >= plc_get($pt);',

	OUT => 'plc_set($pt,acc);',
	OUTI => 'plc_set($pt,!acc);',

	SET => 'if (acc) plc_set($pt,1);',
	RST => 'if (acc) plc_set($pt,0);',

	#master control (start and end)
	MCS => 'master = acc;',
	MCE => 'master = 1;',

	#and block, or block
	ANB => 'pop; acc = acc && next;',
	ORB => 'pop; acc = acc || next;',

	#comparison block
	LTB => 'pop; acc = acc < next;',
	LEB => 'pop; acc = acc <= next;',
	GTB => 'pop; acc = acc > next;',
	GEB => 'pop; acc = acc >= next;',
	
	#end of program flow/sub (unconditional, but a JMP can jump over this)
	'END' => 'return;',

	#subroutines (note: these two identifiers are special below)
	'SUB' => '} static void IL_sub_$pt(void) {',
	'JSR' => 'if (acc) {pop; IL_sub_$pt();}',

	#conditional end of program flow/sub
	'RET' => 'if (acc) {pop; return;}',

	#jumps (note: these two identifiers are special to the code below)
	JMP => 'if (acc) {pop; goto label_$pt;}',
	LBL => 'label_$pt:',

	#pop the stack (for completeness)
	POP => 'pop;',

	#we have to have this, right?
	NOP => '',
);

#leave this many blank lines before/after these instructions
%blankbefore = (LD=>1,LDI=>1,K=>1,ANB=>1,ORB=>1,
                LEB=>1,LTB=>1,GEB=>1,GTB=>1,LBL=>1,'SUB'=>2);
%blankafter = (LBL=>1,'END'=>2,'SUB'=>1);

$subdecl = qq(# 1 "$0:subroutine_declaration"\n);

# the heart of the translation, using the above table.
while (<>) {
	#check if this is the first line in a new file
	if ($.==1) {
		$module = $ARGV if not defined $module;
		$code .= <<end_header
/************************************** $ARGV */
# 1 "$ARGV"
end_header
	}

	chomp; s/^\s+//; s/\s+$//; #get rid of leading and trailing blanks

        ####################
	# switch statement #
	####################

	#normal command (translate)
	($key,$pt)=/^(\w+)\s*(\w*)$/
	and do {
		$key = uc $key;

		exists $cmds{$key} or die qq($ARGV:$.: Bad keyword "$key"\n);

                if ($key eq "SUB") {
		  $subdecl .= "static void IL_sub_$pt(void);\n";
		} elsif ($key eq "K") {
		  $pt =~ s/^on$/1/i; $pt =~ s/^off$/0/i;
		} elsif (($pt ne "") and ($key ne "LBL") and ($key ne "JMP")
		         and $key ne "JSR") {
		  $points{$pt} = 1; #mark the point as used
		}

		$code .= "\n"x$blankbefore{$key} if $blankbefore{$key};
		$code .= qq(# $. "$ARGV"\n); # output a #line directive
		$cmd=eval qq("	$cmds{$key}"); # translate the command
		if ($key eq "SUB") {
		  $cmd =~ s/^	//; # don't indent this one
		}
		$code .= $cmd.(" "x(40-length $cmd))."/* $_ */\n";
		$code .= "\n"x$blankafter{$key} if $blankafter{$key};
	1} or

	#comment (copy to output)
	($pre,$txt)=/^([#;]+)\s*(.*)$/ and do {
		#get rid of trailing comment-marker (if same)
		$txt=~s/\s*$pre$//;
		#get rid of embedded comment-ends
		$txt=~s'/\*'/ *'g; $txt=~s'\*/'* /'g;

		($txt,$pre)=($pre,"#") if $txt eq "";
		$code .= "\t".(" "x39) ."/".("*" x length $pre)
					." $txt "
		.("*" x length $pre)."/\n";
	1} or

	#blank line (ignore)
	not /\S/ and do {1} or

	#default case (error)
	die "$ARGV:$.: Syntax error: $_\n";
	
	##############
	# end switch #
	##############

	# Reset linenumbers at end of each file (sorry about the error msg)
	close($ARGV) if (eof);
}


# now handle the variable declarations - that's why we aren't printing it
# out as we go along - and initialization.
$decl = qq(# 1 "$0:point_handle_declaration"\n);
$init = qq(# 1 "$0:point_handle_initialization"\n);
for $pt (keys %points) {
  $decl .= "static plc_pt_t $pt;\n";
  $init .= qq(  $pt = plc_pt_by_name("$pt");\n);
}

#default module name
$module = "IL" if not defined $module;

#chop trailing .il from module name
$module =~ s/\.il$//;

#now print it out with all the stuff around it
print <<end_template;
# 1 "$0:template1"

/**************************************************************
 * DO NOT EDIT - This file is automatically generated by il2c *
 **************************************************************/

#include <plc.h>

#define stk_depth 8 /* stack depth - eight is normal */

/* a couple of shortcuts */
#define acc stack[top] /* accumulator == top of stack */
#define next stack[(top + 1) % stk_depth] /* one beyond top of stack */
#define push top = (top + 1) % stk_depth
#define pop top = (top + stk_depth - 1) % stk_depth

/* 
 * Should probably use `n&7' instead of `n%8' but never mind for now.
 * Alternatively, switch to using the bits of a single char for the stack
 * and shifts for push and pop.
 */

$subdecl
$decl
# 1 "$0:template2"

static unsigned char top=0; /* top of stack (ie, stack pointer) */
static u32 stack[stk_depth];
static u32 master=1; /* for the MCS instruction */

static void IL_main(void) {
	master=1; /* reset MCS */

$code
/************************************** [end] */
# 1 "$0:template3"

}

int main(int argc,char *argv[])
{
  plc_init("$module",argc,argv);
$init
# 1 "$0:template4"
  while (1) {
    plc_scan_beg();
    plc_update();

    IL_main();

    plc_update();
    plc_scan_end();
  }
}

end_template
#
