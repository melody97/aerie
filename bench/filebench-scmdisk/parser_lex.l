/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * Portions Copyright 2008 Denis Cheng
 */

%option nounput

%{

#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "filebench.h"
#include "parsertypes.h"
#include "utils.h"
#include "parser_gram.h"

int lex_lineno = 1;		/* line-number for error reporting */
extern void yyerror(char *s);
extern int dofile;			/* are we processing a file? */
%}

%s WHITESTRINGSTATE

%a 50000
%p 50000
%o 50000
%n 5000

%%

\n			{ lex_lineno++; }

<INITIAL>[ \t]+			;

<INITIAL>#.*			;

create                  { return FSC_CREATE; }
define			{ return FSC_DEFINE; }
debug                   { return FSC_DEBUG; }
domultisync		{ return FSC_DOMULTISYNC; }
echo                    { return FSC_ECHO; }
enable			{ return FSC_ENABLE; }
eventgen                { return FSC_EVENTGEN; }
exit		        { return FSC_QUIT; }
foreach                 { return FSC_FOREACH; }
flowop		        { return FSC_FLOWOP; }
fscheck		        { return FSC_FSCHECK; }
fsflush		        { return FSC_FSFLUSH; }
help		        { return FSC_HELP; }
list		        { return FSC_LIST; }
load                    { return FSC_LOAD; }
log		        { return FSC_LOG; }
nousestats		{ return FSC_NOUSESTATS; }
run                     { return FSC_RUN; }
set                     { return FSC_SET; }
shutdown                { return FSC_SHUTDOWN; }
sleep                   { return FSC_SLEEP; }
stats                   { return FSC_STATS; }
system                  { return FSC_SYSTEM; }
usage                   { return FSC_USAGE; }
vars                    { return FSC_VARS; }
version                 { return FSC_VERSION; }
enable_osprof		{ return FSC_OSPROF_ENABLE; }
disable_osprof		{ return FSC_OSPROF_DISABLE; }
warmup                  { return FSC_WARMUP; }
quit		        { return FSC_QUIT; }

file[s]*       	        { return FSE_FILE; }
fileset[s]*             { return FSE_FILESET; }
posset[s]*              { return FSE_POSSET; }
directory               { return FSE_DIRECTORY; }
command                 { return FSE_COMMAND; }
process[es]*	        { return FSE_PROC; }
thread		        { return FSE_THREAD; }
randvar		        { return FSE_RAND; }
clear                   { return FSE_CLEAR; }
snap                    { return FSE_SNAP; }
dump                    { return FSE_DUMP; }
xmldump                 { return FSE_XMLDUMP; }
multidump               { return FSE_MULTIDUMP; }
all                     { return FSE_ALL; }
mode                    { return FSE_MODE; }
multi			{ return FSE_MULTI; }

alldone                 { return FSA_ALLDONE; }
blocking                { return FSA_BLOCKING; }
cached                  { return FSA_CACHED; }
client			{ return FSA_CLIENT; }
dirwidth                { return FSA_DIRWIDTH; }
dirdepthrv              { return FSA_DIRDEPTHRV; }
directio                { return FSA_DIRECTIO; }
dirgamma                { return FSA_DIRGAMMA; }
dsync                   { return FSA_DSYNC;  }
entries                 { return FSA_ENTRIES;}
fd                      { return FSA_FD; }
filename                { return FSA_FILE; }
filesetname             { return FSA_FILE; }
possetname             	{ return FSA_POSSET; }
filesize                { return FSA_SIZE; }
filesizegamma           { return FSA_FILESIZEGAMMA; }
firstdone               { return FSA_FIRSTDONE; }
fstype		        { return FSA_FSTYPE; }
gamma                   { return FSA_RANDGAMMA; }
highwater               { return FSA_HIGHWATER; }
indexed                 { return FSA_INDEXED; }
instances               { return FSA_INSTANCES;}                  
iosize                  { return FSA_IOSIZE; }
iters                   { return FSA_ITERS;}
leafdirs                { return FSA_LEAFDIRS;}
master			{ return FSA_MASTER; }
mean                    { return FSA_RANDMEAN; }
memsize                 { return FSA_MEMSIZE; }
ioprio                  { return FSA_IOPRIO; }
min                     { return FSA_RANDMIN; }
max                     { return FSA_RANDMAX; }
name                    { return FSA_NAME;}
namelength              { return FSA_NAMELENGTH; }
nice                    { return FSA_NICE;}
opennext                { return FSA_ROTATEFD; }
paralloc                { return FSA_PARALLOC; }
path                    { return FSA_PATH; }
prealloc                { return FSA_PREALLOC; }
procname                { return FSA_PROCESS; }
random                  { return FSA_RANDOM;}
randsrc			{ return FSA_RANDSRC; }
randtable		{ return FSA_RANDTABLE; }
rate                    { return FSA_RATE;}
readonly		{ return FSA_READONLY; }
writeonly		{ return FSA_WRITEONLY; }
reuse                   { return FSA_REUSE; }
round			{ return FSA_RANDROUND; }
seed			{ return FSA_RANDSEED; }
size                    { return FSA_SIZE; }
srcfd                   { return FSA_SRCFD; }
target                  { return FSA_TARGET;}
timeout                 { return FSA_TIMEOUT; }
trusttree		{ return FSA_TRUSTTREE; }
type			{ return FSA_TYPE; }
useism                  { return FSA_USEISM;}
value                   { return FSA_VALUE;}
workingset              { return FSA_WSS; }

uniform                 { return FSV_RANDUNI; }
tabular			{ return FSV_RANDTAB; }
"."type			{ return FSS_TYPE; }
"."seed			{ return FSS_SEED; }
"."gamma		{ return FSS_GAMMA; }
"."mean			{ return FSS_MEAN; }
"."min			{ return FSS_MIN; }
"."round		{ return FSS_ROUND; }
"."randsrc		{ return FSS_SRC; }
urandom			{ return FSV_URAND; }
rand48			{ return FSV_RAND48; }
noreadahead             { return FSA_NOREADAHEAD; }


<INITIAL>\"			{ 
                                BEGIN WHITESTRINGSTATE;
                                return FSK_QUOTE;
                        }

<WHITESTRINGSTATE>\"    {
                                BEGIN INITIAL;
                                return FSK_QUOTE;
                        }

<WHITESTRINGSTATE>[^$^\\^"][^$^"]*[^\\^$^"] {
				if ((yylval.sval = strdup(yytext)) == NULL) {
					yyerror("Out of memory");
					filebench_shutdown(1);
				}
 				return FSV_WHITESTRING;
	       	}

<WHITESTRINGSTATE>\\n	{
				yylval.sval = "\n";
 				return FSV_WHITESTRING;
	       	}


<WHITESTRINGSTATE>\\$[^"^$^\\]+	{
				if ((yylval.sval = strdup(yytext + 1)) == NULL) {
					yyerror("Out of memory");
					filebench_shutdown(1);
				}
 				return FSV_WHITESTRING;
	       	}

<WHITESTRINGSTATE>[^$^\\^"] {
				if ((yylval.sval = strdup(yytext)) == NULL) {
					yyerror("Out of memory");
					filebench_shutdown(1);
				}
 				return FSV_WHITESTRING;
	       	}


<INITIAL>\{			{ return FSK_OPENLST; }
<INITIAL>\}			{ return FSK_CLOSELST; }
<INITIAL>=			{ return FSK_ASSIGN; }
<INITIAL>\,			{ return FSK_SEPLST; }
<INITIAL>in                     { return FSK_IN; }
<INITIAL>\+                     { return FSK_PLUS; }
<INITIAL>\-                     { return FSK_MINUS; }
<INITIAL>\*                     { return FSK_MULTIPLY; }
<INITIAL>\/                     { return FSK_DIVIDE; }

<INITIAL>[0-9]+	{
                                errno = 0;
				yylval.ival = strtoll(yytext, NULL, 10);
				if (errno == EINVAL || errno == ERANGE) {
                                        (void) filebench_log(LOG_ERROR, 
						"Invalid I value '%s':%s", yytext,
						strerror(errno));
				}
                                return FSV_VAL_INT;
}

<INITIAL>[0-9]+k	{
                                errno = 0;
				yylval.ival = KB * strtoll(yytext, NULL, 10);
				if (errno == EINVAL || errno == ERANGE) {
                                        (void) filebench_log(LOG_ERROR, 
						"Invalid I value '%s':%s", yytext,
						strerror(errno));
				}
                                return FSV_VAL_INT;
}

<INITIAL>[0-9]+m	{
                                errno = 0;
				yylval.ival = MB * strtoll(yytext, NULL, 10);
				if (errno == EINVAL || errno == ERANGE) {
                                        (void) filebench_log(LOG_ERROR, 
						"Invalid I value '%s':%s", yytext,
						strerror(errno));
				}
                                return FSV_VAL_INT;
}

<INITIAL>[0-9]+g	{
                                errno = 0;
				yylval.ival = GB * strtoll(yytext, NULL, 10);
				if (errno == EINVAL || errno == ERANGE) {
                                        (void) filebench_log(LOG_ERROR, 
						"Invalid I value '%s':%s", yytext,
						strerror(errno));
				}
                                return FSV_VAL_INT;
}

<INITIAL>true	{
				yylval.bval = TRUE;
				return FSV_VAL_BOOLEAN;
		}

<INITIAL>false	{
				yylval.bval = FALSE;
				return FSV_VAL_BOOLEAN;
		}

$[({A-Za-z][A-Za-z0-9_]*[A-Za-z0-9][)}]*	{
				if ((yylval.sval = strdup(yytext)) == NULL) {
					yyerror("Out of memory");
					filebench_shutdown(1);
				}

 				return FSV_VARIABLE;
			}


$[({A-Za-z][A-Za-z0-9_]*"."[A-Za-z0-9][)}]*	{
				int backtrack;

				if ((backtrack =
				    var_is_set4_randvar(yytext)) != 0)
					yyless(yyleng - backtrack);

				if ((yylval.sval = strdup(yytext)) == NULL) {
					yyerror("Out of memory");
					filebench_shutdown(1);
				}

 				return FSV_RANDVAR;
			}


<INITIAL>[/A-Za-z-][/A-Za-z0-9._-]*	{
				if ((yylval.sval = strdup(yytext)) == NULL) {
					yyerror("Out of memory");
					filebench_shutdown(1);
				}
 				return FSV_STRING;
			}


.			{
				yyerror("Illegal character");
			}

%%

void
yyerror(char *s)
{
	if (dofile == DOFILE_TRUE) {
		if (yytext[0] == '\0') {
			filebench_log(LOG_ERROR, 
				      "%s, token expected",
				      s);
			return;
		}
		(void) filebench_log(LOG_ERROR, 
				     "%s at '%s'", 
				     s,
				     yytext);
	} else {
		if (yytext[0] == '\0') {
			(void) filebench_log(LOG_ERROR, 
					     "%s, token expected", s);
			return;
		}
		(void) filebench_log(LOG_ERROR, "%s at '%s'", s, yytext);
	}
}

struct yy_buffer_state *parent;
struct yy_buffer_state *script;

int
yy_switchfileparent(FILE *file)
{
	script = YY_CURRENT_BUFFER;
	parent = (struct yy_buffer_state *)yy_create_buffer(yyin, 128);
	yy_switch_to_buffer(parent);
	return (0);
}

int
yy_switchfilescript(FILE *file)
{
	yy_switch_to_buffer(script);
	return (0);
}

