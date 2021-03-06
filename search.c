#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STRBUFSIZ 256

#if !defined(NDEBUG)
#define DBG(x) x
#define RUN(x)
#else
#define DBG(x)
#define RUN(x) x
#endif

#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif
#define true 1
#define false 0

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef unsigned int bool;

char *usage = "\n\
\n\
search string file\n\
\n\
Assumes that the file was generated by kabi-parser.\n\
\n";

const char *EXPSTR = "EXPORTED";
const char *ARGSTR = "ARG";
const char *RETSTR = "RETURN";
const char *NSTSTR = "NESTED";


struct flocations {
	fpos_t currentpos;
	fpos_t deepestpos;
	fpos_t shallowpos;
	int currentlevel;
	int deepestlevel;
	int shallowlevel;
	int count;
};

static FILE *open_file(char *filename)
{
	FILE *fh = fopen(filename, "r");
	if (fh)
		return fh;
	printf("\nCould not open file: %s\n\n", filename);
	exit(1);
}

static bool check_args(int argc)
{
	if(argc == 3)
		return true;
	puts(usage);
	exit(1);
}

static int get_level(char *line)
{
	int  level;

	if (!(strstr(line, NSTSTR)))
		return 0;

	sscanf(line, " NESTED: %d ", &level);
	return level;
}

static int update_flocations
		(struct flocations *flocs,
		 fpos_t curpos,
		 char *line)
{
	int level = get_level(line);

	if (level > flocs->deepestlevel) {
		flocs->deepestlevel = level;
		flocs->deepestpos = curpos;
	}
	else if (level < flocs->shallowlevel) {
		flocs->shallowlevel = level;
		flocs->shallowpos = curpos;
	}

	return level;
}

// While searching through the file:
//	. count the number of times the object is found
//	. keep fpos on the most deeply nested and least deeply nested
//	  occurrances.
//	. keep fpos of the current occurance
//
int main(int argc, char **argv)
{
	char strbuf[STRBUFSIZ];
	char *filename;
	char *searchstr;
	FILE *fh;
	struct flocations flocs;
	fpos_t curpos;

	memset(&flocs, 0, sizeof(struct flocations));
	check_args(argc);
	searchstr = argv[1];
	filename = argv[2];
	fh = open_file(filename);

	while (!feof(fh)) {
		fgetpos(fh, &curpos);
		fgets(strbuf, STRBUFSIZ-1, fh);
		if (strstr(strbuf, searchstr))
			update_flocations(&flocs, curpos, strbuf);
	}

	fclose(fh);
	return 0;
}
