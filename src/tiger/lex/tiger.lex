%filenames = "scanner"

 /*
  * Please don't modify the lines above.
  */

/// KH-note:
/// Flex is a C++ alternative of Lex.
/// A Flex file has 2 parts, divided by "%%".
/// The first one are definitions, and the second are regex patterns.
/// Code blocks, or the third part of Lex, are in "scanner.h".

 /* You can add lex definitions here. */

/// KH-note: Use "%x" to define a set of states.
/// By default, StartCondition_::INITIAL is the starting state.
%x COMMENT STR IGNORE

/// KH-note: This is created for \n, \\, \", etc.
%x ESCAPE

/// KH-note: Some pre-defs of common-used patterns.

/// KH-note: Arabic numbers
Number          [0-9]
/// KH-note: Pure letters
Letter          [A-Za-z]
/// KH-note: Including numbers, letters and underline. Used in token names.
Character       {Number}|{Letter}|_
/// KH-note: Int in tiger is unsigned. Negative will be considered 0-x.
BinInt          0[Bb][01]+
OctInt          0[0-7]+
DecInt          0|[1-9]{Number}*
HexInt          0[Xx][0-9A-Fa-f]+
Uint            {BinInt}|{OctInt}|{DecInt}|{HexInt}
/// KH-note: Four blank chars. Some invisible chars are not included.
Blanked         [ \n\t\f]

%%

/// KH-note: Punctuations are of highest priority. Use "" to specify a literal string.
/// adjust() is a function of scanner.h. It pushes current position and error position forward.
","     { adjust(); return Parser::COMMA; }
":"     { adjust(); return Parser::COLON; }
";"     { adjust(); return Parser::SEMICOLON; }
"("     { adjust(); return Parser::LPAREN; }
")"     { adjust(); return Parser::RPAREN; }
"["     { adjust(); return Parser::LBRACK; }
"]"     { adjust(); return Parser::RBRACK; }
"{"     { adjust(); return Parser::LBRACE; }
"}"     { adjust(); return Parser::RBRACE; }
"."     { adjust(); return Parser::DOT; }

/// KH-note: Calculators
"+"     { adjust(); return Parser::PLUS; }
"-"     { adjust(); return Parser::MINUS; }
"*"     { adjust(); return Parser::TIMES; }
"/"     { adjust(); return Parser::DIVIDE; }
"="     { adjust(); return Parser::EQ; }
"<>"    { adjust(); return Parser::NEQ; }
"<"     { adjust(); return Parser::LT; }
"<="    { adjust(); return Parser::LE; }
">"     { adjust(); return Parser::GT; }
">="    { adjust(); return Parser::GE; }
"&"     { adjust(); return Parser::AND; }
"|"     { adjust(); return Parser::OR; }
":="    { adjust(); return Parser::ASSIGN; }

 /* reserved words */
/// KH-note: Or say "Key Words"
"array"     { adjust(); return Parser::ARRAY; }
"if"        { adjust(); return Parser::IF; }
"then"      { adjust(); return Parser::THEN; }
"else"      { adjust(); return Parser::ELSE; }
"while"     { adjust(); return Parser::WHILE; }
"for"       { adjust(); return Parser::FOR; }
"to"        { adjust(); return Parser::TO; }
"do"        { adjust(); return Parser::DO; }
"let"       { adjust(); return Parser::LET; }
"in"        { adjust(); return Parser::IN; }
"end"       { adjust(); return Parser::END; }
"of"        { adjust(); return Parser::OF; }
"break"     { adjust(); return Parser::BREAK; }
"nil"       { adjust(); return Parser::NIL; }
"function"  { adjust(); return Parser::FUNCTION; }
"var"       { adjust(); return Parser::VAR; }
"type"      { adjust(); return Parser::TYPE; }

/// KH-note: Identifiers include variable names, function names, etc.
{Letter}{Character}*    { adjust(); return Parser::ID; }
/// KH-note: Use pre-defs.
{Uint}                  { adjust(); return Parser::INT; }

/// KH-note: String starts from \"
"\""    {
    /// KH-note: Firstly ignore the starting \"; It's not part of the string literal.
    adjust();
    /// KH-note: Initialize string_buf_ in scanner.h.
    string_buf_.assign("");
    /// KH-note: Move to the STR state using Flex API.
    begin(StartCondition_::STR);
}

/// KH-note: This only works in STR state; Others above are in INITIAL state by default.
<STR>   {
    /// KH-note: adjustStr() doesn't move error position.
    "\\"    { adjustStr(); begin(StartCondition_::ESCAPE); }

    /// KH-note: I have ESCAPE state; the '\"' in STR state can't be part of "\\\"" 
    "\""    {
        /// KH-note: adjust() will move the error curser before \"; that's meaningless.
        adjustStr();
        /// KH-note: Flex API. It manually sets value of matched().
        setMatched(string_buf_);
        /// KH-note: The string ends; Return to default state.
        begin(StartCondition_::INITIAL);
        return Parser::STRING;
    }

    /// KH-note: Add other chars to buffer.
    /// Rules, or patterns declared earlier have higher priority.
    .       {adjustStr(); string_buf_ += matched();}
}

/// KH-note: ESCAPE only appear in string literals; So it can only be entered from STR state.
<ESCAPE>        {
    /// KH-note: Some common special chars.
    "\\"|"\""   { adjustStr(); begin(StartCondition_::STR); string_buf_ += matched(); }
    "n"         { adjustStr(); begin(StartCondition_::STR); string_buf_ += '\n'; }
    "t"         { adjustStr(); begin(StartCondition_::STR); string_buf_ += '\t'; }

    /// KH-note: \ddd represents an ASCII char using Octal number according to PPT.
    /// But testcase52 still uses decimal...???
    ([0-1]{Number}{Number})|(2[0-4]{Number})|(25[0-5]) {
        adjustStr();
        begin(StartCondition_::STR);
        string_buf_ += (char)atoi(matched().c_str());
    }

    /// KH-note: \f__f\, where f__f is {Blanked}+
    {Blanked}   {
        adjustStr();
        /// KH-note: Blanked includes '\n'
        if (matched()[0] == '\n') {
            errormsg_->Newline();
        }
        begin(StartCondition_::IGNORE);
    }

    /// KH-note: ASCII(x) == ASCII('\\\^x') + '@', '\\\^@' == '\0'
    \^[@-_]     {
        adjustStr();
        begin(StartCondition_::STR); 
        string_buf_ += (char)(matched()[1] - '@');
    }

    /// KH-note: Other illegal
    .           {
        adjust();
        begin(StartCondition_::STR); 
        errormsg_->Error(errormsg_->tok_pos_, "illegal token after an Escape: " + matched());
    }
}

/// KH-note: Only in STR -> ESCAPE -> IGNORE -> STR
<IGNORE>        {
    /// KH-note: IGNORE it.
    {Blanked}   {
        adjustStr();
        if (matched()[0] == '\n') {
            errormsg_->Newline();
        }
    }

    /// KH-note: End of IGNORE.
    "\\"    {
        adjustStr();
        begin(StartCondition_::STR);
    }

    /// KH-note: No other legal chars.
    .       {
        adjust();
        begin(StartCondition_::STR); 
        errormsg_->Error(errormsg_->tok_pos_, "illegal token in IGNORED string: " + matched());
    }
}

/// KH-note: Comments
"/*"        {
    /// KH-note: errormsg_ can't remember which line is it now.
    /// If an '\n' is in the comment, it must switch to a new line.
    /// adjust() is not required; After all there can't be any error in the comment.
    adjustStr();
    begin(StartCondition_::COMMENT);
    /// KH-note: This is used to remember how many layers are outside this /* */.  
    comment_level_ = 0;
}

<COMMENT>   {
    /// KH-note: A deeper layer of /* */.
    "/*"    { adjustStr(); comment_level_ ++; }
    /// KH-note: End of current layer.
    "*/"    {
        adjustStr();
        /// KH-note: If all layers are closed, switch to normal.
        if (comment_level_ == 0) {
            begin(StartCondition_::INITIAL);
        }
        comment_level_ --;
    }

    /// KH-note: All other chars are ignored.
    .       {adjustStr();}
    \n      {adjustStr(); errormsg_->Newline();}
}

 /*
  * skip white space chars.
  * space, tabs and LF
  */
[ \t]+ {adjust();}
\n {adjust(); errormsg_->Newline();}

 /* illegal input */
. {adjust(); errormsg_->Error(errormsg_->tok_pos_, "illegal token");}
