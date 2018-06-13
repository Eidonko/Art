%{
/****************************************************************************
**
**     File: ariel.l
**
**     Description: scanner of the ariel language
**
**     Language: lex
**
**     Version 3.0a  https://github.com/Eidonko
***************************************************************************/

#include <string.h>
int list[256], card_list;
int lines=1;

%}
IF	([Ii][Ff])
FI	([Ff][Ii])
ELSE	([Ee][Ll][Ss][Ee])
THEN	([Tt][Hh][Ee][Nn])
ELIF	([Ee][Ll][Ii][Ff])
SEND	([Ss][Ee][Nn][Dd])
RECV	([Rr][Ee][Cc][Vv])
CLEAR	([Cc][Ll][Ee][Aa][Rr])
PAUSE	([Pp][Aa][Uu][Ss][Ee])
SET	([Ss][Ee][Tt])
IMALIVE	([Ii][']?[Mm]([ ]+)?[Aa][Ll][Ii][Vv][Ee])
MIA_TO	([Mm][Ii][Aa])[_]?{SEND}[_]?{TIMEOUT}?
TAIA_TO	([Tt][Aa][Ii][Aa])[_]?{SEND}[_]?{TIMEOUT}?
IMALIVE_TO	{IMALIVE}[_]?{CLEAR}[_]?{TIMEOUT}?
MIA_TO_B	([Mm][Ii][Aa])[_]?{RECV}[_]?{TIMEOUT}?
TAIA_TO_B	([Tt][Aa][Ii][Aa])[_]?{RECV}[_]?{TIMEOUT}?
IMALIVE_TO_B	{IMALIVE}[_]?{SET}[_]?{TIMEOUT}?
REQUEST_DB_TO	([Rr][Ee][Qq][Uu][Ee][Ss][Tt])[_]?([Dd][Bb])[_]?{TIMEOUT}?
REPLY_DB_TO	([Rr][Ee][Pp][Ll][Yy])[_]?([Dd][Bb])[_]?{TIMEOUT}?
TEIF_TO		([Tt][Ee][Ii][Ff])[_]?{TIMEOUT}?
MID_TO		([Mm][Ii][Dd])[_]?{TIMEOUT}?
NUMTASKS	([Nn][Uu][Mm]([ ]+)?[Tt][Aa][Ss][Kk][Ss])
Task	([Tt][Aa][Ss][Kk][Ss]?)
Node	([Nn][Oo][Dd][Ee])
TaskId	([Tt][Aa][Ss][Kk][_]?[Ii][Dd][Ss]?)
Is	(([Ii][Ss])|([Aa][Rr][Ee]))
Mbox	([Mm]([Aa][Ii][Ll])?[Bb][Oo][Xx])([Ee][Ss])?
Alias	([Aa][Ll][Ii][Aa][Ss])([Ee][Ss])?
Logical	([Ll][Oo][Gg][Ii][Cc][Aa][Ll])
EndLogical	([Ee][Nn][Dd](([ ]+)?([Ll][Oo][Gg][Ii][Cc][Aa][Ll])?))
DA	([Dd][Aa])
NPROCS	([Nn][Pp][Rr][Oo][Cc][Ss])
KILL	([Kk][Ii][Ll][Ll])|([Ss][Tt][Oo][Pp])
RESTART	([Re][Ee][Ss][Tt][Aa][Rr][Tt])
CALL	[Cc][Aa][Ll][Ll]
INJECT	[Ii][Nn][Jj][Ee][Cc][Tt]
FAULT	[Ff][Aa][Uu][Ll][Tt]
MFAULT	[Mm]([Aa][Nn][Aa][Gg][Ee][Rr])?{FAULT}
BFAULT	[Bb]([Aa][Cc][Kk][Uu][Pp])?{FAULT}
CMP	[Cc][Oo][Mm][Pp][Oo][Nn][Ee][Nn][Tt]
AFTER	[Aa][Ff][Tt][Ee][Rr]
TICKS	[Tt][Ii][Cc][Kk][Ss]
STEST	"["
ETEST	"]"
REMARK	"#"(.)*
COMMA	","
TO	"-"
DOT	"."
SEMICOL	";"
PLUS	[Pp][Ll][Uu][Ss]
MINUS	[Mm][Ii][Nn][Uu][Ss]
MULT	[Tt][Ii][Mm][Ee][Ss]
DIV	[Dd][Ii][Vv]
THREAD	([Tt]([0-9]+)
GROUP	([GgLl]([0-9]+)
NODE	([Nn]([0-9]+)
DIGIT	[0-9]
NUMBER	("-")?[0-9]+
REAL	((({NUMBER})?{DOT})?{NUMBER})
AGENT		([Aa][Gg][Ee][Nn][Tt][Ss]?)
BACKUPAGENT	(([Bb][Aa][Cc][Kk][Uu][Pp][Ss]?){AGENT}?)|([Aa][Ss][Ss][Ii][Ss][Tt][Aa][Nn][Tt][Ss]?)
MANAGER		([Mm][Aa][Nn][Aa][Gg][Ee][Rr])
ROLE		({AGENT}|{BACKUPAGENT}|{MANAGER})
NEWLINE	\n
DEF	[Dd][Ee][Ff][Ii][Nn][Ee]
KILLED		"-"?(([Kk]([Ii][Ll][Ll][Ee][Dd])?)|([Ss][Tt][Oo][Pp][Pp][Ee][Dd]))
RESTARTED	"-"?[Rr]([Ee][Ss][Tt][Aa][Rr][Tt][Ee][Dd])?
PRESENT		"-"?[Pp]([Rr][Ee][Ss][Ee][Nn][Tt])?
ISOLATED	"-"?[Ii]([Ss][Oo][Ll][Aa][Tt][Ee][Dd])?
FAULTY		"-"?[Ff]([Aa][Uu][Ll][Tt][Yy])?
DEADLOCKED	"-"?[Dd]
REINTEGRATED	"-"?[Rr][Ee][Ii]([Nn][Tt][Ee][Gg][Rr][Aa][Tt][Ee][Dd])?
GID		[Gg]{NUMBER}
TID		[Tt]{NUMBER}
NID		[Nn]{NUMBER}
ANYGID		[Gg]"*"
ANYTID		[Tt]"*"
ANYNID		[Nn]"*"
THISGID		[Gg]"$"("$"|{NUMBER})
THISTID		[Tt]"$"("$"|{NUMBER})
THISNID		[Nn]"$"("$"|{NUMBER})
ANY		"*"
AND		("&&"|[Aa][Nn][Dd])
OR		("||"|[Oo][Rr])
NOT		("!"|[Nn][Oo][Tt])
START		[Ss][Tt][Aa][Rr][Tt]
WARN		[Ww][Aa][Rr][Nn]
ON		[Oo][Nn]
ACTION		[Aa][Cc][Tt][Ii][Oo][Nn]
ACT		[Aa][Cc][Tt]([Ii][Oo][Nn])?
ERROR		[Ee][Rr][Rr][Oo][Rr]
EQ		"=="
NEQ		"!="
GT		">"
GE		">="
LT		"<"
LE		"<="
LPAR		"("
RPAR		")"
ERR		[Ee][Rr][Rr]
ERRN		[Ee][Rr][Rr][Nn]
ERRT		[Ee][Rr][Rr][Tt]
REMOVE		[Rr][Ee][Mm][Oo][Vv][Ee]
FROM		[Ff][Rr][Oo][Mm]
ERRORLIST	[Ee][Rr][Rr][Oo][Rr][Ll][Ii][Ss][Tt]
ENABLE		[Ee][Nn][Aa][Bb][Ll][Ee]
Any		[Aa][Nn][Yy]
AT		"@"("@"|{NUMBER})
TILDE		"~"("~"|{NUMBER})
PHASE		[Pp][Hh][Aa][Ss][Ee]
INCLUDE		[Ii][Nn][Cc][Ll][Uu][Dd][Ee]
STRING		(\")[^"]*(\")
ASSOC		(\{)[^}]*(\})
GASSOC		[Gg]{ASSOC}
TASSOC		[Tt]{ASSOC}
NASSOC		[Nn]{ASSOC}
WATCHDOG	[Ww][Aa][Tt][Cc][Hh][Dd][Oo][Gg]
WATCHES		[Ww][Aa][Tt][Cc][Hh][Ee][Ss]
HEARTBEATS	[Hh][Ee][Aa][Rr][Tt][Bb][Ee][Aa][Tt][Ss]
EVERY		[eE][vV][eE][Rr][Yy]
MS		[Mm][Ss]
US		[Uu][Ss]
REBOOT		[Rr][Ee][Bb][Oo][Oo][Tt]
EndWatchdog	([Ee][Nn][Dd](([ ]+)?({WATCHDOG})))
KEYW_VERSION	[Vv][Ee][Rr][Ss][Ii][Oo][Nn]
NVERSION	[Nn]([-])?({KEYW_VERSION})
VOTING		[Vv][Oo][Tt][Ii][Nn][Gg]
ALGORITHM	[Aa][Ll][Gg][Oo][Rr][Ii][Tt][Hh][Mm]
TIMEOUT		[Tt][Ii][Mm][Ee]([-]?)[Oo][Uu][Tt]
MAJORITY	[Mm][Aa][Jj][Oo][Rr][Ii][Tt][Yy]
METRIC		[Mm][Ee][Tt][Rr][Ii][Cc]
SUCCESS		[Ss][Uu][Cc][Cc][Ee][Ss][Ss]
THRESHOLD	[Tt][Hh][Rr][Ee][Ss][Hh][Oo][Ll][Dd]
ALPHACOUNT	[Aa][Ll][Pp][Hh][Aa]([-])?[Cc][Oo][Uu][Nn][Tt]
FACTOR		[Ff][Aa][Cc][Tt][Oo][Rr]
EndAlpha	([Ee][Nn][Dd](([ ]+)?({ALPHACOUNT})?))
EndNVersion	[Ee][Nn][Dd]([ ]+)?{NVERSION}
LET		[Ll][Ee][Tt]
VAL		[Vv][Aa][Ll]
WRITE		[Ww][Rr][Ii][Tt][Ee]
VAR		([A-Z]|[a-z])
LOG		[Ll][Oo][Gg]
CLOCK		[Cc][Ll][Oo][Cc][Kk]
OSTRING		(\')[^']*(\')
FIRED		[Ff][Ii][Rr][Ee][Dd]
%%
{AND}		{
			return AND;
		}
{OR}		{
			return OR;
		}
{NOT}		{
			return NOT;
		}
{PAUSE}		return PAUSE;
{KILLED}	{
			yylval.status = R_KILLED;
			return KILLED;
		}
{RESTARTED}	{
			yylval.status = R_RESTARTED;
			return RESTARTED;
		}
{PRESENT}	{
			yylval.status = R_PRESENT;
			return PRESENT;
		}
{ISOLATED}	{
			yylval.status = R_ISOLATED;
			return ISOLATED;
		}
{FAULTY}	{
			yylval.status = R_FAULTY;
			return FAULTY;
		}
{DEADLOCKED}	{
			yylval.status = R_DEADLOCKED;
			return DEADLOCKED;
		}
{REINTEGRATED}	{
			yylval.status = R_REINTEGRATED;
			return REINTEGRATED;
		}
{DA}		return DA;
{NPROCS}	return NPROCS;
{MIA_TO}	return MIA_TIMEOUT;
{TAIA_TO}	return TAIA_TIMEOUT;
{IMALIVE_TO}	return ALIVE_TIMEOUT;
{MIA_TO_B}	return MIA_TIMEOUT_B;
{TAIA_TO_B}	return TAIA_TIMEOUT_B;
{IMALIVE_TO_B}	return ALIVE_TIMEOUT_B;
{TEIF_TO}	return TEIF_TIMEOUT;
{REQUEST_DB_TO}	return REQUEST_DB_TIMEOUT;
{REPLY_DB_TO}	return REPLY_DB_TIMEOUT;
{MID_TO}	return MID_TIMEOUT;
{KILL}		return KILL;
{RESTART}	return RESTART;
{START}		return START;
{WARN}		return WARN;
{CALL}		return CALL;
{ON}		{
			return ON;
		}
{ACTION}	return ACTION;
{ERROR}		{
			return ERROR;
		}
{DEF}		return DEF;
{REMARK}	;	/* skip comments */
[ \t]*		;	/* skip whitespace */
{NEWLINE}	{
			card_list = 0;
			lines++;
			return '\n';
		}
{AGENT}		{
			strcpy(yylval.string.name, yytext);
			yylval.string.rcode = R_AGENT;
			return ROLE;
		}
{BACKUPAGENT}	{
			strcpy(yylval.string.name, yytext);
			yylval.string.rcode = R_BACKUP;
			return ROLE;
		}
{MANAGER}	{
			strcpy(yylval.string.name, yytext);
			yylval.string.rcode = R_MANAGER;
			return ROLE;
		}
{GID}		{
		yylval.id.role = ID_NORMAL | ID_GROUP;
		sscanf(yytext+1, "%d", &yylval.id.id);
		return GID;
		}
{NID}		{
		yylval.id.role = ID_NORMAL | ID_NODE;
		sscanf(yytext+1, "%d", &yylval.id.id);
		return NID;
		}
{TID}		{
		yylval.id.role = ID_NORMAL | ID_THREAD;
		sscanf(yytext+1, "%d", &yylval.id.id);
		return TID;
		}
{ANYGID}	{
		yylval.id.role = ID_STAR | ID_GROUP;
		yylval.id.id   = 0;
		return GID;
		}
{ANYNID}	{
		yylval.id.role = ID_STAR | ID_NODE;
		yylval.id.id   = 0;
		return NID;
		}
{ANYTID}		{
		yylval.id.role = ID_STAR | ID_THREAD;
		yylval.id.id   = 0;
		return TID;
		}
{ANY}		{
		yylval.id.role = ID_STAR | ID_ENTITY;
		yylval.id.id   = 0;
		return TID;   /* but we return TID as class */
		}
{THISGID}	{
		/* first character is 'G' or 'g' */
		yylval.id.role  = ID_GROUP;

		/* second is obviously '$' */

		/* if third is '$' then this is  G$$ . Note the use of "|=" */
		if (yytext[2] == '$')
			{
			yylval.id.role |= ID_DOLLARS;
			}
		/* else, this is  G${NUMBER} . Note the use of "|=" */
		else
			{
			yylval.id.role |= ID_DOLLAR;
			sscanf(yytext+2, "%d", &yylval.id.id);
			}

		return GID;
		}
{THISNID}	{
		/* first character is 'N' or 'n' */
		yylval.id.role  = ID_NODE;

		/* second is obviously '$' */

		/* if third is '$' then this is  N$$ . Note the use of "|=" */
		if (yytext[2] == '$')
			yylval.id.role |= ID_DOLLARS;
		/* else, this is  N${NUMBER} . Note the use of "|=" */
		else
			{
			yylval.id.role |= ID_DOLLAR;
			sscanf(yytext+2, "%d", &yylval.id.id);
			}

		return NID;
		}
{THISTID}	{
		/* first character is 'T' or 't' */
		yylval.id.role  = ID_THREAD;

		/* second is obviously '$' */

		/* if third is '$' then this is  T$$ . Note the use of "|=" */
		if (yytext[2] == '$')
			yylval.id.role |= ID_DOLLARS;
		/* else, this is  T${NUMBER} . Note the use of "|=" */
		else
			{
			yylval.id.role |= ID_DOLLAR;
			sscanf(yytext+2, "%d", &yylval.id.id);
			}

		return TID;
		}
{NUMBER}	{
		sscanf(yytext, "%d", &yylval.integer);
		return NUMBER;
		}
{IF}		{
		return yylval.integer = IF;
		}
{FI}		{
		return yylval.integer = FI;
		}
{ELSE}		{
		return yylval.integer = ELSE;
		}
{THEN}		{
		return yylval.integer = THEN;
		}
{ELIF}		{
		return yylval.integer = ELIF;
		}
[=,]		{
		return yytext[0];
		}
"["		{
		return '[';
		}
"]"		{
		return ']';
		}
"("		{
		return '(';
		}
")"		{
		return ')';
		}
-		{
		return '-';
		}
{EQ}		{
		yylval.integer = Equal;
		return  EQ;
		}
{NEQ}		{
		yylval.integer = NotEqual;
		return  NEQ;
		}
{GT}		{
		yylval.integer = GreaterThan;
		return  GT;
		}
{GE}		{
		yylval.integer = GreaterThanOrEqualTo;
		return  GE;
		}
{LT}		{
		yylval.integer = LessThan;
		return  LT;
		}
{LE}		{
		yylval.integer = LessThanOrEqualTo;
		return  LE;
		}
{ERRN}		{
			return ERRN;
		}
{ERRT}		{
			return ERRT;
		}
{ACT}		{
			return ACT;
		}
{ERR}		{
			return ERR;
		}
{REMOVE}	{	
			return REMOVE;
		}
{FROM}		{
			return FROM;
		}
{ERRORLIST}	{
			return ERRORLIST;
		}
{ENABLE}	{	
			return ENABLE;
		}
{Any}		return ANY;
{PHASE}		return PHASE;
{NUMTASKS}	return NUMTASKS;
":"		return ':';
";"		return ';';
{AT}		{
		/* if second char is '@' then this is  @@  */
		if (yytext[1] == '@')
			{
			yylval.which = -1;
			}
		/* else, this is  @{NUMBER}  */
		else
			{
			sscanf(yytext+1, "%d", &yylval.which);
			}

		return AT;
		}
{TILDE}		{
		/* if second char is '~' then this is  ~~  */
		if (yytext[1] == '~')
			{
			yylval.which = -1;
			}
		/* else, this is  ~{NUMBER}  */
		else
			{
			sscanf(yytext+1, "%d", &yylval.which);
			}

		return TILDE;
		}
{INCLUDE}	return INCLUDE;
{STRING}	{
			*strchr(yytext+1, '"') = '\0';
			strcpy(yylval.quoted_string, yytext+1);

			return STRING;
		}
{GASSOC}	{
			*strchr(yytext+2, '}') = '\0';
			if (aread(a, yytext+2) != NULL)
			yylval.id.role = ID_NORMAL | ID_GROUP;
			sscanf(aread(a, yytext+2), "%d", &yylval.id.id);
			printf("\tsubstituting %s} with G%d\n", yytext, yylval.id.id);
			return GID;
		}
{TASSOC}	{
			*strchr(yytext+2, '}') = '\0';
			if (aread(a, yytext+2) != NULL)
			yylval.id.role = ID_NORMAL | ID_THREAD;
			sscanf(aread(a, yytext+2), "%d", &yylval.id.id);
			printf("\tsubstituting %s} with T%d\n", yytext, yylval.id.id);
			return TID;
		}
{NASSOC}	{
			*strchr(yytext+2, '}') = '\0';
			if (aread(a, yytext+2) != NULL)
			yylval.id.role = ID_NORMAL | ID_NODE;
			sscanf(aread(a, yytext+2), "%d", &yylval.id.id);
			printf("\tsubstituting %s} with N%d\n", yytext, yylval.id.id);
			return NID;
		}
{ASSOC}		{
			*strchr(yytext+1, '}') = '\0';
			if (aread(a, yytext+1) != NULL)
			yylval.integer = atoi(aread(a, yytext+1));
			printf("\tsubstituting %s} with %d\n", yytext, yylval.integer);
			return NUMBER;
		}
{INJECT}	return INJECT;
{BFAULT}	return BFAULT;
{MFAULT}	return MFAULT;
{CMP}		return COMPONENT;
{AFTER}		return AFTER;
{TICKS}		return TICKS;
{Task}		return KEYW_TASK;
{Node}		return KEYW_NODE;
{Is}		return KEYW_IS;
{TaskId}	return KEYW_TASKID;
{Mbox}		return KEYW_MBOX;
{Alias}		return KEYW_ALIAS;
{SEND}		return SEND;
{Logical}	return KEYW_LOGICAL;
{EndLogical}	return KEYW_ENDLOGICAL;
{WATCHDOG}	{
			return WATCHDOG;
		}
{WATCHES}	{
			return WATCHES;
		}
{HEARTBEATS}	{
			return HEARTBEATS;
		}
{EVERY}		{
			return EVERY;
		}
{MS}		{
			return MILLISEC;
		}
{US}		{
			return MICROSEC;
		}
{REBOOT}	{
			return REBOOT;
		}
{EndWatchdog}	{
			return KEYW_ENDWATCHDOG;
		}
{NVERSION}	{
			return NVERSION;
		}
{KEYW_VERSION}	{
			return KEYW_VERSION;
		}
{TIMEOUT}	{
			return TIMEOUT;
		}
{VOTING}	{
			return VOTING;
		}
{MAJORITY}	{
			return MAJORITY;
		}
{ALGORITHM}	{
			return ALGORITHM;
		}
{METRIC}	{
			return METRIC;
		}
{SUCCESS}	{
			return SUCCESS;
		}
{EndNVersion}	{
			return KEYW_ENDNVERSION;
		}
{THRESHOLD}	{
			return THRESHOLD;
		}
{ALPHACOUNT}	{
			return ALPHACOUNT;
		}
{FACTOR}	{
			return FACTOR;
		}
{REAL}		{
			sscanf(yytext, "%f", &yylval.real);
			return REAL;
		}
{EndAlpha}	return	KEYW_ENDALPHA;
{VAL}		return	VAL;
{LET}		return	LET;
{WRITE}		return	WRITE;
{VAR}		{
		yylval.integer = tolower(*yytext) - 'a';
		return VAR;
		}
{PLUS}		return '+';
{MINUS}		return '-';
{MULT}		return '*';
{DIV}		return '/';
{LOG}		{
		return LOG;
		}
{CLOCK}		{
		return CLOCK;
		}
{OSTRING}	{
		*strchr(yytext+1, '"') = '\0';
		strcpy(yylval.quoted_string, yytext+1);
		return OSTRING;
		}
{FIRED}		{
			yylval.status = R_FIRED;
			return FIRED;
		}
{CLEAR}		return CLEAR;
.		printf("unrecognized char: %s\n", yytext);
%%
/* EOF ariel.l */
