#include <stddef.h>

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

/*** filetypes ***/

// C/C++
char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };
char *C_HL_keywords[] = {
	"switch", "if", "while", "for", "break", "continue", "return", "else", "struct", "union",
	"typedef", "static", "enum", "class", "case",

	"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", NULL
};

// Crystal

// CSS

// Fish
char *Fish_HL_extensions[] = { ".fish" };
char *Fish_HL_keywords[] = {
	"function", "end", "set", "switch", "case", "return", "while", "if", "else", NULL
};

// Haskell

// HTML
char *HTML_HL_extensions[] = { ".html" };
char *HTML_HL_keywords[] = {
	"html", "head", "body", "div", "span", "ul", "ol", "li", "title", "a", "link",
	"script", "h1", "h2", "h3", "h4", "h5", "h6", "h7",

	"href|", "src|", "type|", "rel|", "id|", "class|", "onmouseover|", "onmouseleave|", NULL
};

// Javascript
char *Javascript_HL_extensions[] = { ".js", NULL };
char *Javascript_HL_keywords[] = {
	"break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete",
	"do", "else", "enum", "export", "extends", "finally", "for", "function", "if", "implements",
	"import", "in", "instanceof", "interface", "let", "new", "package", "private", "protected",
	"public", "return", "static", "super", "switch", "this", "throw", "try", "typeof", "var",
	"void", "while", "with", "yield", "true", "false", "null", "NaN", "global","window", "prototype",
	"constructor", "document", "isNaN", "arguments", "undefined",

	"Infinity|", "Array|", "Object|", "Number|", "String|", "Boolean|", "Function|", "ArrayBuffer|",
	"DataView|", "Float32Array|", "Float64Array|", "Int8Array|", "Int16Array|", "Int32Array|",
	"Uint8Array|", "Uint8ClampedArray|", "Uint32Array|", "Date|", "Error|", "Map|", "RegExp|",
	"Symbol|", "WeakMap|", "WeakSet|", "Set|", NULL
};

// MUMPS
char *MUMPS_HL_extensions[] = { ".m", ".mps", NULL };
char *MUMPS_HL_keywords[] = {
	"n", "f", "w", "s", "r", "d", "k", "i", "e", "o", "c", "u", "q", "h", "b", "g",

	"new", "for", "while", "set", "read", "do", "kill", "if", "else",
	"open", "close", "use", "quit", "halt", "hang", "break", "goto", NULL
};

// Objective-C

// PHP
char *PHP_HL_extensions[] = { ".php", NULL };
char *PHP_HL_keywords[] = {
    "if", "else", "elseif", "while", "for", "return", "class", "function", "public",
    "private", "extends", "use", "namespace", NULL
};

// Python

// Ruby
char *Ruby_HL_extensions[] = { ".rb", NULL };
char *Ruby_HL_keywords[] = {
	"def", "end", "require", "if", "elsif", "else", "for", "in", "while", "do", "begin",
	"until", "then", "break", "redo", "rescue", "class", "module", "return", NULL
};

// Scheme

// Swift

// Vimscript
char *Vimscript_HL_extensions[] = { ".vim", ".vimrc", NULL };
char * Vimscript_HL_keywords[] = {
    "function", "endfunction", "if", "else", "endif", "while", "endwhile", "let", "set",
    "wincmd", "autocmd", "execute", "colo", "silent", "map", "nmap", "nnoremap", "syntax",
    "return",

    "tabstop|", "expandtab|", "shiftwidth|", "number|", "showmap|", "mouse|", "splitbelow|",
    "splitright|", "hlsearch|", "incsearch|", "switchbuf", "laststatus", "clipboard|",
    "showmatch|", NULL
};

struct editorSyntax HLDB[] = {
	{
		"c",
		C_HL_extensions,
		C_HL_keywords,
		"//", "/*", "*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
		"html",
		HTML_HL_extensions,
		HTML_HL_keywords,
		"<!--", "<!--", "-->",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
		"fish",
		Fish_HL_extensions,
		Fish_HL_keywords,
		"#", NULL, NULL,
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
		"javascript",
		Javascript_HL_extensions,
		Javascript_HL_keywords,
		"//", "/*", "*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
		"mumps",
		MUMPS_HL_extensions,
		MUMPS_HL_keywords,
		";", NULL, NULL,
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
        "php",
        PHP_HL_extensions,
        PHP_HL_keywords,
        "//", "/*", "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
	{
		"ruby",
		Ruby_HL_extensions,
		Ruby_HL_keywords,
		"#", "=begin", "=end",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
        "vim",
        Vimscript_HL_extensions,
        Vimscript_HL_keywords,
        "\"", NULL, NULL,
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))