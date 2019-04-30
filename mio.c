/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h> // iscntrl()
#include <errno.h> // errno, EAGAIN
#include <fcntl.h> // open(), O_RDWR, O_CREAT
#include <stdio.h> // printf(), perror(), sscanf(), snprintf(), FILE, fopen(), getline(), vsnprintf()
#include <stdarg.h> // va_list, va_start(), va_end()
#include <stdlib.h> // atexit(), exit(), realloc(), free(), malloc()
#include <string.h> // memcpy(), strlen(), strdup(), memmmove(), strerror(), strstr(), memset(), strchr(), strrchr(), strcmp(), strncmp()
#include <sys/ioctl.h> // ioctl(), TIOCGWINSZ, struct winsize
#include <sys/types.h> // ssize_t
#include <termios.h> // tcgetattr(), tcsetattr()
#include <time.h> // time_t, time()
#include <unistd.h> // write(), STDOUT_FILENO, ftruncate(), close();
#include <inttypes.h> // strtoumax()

#include "config.h"
#include "data.h"
#include "syntax.h"

/*** defines ***/

#define VERSION "0.0.1"

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
	BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

enum editorHighlight {
	HL_NORMAL = 0,
	HL_COMMENT,
	HL_MLCOMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH
};

/*** data ***/

typedef struct editorRow {
    int index;
    int size;
    int renderSize;
    char *chars;
    char *render;
    unsigned char *highlight;
    int highlight_open_comment;
} editorRow;

struct editorConfig {
    int cx;
    int cy;
    int rx; // render index, could be greater than cx due to tabs
    int rowOffset;
    int colOffset;
    int screenRows;
    int screenCols;
    int numRows;
    editorRow *row;
    int dirty;
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct editorSyntax *syntax;
    struct termios orig_termios;
};

struct editorConfig E;

/*** prototypes ***/

void editorSetStatusMessage(const char* fmt, ...);
void editorRefreshScreen();
char *editorPrompt(char *prompt, void (*callback)(char *, int));

/*** terminal  ***/

void die(const char *s) {
    // Clear the screen on exit
    write(STDOUT_FILENO, "\x1b[2j", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

// Reset the terminal's attributes on quit
void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
	    die("tcsetattr");
	}
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
	    die ("tcgetattr");
	}
	atexit(disableRawMode);

	struct termios raw =  E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN  | ISIG);
	raw.c_cc[VMIN] = 0;  // min # of bytes needed before read() returns
	raw.c_cc[VTIME] = 1; // max about of time for read() to wait before returning

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
	    die("tcsetattr");
	}
}

// Waits for a keypress and returns it
int editorReadKey() {
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
    }

    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return '\x1b';
        }

        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return '\x1b';
        }

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) {
                    return '\x1b';
                }

                if (seq[2] == '~') {
                    switch(seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch(seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\x1b';
    } else {
        return c;
    }
}

int getCursorPosition(int *rows, int *cols) {

    char buf[32];
    unsigned int i = 0;

    // n queries the terminal for status information
    // 6 gives cursor position
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
        return -1;
    }

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) {
            break;
        }
        if (buf[i] == 'R') {
            break;
        }
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        return -1;
    }

    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return -1;
    }

    return 0;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    // Use ioctl or manually determine window size
    // C means Cursor Forwards (moves us to the right)
    // B means Cursor Down (moves us down)
    // 999 ensures we go all the way right and down
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return getCursorPosition(rows, cols);
        }
        editorReadKey();
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** syntax highlighting ***/

int is_separator(int c) {
	return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorUpdateSyntax(editorRow *row) {
	row->highlight = realloc(row->highlight, row->renderSize);
	memset(row->highlight, HL_NORMAL, row->renderSize);

	if (E.syntax == NULL) {
		return;
	}

	char **keywords = E.syntax->keywords;

	char *single_comments = E.syntax->singleline_comment_start;
	char *multi_comment_start = E.syntax->multiline_comment_start;
	char *multi_comment_end = E.syntax->multiline_comment_end;

	int single_comments_length = single_comments ? strlen(single_comments) : 0;
	int multi_comment_start_length = multi_comment_start ? strlen(multi_comment_start) : 0;
	int multi_comment_end_length = multi_comment_end ? strlen(multi_comment_end) : 0;

	int prev_separator = 1;
	int in_string = 0;
	int in_comment = (row->index > 0 && E.row[row->index - 1].highlight_open_comment);

	int i = 0;
	//for (i = 0; i < row->renderSize; i++) {
	while(i < row->renderSize) {
		char c = row->render[i];
		unsigned char prev_highlight = (i > 0) ? row->highlight[i - 1] : HL_NORMAL;

		if (single_comments_length && !in_string && !in_comment) {
			if (!strncmp(&row->render[i], single_comments, single_comments_length)) {
				memset(&row->highlight[i], HL_COMMENT, row->renderSize - i);
				break;
			}
		}

		if (multi_comment_start_length && multi_comment_end_length && !in_string) {
			if (in_comment) {
				row->highlight[i] = HL_MLCOMMENT;
				if (!strncmp(&row->render[i], multi_comment_end, multi_comment_end_length)) {
					memset(&row->highlight[i], HL_MLCOMMENT, multi_comment_end_length);
					i += multi_comment_end_length;
					in_comment = 0;
					prev_separator = 1;
					continue;
				} else {
					i++;
					continue;
				}
			} else if (!strncmp(&row->render[i], multi_comment_start, multi_comment_start_length)) {
				memset(&row->highlight[i], HL_MLCOMMENT, multi_comment_start_length);
				i += multi_comment_start_length;
				in_comment = 1;
				continue;
			}
		}

		if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
			if (in_string) {
				row->highlight[i] = HL_STRING;
				if (c == '\\' && i + 1 < row->renderSize) {
					row->highlight[i + 1] = HL_STRING;
					i += 2;
					continue;
				}
				if (c == in_string) {
					in_string = 0;
				}
				i++;
				prev_separator = 1;
				continue;
			} else {
				if (c == '"' || c == '\'') {
					in_string = c;
					row->highlight[i] = HL_STRING;
					i++;
					continue;
				}
			}
		}

		if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
			if ((isdigit(c) && (prev_separator || prev_highlight == HL_NUMBER)) || (c == '.' && prev_highlight == HL_NUMBER)) {
				row->highlight[i] = HL_NUMBER;
				i++;
				prev_separator = 0;
				continue;
			}
		}

		if (prev_separator) {
			int j;
			for (j = 0; keywords[j]; j++) {
				int keyLen = strlen(keywords[j]);
				int kw2 = keywords[j][keyLen - 1] == '|';
				if (kw2) {
					keyLen--;
				}

				if (!strncmp(&row->render[i], keywords[j], keyLen) && is_separator(row->render[i+keyLen])) {
					memset(&row->highlight[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, keyLen);
					i += keyLen;
					break;
				}
			}
			if (keywords[j] != NULL) {
				prev_separator = 0;
				continue;
			}
		}

		prev_separator = is_separator(c);
		i++;
	}

	int changed = (row->highlight_open_comment != in_comment);
	row->highlight_open_comment = in_comment;
	if (changed && row->index + 1 < E.numRows) {
		editorUpdateSyntax(&E.row[row->index + 1]);
	}
}

int editorSyntaxToColor(int highlight) {
	switch (highlight) {
		case HL_COMMENT:
		case HL_MLCOMMENT:
			return 36;

		case HL_KEYWORD1:
			return 33;

		case HL_KEYWORD2:
			return 32;

		case HL_STRING:
			return 35;

		case HL_NUMBER:
			return 31;

		case HL_MATCH:
			return 34;

		default:
			return 37;
	}
}

void editorSelectSyntaxHighlight() {
	E.syntax = NULL;

	if (E.filename == NULL) {
		return;
	}

	char *extension = strrchr(E.filename, '.');

	for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
		struct editorSyntax *s = &HLDB[j];
		unsigned int i = 0;
		while(s->filematch[i]) {
			int is_extension = (s->filematch[i][0] == '.');
			if ((is_extension && extension && !strcmp(extension, s->filematch[i])) || (!is_extension && strstr(E.filename, s->filematch[i]))) {
				E.syntax = s;

				int fileRow;
				for (fileRow = 0; fileRow < E.numRows; fileRow++) {
					editorUpdateSyntax(&E.row[fileRow]);
				}

				return;
			}
			i++;
		}
	}
}

/*** row operations  ***/

// Convers the char index to a render index
int editorRowCxToRx(editorRow *row, int cx) {
    int rx = 0;
    int j = 0;
    for (j = 0; j < cx; j++) {
        if (row->chars[j] == '\t') {
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        }
        rx++;
    }
    return rx;
}

int editorRowRxToCx(editorRow *row, int rx) {
	int cur_rx = 0;
	int cx;

	for (cx = 0; cx < row->size; cx++) {
		if (row->chars[cx] == '\t') {
			cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
		}
		cur_rx++;

		if (cur_rx > rx) {
			return cx;
		}
	}
	return cx;
}

void editorUpdateRow(editorRow *row) {
    int j;
    int tabs = 0;
    // Count tabs
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            tabs++;
        }
    }

    free(row->render);
    row->render = malloc(row->size + tabs*(TAB_STOP - 1)  + 1);

    int index = 0;
    for(j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[index++] = ' ';
            while (index % TAB_STOP != 0) {
                row->render[index++] = ' ';
            }
        } else {
            row->render[index++] = row->chars[j];
        }
    }
    row->render[index] = '\0';
    row->renderSize = index;

    editorUpdateSyntax(row);
}

void editorInsertRow(int at, char *s, size_t len) {

	if (at < 0 || at > E.numRows) {
		return;
	}

    E.row = realloc(E.row, sizeof(editorRow) * (E.numRows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(editorRow) * (E.numRows - at));
	for (int j = at + 1; j <= E.numRows; j++) {
		E.row[j].index++;
	}

    E.row[at].index = at;

    //int at = E.numRows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].renderSize = 0;
    E.row[at].render = NULL;
    E.row[at].highlight = NULL;
    E.row[at].highlight_open_comment = 0;
    editorUpdateRow(&E.row[at]);

    E.numRows++;
    E.dirty++;
}

void editorFreeRow(editorRow *row) {
	free(row->render);
	free(row->chars);
	free(row->highlight);
}

void editorDeleteRow(int at) {
	if (at < 0 || at >= E.numRows) {
		return;
	}
	editorFreeRow(&E.row[at]);
	memmove(&E.row[at], &E.row[at + 1], sizeof(editorRow) * (E.numRows - at - 1));
	for (int j = at; j < E.numRows - 1; j++) {
		E.row[j].index--;
	}
	E.numRows--;
	E.dirty++;
}

void editorRowInsertChar(editorRow *row, int at, int c) {
	// Make sure our position is valid
	if (at < 0 || at > row->size) {
		at = row->size;
	}

	// Allocate one more byte in our row
	row->chars = realloc(row->chars, row->size + 2);
	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
	row->size++;
	row->chars[at] = c;
	editorUpdateRow(row);
	E.dirty++;
}

void editorRowAppendString(editorRow *row, char *s, size_t len) {
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len;
	row->chars[row->size] = '\0';
	editorUpdateRow(row);
	E.dirty++;
}

void editorRowDeleteChar(editorRow *row, int at) {
	if (at < 0 || at >= row->size) {
		return;
	}

	memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
	row->size--;
	editorUpdateRow(row);
	E.dirty++;
}

/*** editor operations ***/

void editorInsertChar(int c) {
	// Checks if we're on the tilde line
	if (E.cy == E.numRows) {
		editorInsertRow(E.numRows, "", 0);
	}

	// Insert character and advance the cursor
	editorRowInsertChar(&E.row[E.cy], E.cx, c);
	E.cx++;
}

void editorInsertNewline() {
	if (E.cx == 0) {
		editorInsertRow(E.cy, "", 0);
	} else {
		editorRow *row = &E.row[E.cy];
		editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
		row = &E.row[E.cy];
		row->size = E.cx;
		row->chars[row->size] = '\0';
		editorUpdateRow(row);
	}
	E.cy++;
	E.cx = 0;
}

void editorDeleteChar() {
	// Can't delete past the end of the file
	if (E.cy == E.numRows) {
		return;
	}

	if (E.cx == 0 && E.cy == 0) {
		return;
	}

	editorRow *row = &E.row[E.cy];
	if (E.cx > 0) {
		editorRowDeleteChar(row, E.cx - 1);
		E.cx--;
	} else {
		E.cx = E.row[E.cy - 1].size;
		editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
		editorDeleteRow(E.cy);
		E.cy--;
	}
}

/*** file i/o  ***/

// Converts all of the editorRows to a string
char *editorRowsToString(int *bufLen) {

	// Add up the lengths of our rows
	int totalLength = 0;
	int j;
	for (j = 0; j < E.numRows; j++) {
		totalLength += E.row[j].size + 1;
	}
	*bufLen = totalLength;

	// Allocate a buffer and copy our rows
	char *buf = malloc(totalLength);
	char *p = buf;
	for (j = 0; j < E.numRows; j++) {
		memcpy(p, E.row[j].chars, E.row[j].size);
		p += E.row[j].size;
		*p = '\n';
		p++;
	}

	return buf;
}

void editorKillCurrentBuffer() {
	E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowOffset = 0;
    E.colOffset = 0;
    E.numRows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;
}

void editorOpen(char *filename) {
    free(E.filename);
    E.filename = strdup(filename);

    editorSelectSyntaxHighlight();

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        die("fopen");
    }

    char *line = NULL;
    size_t lineCap = 0; // line capacity
    ssize_t lineLen;
    while ((lineLen = getline(&line, &lineCap, fp)) != -1) {
        while (lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) {
            lineLen--;
        }
        editorInsertRow(E.numRows, line, lineLen);
    }

    free(line);
    fclose(fp);
    E.dirty = 0;
}

void editorSave() {
	// Check if this is a new file
	if (E.filename == NULL) {
		E.filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);
		if (E.filename == NULL) {
			editorSetStatusMessage("Save aborted");
			return;
		}
		editorSelectSyntaxHighlight();
	}

	int length;
	char *buf = editorRowsToString(&length);

	// Open the file and write to it
	// TODO:
	// Safer to write a temp file and then rename it to the file we're overwriting
	// Be sure to check for errors
	int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1) {
		if (ftruncate(fd, length) != -1) {
			if (write(fd, buf, length) == length) {
				close(fd);
				free(buf);
				E.dirty = 0;
				editorSetStatusMessage("%d bytes written to disk", length);
				return;
			}
		}
		close(fd);
	}
	free(buf);
	editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

// TODO:
// Should open in a new buffer
// Currently kills the existing buffer
void editorOpenFile() {
	size_t bufsize = 128;
	char *file = malloc(bufsize);
    file = editorPrompt("Select a file: %s (Use ESC/Enter)", NULL);
	editorKillCurrentBuffer();
	editorOpen(file);
	free(file);
}

/*** find ***/

void editorFindCallback(char *query, int key) {
	static int last_match = -1;
	static int direction = 1; // 1 for searching forward, -1 for backward

	static int saved_highlight_line;
	static char *saved_highlight = NULL;

	if (saved_highlight) {
		memcpy(E.row[saved_highlight_line].highlight, saved_highlight, E.row[saved_highlight_line].renderSize);
		free(saved_highlight);
		saved_highlight = NULL;
	}

	if (key == '\r' || key == '\x1b') {
		last_match = -1;
		direction = 1;
		return;
	} else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
		direction = 1;
	} else if (key == ARROW_LEFT || key == ARROW_UP) {
		direction = -1;
	} else {
		last_match = -1;
		direction = 1;
	}

	if (last_match == -1) {
		direction = 1;
	}

	int current = last_match;
	int i;
	for (i = 0; i< E.numRows; i++) {
		current += direction;
		if (current == -1) {
			current = E.numRows - 1;
		} else if (current == E.numRows) {
			current = 0;
		}
		editorRow *row = &E.row[current];
		char *match = strstr(row->render, query);
		if (match) {
			last_match = current;
			E.cy = current;
			E.cx = editorRowRxToCx(row, match - row->render);
			E.rowOffset = E.numRows;

			saved_highlight_line = current;
			saved_highlight = malloc(row->renderSize);
			memcpy(saved_highlight, row->highlight, row->renderSize);
			memset(&row->highlight[match - row->render], HL_MATCH, strlen(query));
			break;
		}
	}

}

void editorFind() {
	int saved_cx = E.cx;
	int saved_cy = E.cy;
	int saved_colOffset = E.colOffset;
	int saved_rowOffset = E.rowOffset;

	char *query = editorPrompt("Search: %s (Use ESC/Arrow/Enter)", editorFindCallback);

	if (query) {
		free(query);
	} else {
		E.cx = saved_cx;
		E.cy = saved_cy;
		E.colOffset = saved_colOffset;
		E.rowOffset = saved_rowOffset;
	}
}

/*** go to ***/

void editorGoToCallback(char *query, int key) {

	if (key == '\r' || key == '\x1b') {
		return;
	} else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
		return;
	} else if (key == ARROW_LEFT || key == ARROW_UP) {
		return;
	}

	intmax_t num = strtoumax(query, NULL, 10);
	if (num == UINTMAX_MAX && errno == ERANGE) {
		// Could not convert
		return;
	} else {
 		// Our rows are indexed from 0, but lines start at 1
 		// Need to subtract 1 so they match
		num--;

		// Move the cursor
		if (num > E.numRows) {
			E.cy = E.numRows;
		} else if (num < 0) {
			E.cy = 0;
		} else {
			E.cy = num;
		}
	}
}

// TODO
// Rewrite so we don't have to rely on a callback here
// Callback does an incremental search, which is weird in this case
void editorGoToLine() {

	int curr_cy = E.cy;

	char *query = editorPrompt("Go To: %s (Use ESC/Enter)", editorGoToCallback);

	if (query) {
		free(query);
	}
}

/*** append buffer  ***/

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) {
        return;
    }
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}

/*** input  ***/

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
	size_t bufsize = 128;
	char *buf = malloc(bufsize);

	size_t buflen = 0;
	buf[0] = '\0';

	while (1) {
		editorSetStatusMessage(prompt, buf);
		editorRefreshScreen();

		int c = editorReadKey();
		if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
			if (buflen != 0) {
				buf[--buflen] = '\0';
			}
		} else if (c == '\x1b') {
			editorSetStatusMessage("");
			if (callback) {
				callback(buf, c);
			}
			free(buf);
			return NULL;
		} else if (c == '\r') {
			if (buflen != 0) {
				editorSetStatusMessage("");
				if (callback) {
					callback(buf, c);
				}
				return buf;
			}
		} else if (!iscntrl(c) && c < 128) {
			if (buflen == bufsize - 1) {
				bufsize *= 2;
				buf = realloc(buf, bufsize);
			}
			buf[buflen++] = c;
			buf[buflen] = '\0';
		}

		if (callback) {
			callback(buf, c);
		}
	}
}

void editorMoveCursor(int key) {
    // row should point to the editorRow that the cursor is on
    // E.cy can be one past the last line, so row might be NULL
    editorRow *row = (E.cy >=  E.numRows) ? NULL : &E.row[E.cy];

    switch(key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
            } else if (E.cy > 0) {
                // Move to the end of the previous line
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            } else if (row && E.cx == row->size) {
                // Move to the start of the next line
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0) {
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.numRows) {
                E.cy++;
            }
            break;
    }

    // E.cx should snap to the end of the line
    // e.g. line 1 is really long, line 2 is not, you move down to line 2
    row = (E.cy >= E.numRows) ? NULL : &E.row[E.cy];
    int rowLen = row ? row->size : 0;
    if (E.cx > rowLen) {
        E.cx = rowLen;
    }
}

// Handles the keypress returned by editorReadKey()
void editorProcessKeypress() {
	static int quit_times = QUIT_TIMES;

    int c = editorReadKey();
    switch(c) {
    	case '\r':
    		editorInsertNewline();
    		break;

        case CTRL_KEY('q'):
        	if (E.dirty && quit_times > 0) {
        		editorSetStatusMessage("WARNING!!! File has unsaved changes. " "Press Ctrl-Q %d more times to quit.", quit_times);
        		quit_times--;
        		return;
        	}
            write(STDOUT_FILENO, "\x1b[2j", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case CTRL_KEY('s'):
        	editorSave();
        	break;

        case CTRL_KEY('o'):
        	editorOpenFile();
			break;

        case CTRL_KEY('k'):
        	editorKillCurrentBuffer();
			break;

        case CTRL_KEY('b'):
        case HOME_KEY:
            E.cx = 0;
            break;

        case CTRL_KEY('e'):
        case END_KEY:
            if (E.cy < E.numRows) {
                E.cx = E.row[E.cy].size;
            }
            break;

        case CTRL_KEY('f'):
        	editorFind();
        	break;

        case CTRL_KEY('g'):
        	editorGoToLine();
        	break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
        	if (c == DEL_KEY) {
        		editorMoveCursor(ARROW_RIGHT);
        		editorDeleteChar();
        	} else if (c == BACKSPACE) {
        		editorDeleteChar();
        	}
        	break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP) {
                    E.cy = E.rowOffset;
                } else if (c == PAGE_DOWN) {
                    E.cy = E.rowOffset + E.screenRows - 1;
                    if (E.cy > E.numRows) {
                        E.cy = E.numRows;
                    }
                }

                int times = E.screenRows;
                while(times--) {
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
            }
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;

        // Traditionally used to refresh the screen in terminal programs
        // We don't want to do anything with it
        case CTRL_KEY('l'):
        case '\x1b':
        	break;

        default:
        	editorInsertChar(c);
        	break;
    }

    quit_times = QUIT_TIMES;
}

/*** output ***/

void editorScroll() {
    E.rx = 0;
    if (E.cy < E.numRows) {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }

    // Are we above the window?
    if (E.cy < E.rowOffset) {
        E.rowOffset = E.cy;
    }

    // Are we below the window?
    if (E.cy >= E.rowOffset + E.screenRows) {
        E.rowOffset = E.cy - E.screenRows + 1;
    }

    // Are we left of the window?
    if (E.rx < E.colOffset) {
        E.colOffset = E.rx;
    }

    // Are we right of the window?
    if (E.rx >= E.colOffset + E.screenCols) {
        E.colOffset = E.rx - E.screenCols + 1;
    }
}

void editorDrawRows(struct abuf *ab) {
    int y;
    for(y = 0; y < E.screenRows; y++) {
        int fileRow = y + E.rowOffset;
        if (fileRow >= E.numRows) { // Check if the current row is part of the text buffer
            if (E.numRows == 0 && y == E.screenRows / 3) {
                char welcome[80];
                int welcomeLen = snprintf(welcome, sizeof(welcome), "Mio -- version %s", VERSION);
                if (welcomeLen > E.screenCols) {
                    welcomeLen = E.screenCols;
                }
                int padding = (E.screenCols - welcomeLen) / 2; // Center the message
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) {
                    abAppend(ab, " ", 1);
                }
                abAppend(ab, welcome, welcomeLen);
            } else {
                abAppend(ab, "~", 1);
            }
        } else {
            int len = E.row[fileRow].renderSize - E.colOffset;

            if (len < 0) {
                len = 0;
            }

            if (len > E.screenCols) {
                len = E.screenCols;
            }

            char *c = &E.row[fileRow].render[E.colOffset];
            unsigned char *highlight = &E.row[fileRow].highlight[E.colOffset];
            int current_color = -1;
            int j;
            for (j = 0; j < len; j++) {
            	if (iscntrl(c[j])) {
            		char sym = (c[j] <= 26) ? '@' + c[j] : '?';
            		abAppend(ab, "\x1b[7m", 4);
            		abAppend(ab, &sym, 1);
            		abAppend(ab, "\x1b[m", 3);
            		if (current_color != -1) {
            			char buf[16];
            			int colorLen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
            			abAppend(ab, buf, colorLen);
            		}
            	} else if (highlight[j] == HL_NORMAL) {
            		if (current_color != -1) {
            			abAppend(ab, "\x1b[39m", 5);
            			current_color = -1;
            		}
            		abAppend(ab, &c[j], 1);
            	} else {
            		int color = editorSyntaxToColor(highlight[j]);
            		if (color != current_color) {
            			current_color = color;
            			char buf[16];
            			int colorLen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
            			abAppend(ab, buf, colorLen);
            		}
            		abAppend(ab, &c[j], 1);
            	}
            }
            abAppend(ab, "\x1b[39m", 5);

            //abAppend(ab, &E.row[fileRow].render[E.colOffset], len);
        }

        // K means Erase In Line
        abAppend(ab, "\x1b[K", 3);
        //if (y < E.screenRows - 1) { // removed to make room for status bar
            abAppend(ab, "\r\n", 2);
        //}
    }
}

void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4);
    // m is Select Graphic Rendition
    // 7 is inverted colors

    char status[80];
    char rightStatus[80];
    char commands[80];


 //    // Print the commands status bar
 //    int commandLen = snprintf(commands, sizeof(commands), "^Q Quit | ^S Save | ^F Find | ^B Begin Line | ^E End Line | ^G Go To Line");
 //    if (commandLen > E.screenCols) {
 //        commandLen = E.screenCols;
 //    }
	// abAppend(ab, commands, commandLen);
	// // Fill the rest of the line
	// while (commandLen < E.screenCols) {
 //        abAppend(ab, " ", 1);
 //        commandLen++;
 //    }
 //    abAppend(ab, "\r\n", 2); // print a new line for our next status

    abAppend(ab, "^Q", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Quit ", 6);
    
    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^S", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Save ", 6);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^O", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Open ", 6);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^K", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Kill ", 6);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^F", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Find ", 6);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^B", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " BeginLine ", 11);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^E", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " EndLine ", 9);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^G", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " GoTo ", 6);

    abAppend(ab, "\r\n", 2); // new line

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^N", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Next ", 6);

    abAppend(ab, "\x1b[7m", 4); // invert
    abAppend(ab, "^P", 2);
    abAppend(ab, "\x1b[m", 3); // normal
    abAppend(ab, " Prev ", 6);

	abAppend(ab, "\x1b[7m", 4); // invert
	abAppend(ab, "\r\n", 2); // print a new line for our next status

    // Print the file status bar
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", E.filename ? E.filename : "[No Name]", E.numRows, E.dirty ? "(modified)" : "");
    int rightLen = snprintf(rightStatus, sizeof(rightStatus), "%s | %d/%d", E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numRows);

    if (len > E.screenCols) {
        len = E.screenCols;
    }

    abAppend(ab, status, len);

    while (len < E.screenCols) {
        if (E.screenCols - len == rightLen) {
            abAppend(ab, rightStatus, rightLen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }
    }

    abAppend(ab, "\x1b[m", 3); // switch back to normal formatting
    abAppend(ab, "\r\n", 2); // print a new line for our next status

}

void editorDrawMessageBar(struct abuf *ab) {
    abAppend(ab, "\x1b[K", 3); // clear the message bar
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screenCols) {
        msglen = E.screenCols;
    }

    if (msglen && time(NULL) - E.statusmsg_time < 5) {
        abAppend(ab, E.statusmsg, msglen);
    }
}

// Clears the screen
void editorRefreshScreen() {
    editorScroll();

    // Use abuf to prevent calling write() several times
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    // l means reset mode

    //abAppend(&ab, "\x1b[2J", 4);
    // 4 means write 4 bytes
    // \x1b is the escape character, and our first byte
    // Escape sequences must start with the escape character
    // Followed by [, which tells the terminal to do some formatting
    // J means Erase In Display, 2 means to clear the whole screen

    abAppend(&ab, "\x1b[H", 3);
    // H means Cursor Position

    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy -E.rowOffset) + 1, (E.rx - E.colOffset) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);
    // h means Set Mode

    write(STDOUT_FILENO, ab.b, ab.len);

    abFree(&ab);
}

// variadic function
void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/*** main  ***/

void initEditor() {
    // Init cursor at top left
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowOffset = 0;
    E.colOffset = 0;
    E.numRows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;

    if (getWindowSize(&E.screenRows, &E.screenCols) == -1) {
        die("getWindowSize");
    }

    // Create space for status bar
    E.screenRows -= 4;
}

int main(int argc, char *argv[]) {
	enableRawMode();
    initEditor(); // might make sense to put enableRawMode() in here
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    // Init status message with key bindings
    editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");

	// Main loop, quits on ctrl+q
	while (1) {
	    editorRefreshScreen();
	    editorProcessKeypress();
	}

    return 0;
}
