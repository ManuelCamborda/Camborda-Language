#pragma once

enum class TokenKind {
  EndOfFile,
  Invalid,

  Identifier,

  IntLiteral,
  FloatLiteral,
  CharLiteral,
  StringLiteral,

  KwLet,
  KwConst,
  KwFunction,
  KwStruct,
  KwUnion,
  KwEnum,
  KwClass,
  KwType,
  KwImport,
  KwExport,
  KwFrom,
  KwAs,
  KwDefault,

  KwUnsafe,
  KwExtern,
  KwInline,
  KwVolatile,
  KwStatic,
  KwMut,
  KwPacked,
  KwAlign,
  KwSizeof,
  KwAlignof,
  KwOffsetof,
  KwAsm,
  KwGoto,

  KwIf,
  KwElse,
  KwFor,
  KwWhile,
  KwDo,
  KwSwitch,
  KwCase,
  KwBreak,
  KwContinue,
  KwReturn,
  KwThrow,
  KwTry,
  KwCatch,
  KwFinally,

  KwExtends,
  KwNew,
  KwDelete,
  KwThis,
  KwSuper,
  KwPublic,
  KwPrivate,
  KwProtected,
  KwReadonly,
  KwGet,
  KwSet,
  KwConstructor,

  KwTypeof,
  KwIn,
  KwOf,
  KwVoid,

  KwTrue,
  KwFalse,
  KwNull,

  KwI8,
  KwI16,
  KwI32,
  KwI64,
  KwI128,
  KwU8,
  KwU16,
  KwU32,
  KwU64,
  KwU128,
  KwF32,
  KwF64,
  KwIsize,
  KwUsize,
  KwBool,
  KwChar,
  KwStr,

  LeftParen,        // (
  RightParen,       // )
  LeftBrace,        // {
  RightBrace,       // }
  LeftBracket,      // [
  RightBracket,     // ]
  Semicolon,        // ;
  Comma,            // ,
  Dot,              // .
  DotDot,           // ..
  DotDotEqual,      // ..=
  DotDotDot,        // ...
  Colon,            // :
  ColonColon,       // ::
  Question,         // ?
  QuestionDot,      // ?.
  QuestionQuestion, // ??
  Arrow,            // =>
  ThinArrow,        // ->
  At,               // @
  Hash,             // #

  Plus,       // +
  Minus,      // -
  Star,       // *
  Slash,      // /
  Percent,    // %
  StarStar,   // **
  PlusPlus,   // ++
  MinusMinus, // --

  Equal,                 // =
  PlusEqual,             // +=
  MinusEqual,            // -=
  StarEqual,             // *=
  SlashEqual,            // /=
  PercentEqual,          // %=
  StarStarEqual,         // **=
  AmpersandEqual,        // &=
  PipeEqual,             // |=
  CaretEqual,            // ^=
  LessLessEqual,         // <<=
  GreaterGreaterEqual,   // >>=
  AmpAmpEqual,           // &&=
  PipePipeEqual,         // ||=
  QuestionQuestionEqual, // ??=

  EqualEqual,   // ==
  BangEqual,    // !=
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=

  Bang,           // !
  AmpAmp,         // &&
  PipePipe,       // ||
  Ampersand,      // &
  Pipe,           // |
  Caret,          // ^
  Tilde,          // ~
  LessLess,       // <<
  GreaterGreater, // >>

  LineComment,  // // ...
  BlockComment, // /* ... */
  Newline
};
