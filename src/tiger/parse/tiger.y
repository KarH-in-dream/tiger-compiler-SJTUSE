%filenames parser
%scanner tiger/lex/scanner.h
%baseclass-preinclude tiger/absyn/absyn.h

 /*
  * Please don't modify the lines above.
  */

/// KH-note:
/// This is the Debug option. View in generated parse.cc:
/// Array s_k == kth state, starting from s_0, consists of all possible transitions from it.
/// {{t}, {s}}, look ahead to t and shift to s. {{0}, {-s}}, reduce with rule s.
/// View reduce rules in s_productionInfo.
/// %debug

/// KH-note:
/// Bisonc++ also has 2 parts divided by "%%", similiar to Flex.
/// Define terminals and nonterminals in this part, the Bisonc++ Directives.

%union {
  int ival;
  std::string* sval;
  sym::Symbol *sym;
  absyn::Exp *exp;
  absyn::ExpList *explist;
  absyn::Var *var;
  absyn::DecList *declist;
  absyn::Dec *dec;
  absyn::EFieldList *efieldlist;
  absyn::EField *efield;
  absyn::NameAndTyList *tydeclist;
  absyn::NameAndTy *tydec;
  absyn::FieldList *fieldlist;
  absyn::Field *field;
  absyn::FunDecList *fundeclist;
  absyn::FunDec *fundec;
  absyn::Ty *ty;
  }

/// KH-note: 
/// Grammar: `%token <${field name declared in %union}> ${nonterminal name}`
/// By convention, name terminals with UPPERCASE and nonterminals with LOWERCASE.

%token <sym> ID
%token <sval> STRING
%token <ival> INT

%token
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
  LBRACE RBRACE DOT
  /// KH-note: These symbols should be defined with priority.
  // PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
  // AND OR ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF
  BREAK NIL
  FUNCTION VAR TYPE

 /* token priority */
/// KH-note:
/// Declare with operator precedence
///   Let op. Deal with `x op y op z`:
///     %left => `(x op y) op z`, %right => `x op (y op z)`
///     %nonassoc => this is undefined. (eg. wtf is `x == y == z`?)
///   These are also declarations.
/// Token priority
///   Token declared earlier has higher priority.
///   Tokens declared in the same line has same priority.
/// Overrule default
///   Use `%prec ${TOKEN}` in production rules to modify its priority to the same as TOKEN.

/// KH-note: I refer to C++ priority for inspiration.
/// C++ Lv16: assignments       -- support `a = b = 1` grammar
/// C++ Lv14: OR, C++ Lv13: AND -- avoid conflicts of `A or B and C`
/// C++ Lv09: EQ, NEQ           -- better support `bool ok = a <= b`
/// C++ Lv08: comparing
/// C++ Lv06: PLUS, MINUS
/// C++ Lv05: TIMES, DIVIDE
/// C++ Lv03: unary operators   -- however no NOT in tiger language.

%right ASSIGN
%left OR
%left AND
%nonassoc EQ NEQ
%nonassoc LT LE GT GE
%left PLUS MINUS
%left TIMES DIVIDE
%nonassoc NEGATIVE

/// KH-note: Declare nonterminals here.

%type <exp> exp expseq
%type <explist> actuals nonemptyactuals sequencing sequencing_exps
%type <var> lvalue one oneormore
%type <declist> decs decs_nonempty
%type <dec> decs_nonempty_s vardec
%type <efieldlist> rec rec_nonempty
%type <efield> rec_one
%type <tydeclist> tydec
%type <tydec> tydec_one
%type <fieldlist> tyfields tyfields_nonempty
%type <field> tyfield
%type <ty> ty
%type <fundeclist> fundec
%type <fundec> fundec_one

%start program

%%

/// KH-note:
/// Define production rules here.
/// The basic grammar is <nonterminal>: [ [symbol](divided by space)+ {<C++ code>} ](divided by |)*;
///   Can convert to empty (or epsilon mark).
/// $$ and $n are placeholders for token values. $$ is the return value, $n is the n-th matched token.

program:  exp  {absyn_tree_ = std::make_unique<absyn::AbsynTree>($1);};


/// KH-note: ----------------Var----------------

lvalue:  ID  {$$ = new absyn::SimpleVar(scanner_.GetTokPos(), $1);}
  |  oneormore  {$$ = $1;}
  ;

/// KH-note: Read absyn.h so that there's nothing missed.

/// KH-note: 
/// "one" is created to avoid conflict.
/// Suppose we need to parse `ID LBRACK ID RBRACK ASSIGN INT`.
/// If ID can directly reduce to "one":
///   Shift ID1.
///   Look ahead, get LBRACK:
///     Matched `oneormore LBRACK exp RBRACK`. Need to reduce to "oneormore".
///     Matched `ID LBRACK exp RBRACK OF exp`. Need to shift LBRACK.
///   That's shift-reduce conflict. BAM!!!
/// But if we have "one" that means one step:
///   Shift ID1.
///   Look ahead, get LBRACK. No reduce available, shift LBRACK.
///   Look ahead, get ID2. No reduce available, shift ID2.
///   Look ahead, get RBRACK. Do reduce ID2 -> exp.
///   Look ahead, get RBRACK. No reduce available, shift RBRACK.
///   Look ahead, get ASSIGN. Do reduce ID1 LBRACK exp RBRACK -> one.
///   We don't get OF here! Double BAM!!!

one:
  ID DOT ID
    {
      $$ = new absyn::FieldVar(scanner_.GetTokPos(), new absyn::SimpleVar(scanner_.GetTokPos(), $1), $3);
    } |
  ID LBRACK exp RBRACK
    {
      $$ = new absyn::SubscriptVar(scanner_.GetTokPos(), new absyn::SimpleVar(scanner_.GetTokPos(), $1), $3);
    } ;

oneormore:
  one
    {
      $$ = $1;
    } |
  oneormore DOT ID
    {
      $$ = new absyn::FieldVar(scanner_.GetTokPos(), $1, $3);
    } |
  oneormore LBRACK exp RBRACK
    {
      $$ = new absyn::SubscriptVar(scanner_.GetTokPos(), $1, $3);
    } ;


/// KH-note: ----------------Exp----------------

exp:
  lvalue
    {
      // KH-note: lvalue is a Var. Wrap it to Exp type.
      $$ = new absyn::VarExp(scanner_.GetTokPos(), $1);
    } |
  NIL
    {
      $$ = new absyn::NilExp(scanner_.GetTokPos());
    } |
  INT
    {
      $$ = new absyn::IntExp(scanner_.GetTokPos(), $1);
    } |
  STRING
    {
      $$ = new absyn::StringExp(scanner_.GetTokPos(), $1);
    } |
  ID LPAREN actuals RPAREN
    {
      $$ = new absyn::CallExp(scanner_.GetTokPos(), $1, $3);
    } |

  /// KH-note: Keep AND and OR op (differ from textbook).
  exp AND exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::AND_OP, $1, $3);
    } |
  exp OR exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::OR_OP, $1, $3);
    } |
  exp PLUS exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::PLUS_OP, $1, $3);
    } |
  exp MINUS exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::MINUS_OP, $1, $3);
    } |
  /// KH-note:
  /// In tiger language, `-a` is `0-a`.
  /// Use %prec to manually define its priority.
  /// NEGATIVE is just a placeholder, not an used token.
  MINUS exp %prec NEGATIVE
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::MINUS_OP, 
        new absyn::IntExp(scanner_.GetTokPos(), 0), $2);
    } |
  exp TIMES exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::TIMES_OP, $1, $3);
    } |
  exp DIVIDE exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::DIVIDE_OP, $1, $3);
    } |
  exp EQ exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::EQ_OP, $1, $3);
    } |
  exp NEQ exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::NEQ_OP, $1, $3);
    } |
  exp LT exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::LT_OP, $1, $3);
    } |
  exp LE exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::LE_OP, $1, $3);
    } |
  exp GT exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::GT_OP, $1, $3);
    } |
  exp GE exp
    {
      $$ = new absyn::OpExp(scanner_.GetTokPos(), absyn::Oper::GE_OP, $1, $3);
    } |

  ID LBRACE rec RBRACE
    {
      /// KH-note: 
      /// This is a record created with params.
      /// Struct of C++ is similiar to record in tiger language.
      $$ = new absyn::RecordExp(scanner_.GetTokPos(), $1, $3);
    } |
  sequencing
    {
      /// KH-note: A list of expressions, the last exp is return value.
      $$ = new absyn::SeqExp(scanner_.GetTokPos(), $1);
    } |
  lvalue ASSIGN exp
    {
      $$ = new absyn::AssignExp(scanner_.GetTokPos(), $1, $3);
    } |
  
  IF exp THEN exp
    {
      $$ = new absyn::IfExp(scanner_.GetTokPos(), $2, $4, nullptr);
    } |
  IF exp THEN exp ELSE exp
    {
      $$ = new absyn::IfExp(scanner_.GetTokPos(), $2, $4, $6);
    } |

  WHILE exp DO exp
    {
      $$ = new absyn::WhileExp(scanner_.GetTokPos(), $2, $4);
    } |
  FOR ID ASSIGN exp TO exp DO exp
    {
      $$ = new absyn::ForExp(scanner_.GetTokPos(), $2, $4, $6, $8);
    } |
  BREAK
    {
      $$ = new absyn::BreakExp(scanner_.GetTokPos());
    } |
  LET decs IN expseq END
    {
      $$ = new absyn::LetExp(scanner_.GetTokPos(), $2, $4);
    } |
  ID LBRACK exp RBRACK OF exp
    {
      /// KH-note: 
      /// ArrayExp is used to create a new Array object.
      /// If you want to get an element of the array, see SubscriptVar.
      $$ = new absyn::ArrayExp(scanner_.GetTokPos(), $1, $3, $6);
    } |
  LPAREN RPAREN
    {
      $$ = new absyn::VoidExp(scanner_.GetTokPos());
    } |
  /// KH-note: This rule is used to simplify the parse tree.
  LPAREN exp RPAREN
    {
      $$ = $2;
    } ;

expseq:
  /* empty */
    {
      $$ = new absyn::VoidExp(scanner_.GetTokPos());
    } |
  sequencing_exps
    {
      /// KH-note: From ExpList to Exp
      $$ = new absyn::SeqExp(scanner_.GetTokPos(), $1);
    } ;


/// KH-note: ----------------ExpList----------------

actuals:
  /* empty */
    {
      /// KH-note: Here should not be nullptr. actuals are used to pass params in CallExp.
      $$ = new absyn::ExpList();
    } |
  nonemptyactuals
    {
      $$ = $1;
    } ;

nonemptyactuals:
  exp
    {
      $$ = new absyn::ExpList($1);
    } |
  exp COMMA nonemptyactuals
    {
      $$ = $3 -> Prepend($1);
    } ;

sequencing:
  /// KH-note: 
  /// sequencing is `\((exp;)+exp\)`, at least 2 exps.
  /// We simplify (exp) into exp to make it more clear.
  LPAREN exp SEMICOLON sequencing_exps RPAREN
    {
      $$ = $4 -> Prepend($2);
    } ;

sequencing_exps:
  /// KH-note: 
  /// sequencing_exps has at least one exp in it.
  exp
    {
      $$ = new absyn::ExpList($1);
    } |
  exp SEMICOLON sequencing_exps
    {
      $$ = $3 -> Prepend($1);
    } ;


/// KH-note: ----------------DecList----------------

decs:
  /* empty */
    {
      /// KH-note: Here should not be nullptr for safety.
      $$ = new absyn::DecList();
    } |
  decs_nonempty
    {
      $$ = $1;
    } ;

decs_nonempty:
  decs_nonempty_s
    {
      /// KH-note: decs_nonempty_s is start of decs_nonempty
      $$ = new absyn::DecList($1);
    } |
  decs_nonempty_s decs_nonempty
    {
      $$ = $2 -> Prepend($1);
    } ;


/// KH-note: ----------------Dec----------------

decs_nonempty_s:
  fundec
    {
      $$ = new absyn::FunctionDec(scanner_.GetTokPos(), $1);
    } |
  vardec
    {
      /// KH-note: Vardec is not a list and thus different.
      $$ = $1;
    } |
  tydec
    {
      $$ = new absyn::TypeDec(scanner_.GetTokPos(), $1);
    } ;

vardec:
  VAR ID ASSIGN exp
    {
      $$ = new absyn::VarDec(scanner_.GetTokPos(), $2, nullptr, $4);
    } |
  /// KH-note: 
  /// VarDec's param typ is sym::Symbol *, not Ty.
  /// Ty is the original declared type, eg. int[10], and the symbol is its alias.
  VAR ID COLON ID ASSIGN exp
    {
      $$ = new absyn::VarDec(scanner_.GetTokPos(), $2, $4, $6);
    } ;


/// KH-note: ----------------EFieldList----------------

rec:
  /* empty */
    {
      /// KH-note: 
      /// Here should not be nullptr for safety.
      /// EField is symbol + exp. rec is used to create a new RecordExp.
      $$ = new absyn::EFieldList();
    } |
  rec_nonempty
    {
      $$ = $1;
    } ;

rec_nonempty:
  rec_one
    {
      $$ = new absyn::EFieldList($1);
    } |
  rec_one COMMA rec_nonempty
    {
      $$ = $3 -> Prepend($1);
    } ;


/// KH-note: ----------------EField----------------

rec_one:
  ID EQ exp
    {
      $$ = new absyn::EField($1, $3);
    } ;


/// KH-note: ----------------NameAndTyList----------------

tydec:
  tydec_one
    {
      $$ = new absyn::NameAndTyList($1);
    } |
  /// KH-note: There's no special seperators between type definitions.
  tydec_one tydec
    {
      $$ = $2 -> Prepend($1);
    } ;


/// KH-note: ----------------NameAndTy----------------

tydec_one:
  TYPE ID EQ ty
    {
      $$ = new absyn::NameAndTy($2, $4);
    } ;


/// KH-note: ----------------FieldList----------------

/// KH-note: 
/// tyfields are used in representing a record type's fields.
/// eg. `type list = {`first: int, rest: list`}`
tyfields:
  /* empty */
    {
      $$ = new absyn::FieldList();
    } |
  tyfields_nonempty
    {
      $$ = $1;
    } ;

tyfields_nonempty:  
  tyfield
    {
      $$ = new absyn::FieldList($1);
    } |
  tyfield COMMA tyfields_nonempty
    {
      $$ = $3 -> Prepend($1);
    } ;


/// KH-note: ----------------Field----------------

tyfield:
  ID COLON ID
    {
      $$ = new absyn::Field(scanner_.GetTokPos(), $1, $3);
    } ;


/// KH-note: ----------------Field----------------

/// KH-note: ty is used to describe an existing type in TYPE definitions.
ty:
  ID
    {
      $$ = new absyn::NameTy(scanner_.GetTokPos(), $1);
    } |
  LBRACE tyfields RBRACE
    {
      $$ = new absyn::RecordTy(scanner_.GetTokPos(), $2);
    } |
  ARRAY OF ID
    {
      $$ = new absyn::ArrayTy(scanner_.GetTokPos(), $3);
    } ;


/// KH-note: ----------------FunDecList----------------

/// KH-note: 
/// In tiger language, nested FunctionDec is used to support recursive functions.
/// If func a calls b, and func b calls a, then they should be in the same FunDecList.
fundec:
  fundec_one
    {
      $$ = new absyn::FunDecList($1);
    } |
  fundec_one fundec
    {
      $$ = $2 -> Prepend($1);
    } ;


/// KH-note: ----------------FunDec----------------

fundec_one:
  FUNCTION ID LPAREN tyfields RPAREN EQ exp
    {
      $$ = new absyn::FunDec(scanner_.GetTokPos(), $2, $4, nullptr, $7);
    } |
  FUNCTION ID LPAREN tyfields RPAREN COLON ID EQ exp
    {
      $$ = new absyn::FunDec(scanner_.GetTokPos(), $2, $4, $7, $9);
    } ;