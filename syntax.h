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
char *Crystal_HL_extensions[] = { ".cr", NULL };
char *Crystal_HL_keywords[] = {
	"abstract", "alias", "as", "as?", "asm", "begin", "break", "case",
	"class", "def", "do", "else", "elsif", "end", "ensure", "enum",
	"extend", "false", "for", "fun", "if", "in", "include", "instance_sizeof",
	"is_a?", "lib", "macro", "module", "next", "nil", "nil?", "of", "out",
	"pointerof", "private", "protected", "require", "rescue", "responds_to?",
	"return", "select", "self", "sizeof", "struct", "super", "then", "true",
	"type", "typeof", "uninitialized", "union", "unless", "until", "verbatim",
	"when", "while", "with", "yield", NULL
};

// CSS
char *CSS_HL_extensions[] = { ".css", NULL };
char *CSS_HL_keywords[] = {
	"align-content:", "align-items:", "align-self:", "all:", "animation:",
	"animation-delay:", "animation-direction:", "animation-duration:", "animation-fill-mode:",
	"animation-iteration-count:", "animation-name:", "animation-play-state:",
	"animation-timing-function:",

	"backface-visibility:", "background:", "background-attachment:", "background-blend-mode:",
	"background-clip:", "background-color:", "background-image:", "background-origin:",
	"background-position:", "background-repeat:", "background-size:", "border:", "border-bottom:",
	"border-bottom-color:", "border-bottom-left-radius:", "border-bottom-right-radius:",
	"border-bottom-style:", "border-bottom-width:", "border-collapse:", "border-color:",
	"border-image:", "border-image-outset:", "border-image-repeat:", "border-image-slice",
	"border-image-source:", "border-image-width:", "border-left:", "border-left-color:",
	"border-left-style:", "border-left-width:", "border-radius:", "border-right:", "border-right-color:",
	"border-right-style:", "border-right-width:", "border-spacing:", "border-style:", "border-top:",
	"border-top-color:", "border-top-left-radius:", "border-top-right-radius:", "border-top-style:",
	"border-top-width:", "border-width:", "bottom:", "box-decoration-break:", "box-shadow:",
	"box-sizing:", "break-after:", "break-before:", "break-inside", 

	"caption-side:", "caret-color:", "clear:", "clip:", "color:", "column-count:",
	"column-fill:", "column-gap:", "column-rule:", "column-rule-color:", "column-rule-style:",
	"column-rule-width:", "column-span:", "column-width:", "columns:", "content:",
	"counter-increment:", "counter-reset:", "cursor:",
	
	"direction:", "display:", 
	
	"empty-cells:", 

	"filter:", "flex:", "flex-basis:", "flex-direction:", "flex-flow:", "flex-grow:",
	"flex-shrink:", "flex-wrap:", "float:", "font:", "font-family:", "font-feature-settings:",
	"font-kerning:", "font-language-override:", "font-size:", "font-size-adjust:", "font-stretch:",
	"font-style:", "font-synthesis:", "font-variant:", "font-variant-alternates:",
	"font-variant-caps:", "font-variant-east-asian:", "font-variant-ligatures:", "font-variant-numeric:",
	"font-variant-position:", "font-weight:",  

	"grid:", "grid-area:", "grid-auto-columns:", "grid-auto-flow:", "grid-auto-rows:",
	"grid-column:", "grid-column-end:", "grid-column-gap:", "grid-column-start:", "grid-gap:",
	"grid-row:", "grid-row-end:", "grid-row-gap:", "grid-row-start:", "grid-template:",
	"grid-template-areas:", "grid-template-columns:", "grid-template-rows:", 

	"hanging-punctuation:", "height:", "hyphens:", 

	"image-rendering:", "isolation:", 

	"justify-content:", 

	"left:", "letter-spacing:", "line-break:", "line-height:", "list-style:",
	"list-style-image:", "list-style-position:", "list-style-type:",

	"margin:", "margin-bottom:", "margin-left:", "margin-right:", "margin-top:",
	"max-height:", "max-width:", "min-height:", "min-width:", "mix-blend-mode:",

	"object-fit:", "object-position:", "opacity:", "order:", "orphans:", "outline:",
	"outline-color:", "outline-offset:", "outline-style:", "outline-width:", "overflow:",
	"overflow-wrap:", "overflow-x:", "overflow-y:", 

	"padding:", "padding-bottom:", "padding-left:", "padding-right:", "padding-top:",
	"page-break-after:", "page-break-before:", "page-break-inside:", "perspective:",
	"perspective-origin:", "pointer-events:", "position:", 

	"quotes:",

	"resize:", "right:",

	"scroll-behavior:", 
	
	"tab-size:", "table-layout:", "text-align:", "text-align-last:", "text-combine-upright:",
	"text-decoration:", "text-decoration-color:", "text-decoration-line:", "text-decoration-style:",
	"text-indent:", "text-justify:", "text-orientation:", "text-overflow:", "text-shadow:",
	"text-transform:", "text-underline-position:", "top:", "transform:", "transform-origin:",
	"transform-style:", "transition:", "transition-delay:", "transition-duration:",
	"transition-property:", "transition-timing-function",  

	"unicode-bidi:", "user-select:", 

	"vertical-align:", "visibility:", 

	"white-space:", "widows:", "width:", "word-break:", "word-spacing:", "word-wrap:",
	"writing-mode:", 

	"z-index:", 

	"html|", "head|", "body|", "a|", "div|", "ul|", "ol|", "li|", "span|",  NULL
};

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
		"crystal",
		Crystal_HL_extensions,
		Crystal_HL_keywords,
		"#", NULL, NULL,
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
	},
	{
		"css",
		CSS_HL_extensions,
		CSS_HL_keywords,
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