#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cerrno>
#include <cctype>

#include <string>
#include <map>

#define DATAFIL "assets.dat"
#define LINESIZ FILENAME_MAX

using namespace std;

void nospaces(char **const str);
char *nospaces(char *str);
char *duplicate(const char *const str, size_t len);

bool match(char **const line, char c);
bool match(char **const line, const char *str);
bool runln(map<string, char *> &data, char *line, FILE *stream);
bool loadf(map<string, char *> &data, char *path);
bool savef(map<string, char *> &data, char *path);

void cmd(map<string, char *> &data, char *line);
void add(map<string, char *> &data, char *line);
void rem(map<string, char *> &data, char *line);
char *get(map<string, char *> &data, char *line);
void log(map<string, char *> &data);

void quit(char *line);
void help();

char *tran(char *const word, map<string, char *> &data);

static const char *const help_message = {
	"\n=-------+ UdATranslator HELP MESSAGE +-------=\n"
	"\nWelcome to UdaTranslator by Federico Cristina."
	"\nTo translate a word of \"Sistemi e Reti\" from"
	"\nenglish to italian, only write it like this:\n"
	"\n  Translate >> WORD TO TRANSLATE\n"
	"\nDon't care about spaces or letter cases,"
	"\nthey're adjusted automatically. Do the same to"
	"\ntranslate from italian to english.\n"
	"\nFull commands list:"
	"\n  :add [key; value]    to add a word to dictionary"
	"\n  :rem key             to remove a word from dictionary"
	"\n  :get key             to get a word translation (only by key)"
	"\n  :log                 to print all words in dictionary"
	"\n  :msg                 to print a info message to console,"
	"\n                       doesn't translate words"
	"\n  :help                to show this message :)"
	"\n  :quit code message   to exit with an error level and a"
	"\n                       custom message (code is numeric)"
	"\n  :load path           to load a file as a script or a"
	"\n                       dictionary file"
	"\n  :save path           to save the content of the current"
	"\n                       dictionary into a new file; it"
	"\n                       overwrites old files"
	"\n  :reld                to reload dictionary, removing all"
	"\n                       entries and reloading default assets\n"
	"\nSome commands can be replaced with a symbolic"
	"\nform:"
	"\n  [key; value]         -> :add [key; value]"
	"\n  %key                 -> :get key"
	"\n  !code message        -> :quit code message"
	"\n  @path                -> :load path\n"
	"\n=---------------------------------------------=\n"
};

int main(int argc, char *argv[])
{
	char line[LINESIZ] = { 0 };
	map<string, char *> data;

	if (argc == 1)
	{
		if (!loadf(data, DATAFIL))
			printf("Error %d: %s.\n", errno, strerror(errno)), abort();
	}
	else
	{
		int argi;

		for (argi = 1; argi < argc; argi++)
		{
			if (!loadf(data, argv[argi]))
				printf("Error %d: %s.\n", errno, strerror(errno)), abort();
		}
	}
	
	printf("UdATranslator, to get help type :help\n");

	do
	{
		printf("\nTranslate >> ");
		
		if (!runln(data, line, stdin))
			printf("%s\n", tran(line, data));
	} while (true);

	return EXIT_SUCCESS;
}

inline char *nospaces(char *str)
{
	size_t len = strlen(str) - 1;

	if (len > 0)
	{
		while (isspace(*str))
			str++, len--;

		while (isspace(str[len]))
			str[len--] = '\0';
	}

	return str;
}

inline void nospaces(char **const str)
{
	while (isspace(**str))
		(*str)++;

	return;
}

inline char *duplicate(const char *const str, size_t len)
{
	char *tmp = (char *)calloc(len + 1, sizeof(char));

	if (!tmp)
		printf("Error: Not enough memory?!\n"), abort();

	return strncpy(tmp, str, len);
}

inline bool match(char **const line, char c)
{
	if (**line == c)
		return (*line)++, true;
	else
		return false;
}

bool match(char **const line, const char *str)
{
	char *tmp;

	for (tmp = *line; *str != '\0'; str++)
	{
		if (*tmp != *str)
			return false;
		else
			tmp++;
	}

	*line = tmp;

	return true;
}

bool runln(map<string, char *> &data, char *line, FILE *stream)
{
	if (!fgets(line, LINESIZ, stream))
		return false;

	line = nospaces(line);

	switch (line[0])
	{
	case '[':
		add(data, line);
		break;

	case ':':
		cmd(data, line + 1);
		break;

	case '%':
		printf("%s\n", get(data, line));
		break;

	case '@':
		loadf(data, line + 1);
		break;

	case '!':
		quit(line + 1);
		break;

	default:
		return false;
	}

	return true;
}

bool loadf(map<string, char *> &data, char *path)
{
	char line[LINESIZ] = { 0 };
	FILE *file = fopen(nospaces(path), "r");

	if (!file)
		return false;

	while (!feof(file))
		runln(data, line, file);

	fclose(file);

	return true;
}

bool savef(map<string, char *> &data, char *path)
{
	FILE *file = fopen(nospaces(path), "w+");
	map<string, char *>::iterator i;
	size_t j;

	if (!file)
		return false;

	for (i = data.begin(), j = 0; i != data.end() && j < data.size(); i++, j++)
		fprintf(file, "[%s; %s]\n", i->first.c_str(), i->second);

	fclose(file);

	return true;
}

void cmd(map<string, char *> &data, char *line)
{
	nospaces(&line);

	if (*line == '\0')
		return;
	else if (match(&line, "add") || *line == '[')
		add(data, line);
	else if (match(&line, "rem"))
		rem(data, line);
	else if (match(&line, "get"))
		printf("%s\n", get(data, line));
	else if (match(&line, "log"))
		log(data);
	else if (match(&line, "msg"))
		printf("%s\n", nospaces(line));
	else if (match(&line, "help"))
		help();
	else if (match(&line, "quit"))
		quit(line);
	else if (match(&line, "load"))
		loadf(data, line);
	else if (match(&line, "save"))
		savef(data, line);
	else if (match(&line, "reld"))
		data.clear(), loadf(data, DATAFIL);
	else
		printf("Error: Uknown command!\n");

	return;
}

void add(map<string, char *> &data, char *line)
{
	long pos;
	char *key;

	nospaces(&line);
	match(&line, '[');

	pos = 0;

	while (line[pos] != ';' && line[pos] != '\0')
		pos++;

	key = nospaces(duplicate(line, pos));
	line += pos;

	if (!match(&line, ';'))
		printf("Error: expected ';' at the end of a word.\n"), abort();

	pos = 0;

	while (line[pos] != ']' && line[pos] != '\0')
		pos++;

	if (line[pos] == ']')
		line[pos] = '\0';

	nospaces(&line);

	if (*line == '%')
		data[key] = get(data, line);
	else
		data[key] = duplicate(line, pos);

	return;
}

inline void rem(map<string, char *> &data, char *line)
{
	data.erase(nospaces(line));

	return;
}

inline char *get(map<string, char *> &data, char *line)
{
	nospaces(&line);
	match(&line, '%');

	if (data.count(line))
		return data.find(line)->second;
	else
		return "<null>";
}

inline void log(map<string, char *> &data)
{
	map<string, char *>::iterator i;
	size_t j;

	for (i = data.begin(), j = 0; i != data.end() && j < data.size(); i++, j++)
		printf("(%02d) [%s; %s]\n", j + 1, (char *)i->first.c_str(), i->second);

	return;
}

void quit(char *line)
{
	int code;

	nospaces(&line);

	if (isdigit(*line))
		code = strtol(line, &line, 10);
	else
		code = 0;

	if (*line == '\0')
		printf("\nPress any key to close. . .");
	else
		printf("\n%s", nospaces(line));

	getchar();
	exit(code);

	return;
}

inline void help()
{
	printf("%s", help_message);

	return;
}

char *tran(char *const word, map<string, char *> &data)
{
	map<string, char *>::iterator i;

	for (i = data.begin(); i != data.end(); i++)
	{
		if (!stricmp(i->first.c_str(), word))
			return i->second;
		else if (!stricmp(i->second, word))
			return (char *)i->first.c_str();
		else
			continue;
	}

	return "Uknown word... Retry!";
}