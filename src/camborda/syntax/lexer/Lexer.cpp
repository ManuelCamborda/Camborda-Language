#include "Lexer.hpp"
#include "Tokens.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace {

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isHexDigit(char c) {
  return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int hexValue(char c) {
  if (isDigit(c))
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return c - 'A' + 10;
}

bool isIdentStart(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
         c == '$' || static_cast<unsigned char>(c) >= 0x80;
}

bool isIdentPart(char c) { return isIdentStart(c) || isDigit(c); }

void appendUtf8(std::string &out, char32_t cp) {
  if (cp < 0x80) {
    out += static_cast<char>(cp);
  } else if (cp < 0x800) {
    out += static_cast<char>(0xC0 | (cp >> 6));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else if (cp < 0x10000) {
    out += static_cast<char>(0xE0 | (cp >> 12));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else {
    out += static_cast<char>(0xF0 | (cp >> 18));
    out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  }
}

const std::unordered_map<std::string, TokenKind> kKeywords = {
    {"let", TokenKind::KwLet},
    {"const", TokenKind::KwConst},
    {"function", TokenKind::KwFunction},
    {"struct", TokenKind::KwStruct},
    {"union", TokenKind::KwUnion},
    {"enum", TokenKind::KwEnum},
    {"class", TokenKind::KwClass},
    {"type", TokenKind::KwType},
    {"import", TokenKind::KwImport},
    {"export", TokenKind::KwExport},
    {"from", TokenKind::KwFrom},
    {"as", TokenKind::KwAs},
    {"default", TokenKind::KwDefault},

    {"unsafe", TokenKind::KwUnsafe},
    {"extern", TokenKind::KwExtern},
    {"inline", TokenKind::KwInline},
    {"volatile", TokenKind::KwVolatile},
    {"static", TokenKind::KwStatic},
    {"mut", TokenKind::KwMut},
    {"packed", TokenKind::KwPacked},
    {"align", TokenKind::KwAlign},
    {"sizeof", TokenKind::KwSizeof},
    {"alignof", TokenKind::KwAlignof},
    {"offsetof", TokenKind::KwOffsetof},
    {"asm", TokenKind::KwAsm},
    {"goto", TokenKind::KwGoto},

    {"if", TokenKind::KwIf},
    {"else", TokenKind::KwElse},
    {"for", TokenKind::KwFor},
    {"while", TokenKind::KwWhile},
    {"do", TokenKind::KwDo},
    {"switch", TokenKind::KwSwitch},
    {"case", TokenKind::KwCase},
    {"break", TokenKind::KwBreak},
    {"continue", TokenKind::KwContinue},
    {"return", TokenKind::KwReturn},
    {"throw", TokenKind::KwThrow},
    {"try", TokenKind::KwTry},
    {"catch", TokenKind::KwCatch},
    {"finally", TokenKind::KwFinally},

    {"extends", TokenKind::KwExtends},
    {"new", TokenKind::KwNew},
    {"delete", TokenKind::KwDelete},
    {"this", TokenKind::KwThis},
    {"super", TokenKind::KwSuper},
    {"public", TokenKind::KwPublic},
    {"private", TokenKind::KwPrivate},
    {"protected", TokenKind::KwProtected},
    {"readonly", TokenKind::KwReadonly},
    {"get", TokenKind::KwGet},
    {"set", TokenKind::KwSet},
    {"constructor", TokenKind::KwConstructor},

    {"typeof", TokenKind::KwTypeof},
    {"in", TokenKind::KwIn},
    {"of", TokenKind::KwOf},
    {"void", TokenKind::KwVoid},

    {"true", TokenKind::KwTrue},
    {"false", TokenKind::KwFalse},
    {"null", TokenKind::KwNull},

    {"i8", TokenKind::KwI8},
    {"i16", TokenKind::KwI16},
    {"i32", TokenKind::KwI32},
    {"i64", TokenKind::KwI64},
    {"i128", TokenKind::KwI128},
    {"u8", TokenKind::KwU8},
    {"u16", TokenKind::KwU16},
    {"u32", TokenKind::KwU32},
    {"u64", TokenKind::KwU64},
    {"u128", TokenKind::KwU128},
    {"f32", TokenKind::KwF32},
    {"f64", TokenKind::KwF64},
    {"isize", TokenKind::KwIsize},
    {"usize", TokenKind::KwUsize},
    {"bool", TokenKind::KwBool},
    {"char", TokenKind::KwChar},
    {"str", TokenKind::KwStr},
};

struct OpEntry {
  const char *text;
  TokenKind kind;
};

const OpEntry kOperators[] = {
    {"**=", TokenKind::StarStarEqual},
    {"<<=", TokenKind::LessLessEqual},
    {">>=", TokenKind::GreaterGreaterEqual},
    {"&&=", TokenKind::AmpAmpEqual},
    {"||=", TokenKind::PipePipeEqual},
    {"?\?=", TokenKind::QuestionQuestionEqual},
    {"..=", TokenKind::DotDotEqual},
    {"...", TokenKind::DotDotDot},

    {"?.", TokenKind::QuestionDot},
    {"??", TokenKind::QuestionQuestion},
    {"=>", TokenKind::Arrow},
    {"->", TokenKind::ThinArrow},
    {"::", TokenKind::ColonColon},
    {"**", TokenKind::StarStar},
    {"++", TokenKind::PlusPlus},
    {"--", TokenKind::MinusMinus},
    {"+=", TokenKind::PlusEqual},
    {"-=", TokenKind::MinusEqual},
    {"*=", TokenKind::StarEqual},
    {"/=", TokenKind::SlashEqual},
    {"%=", TokenKind::PercentEqual},
    {"&=", TokenKind::AmpersandEqual},
    {"|=", TokenKind::PipeEqual},
    {"^=", TokenKind::CaretEqual},
    {"==", TokenKind::EqualEqual},
    {"!=", TokenKind::BangEqual},
    {"<=", TokenKind::LessEqual},
    {">=", TokenKind::GreaterEqual},
    {"&&", TokenKind::AmpAmp},
    {"||", TokenKind::PipePipe},
    {"<<", TokenKind::LessLess},
    {">>", TokenKind::GreaterGreater},
    {"..", TokenKind::DotDot},

    {"(", TokenKind::LeftParen},
    {")", TokenKind::RightParen},
    {"{", TokenKind::LeftBrace},
    {"}", TokenKind::RightBrace},
    {"[", TokenKind::LeftBracket},
    {"]", TokenKind::RightBracket},
    {";", TokenKind::Semicolon},
    {",", TokenKind::Comma},
    {".", TokenKind::Dot},
    {":", TokenKind::Colon},
    {"?", TokenKind::Question},
    {"@", TokenKind::At},
    {"#", TokenKind::Hash},
    {"+", TokenKind::Plus},
    {"-", TokenKind::Minus},
    {"*", TokenKind::Star},
    {"/", TokenKind::Slash},
    {"%", TokenKind::Percent},
    {"=", TokenKind::Equal},
    {"<", TokenKind::Less},
    {">", TokenKind::Greater},
    {"!", TokenKind::Bang},
    {"&", TokenKind::Ampersand},
    {"|", TokenKind::Pipe},
    {"^", TokenKind::Caret},
    {"~", TokenKind::Tilde},
};

const std::unordered_set<std::string> kIntSuffixes = {
    "i8", "i16", "i32", "i64", "i128", "isize",
    "u8", "u16", "u32", "u64", "u128", "usize"};

const std::unordered_set<std::string> kFloatSuffixes = {"f32", "f64"};

} // namespace
Lexer::Lexer(std::shared_ptr<std::string> file)
    : file(file), _startPos(1, 1, 0, file), _pos(_startPos) {}

bool Lexer::atEnd() const { return _i >= _src.size(); }

char Lexer::peek(size_t n) const {
  return _i + n < _src.size() ? _src[_i + n] : '\0';
}

char Lexer::advance() {
  char c = _src[_i++];
  _pos = _pos.advance(c);
  return c;
}

char32_t Lexer::readCodePoint() {
  unsigned char c = static_cast<unsigned char>(advance());
  if (c < 0x80)
    return c;

  int extra = (c >> 5) == 0x6    ? 1
              : (c >> 4) == 0xE  ? 2
              : (c >> 3) == 0x1E ? 3
                                 : 0;
  if (extra == 0)
    return 0xFFFD;

  char32_t cp = extra == 1 ? (c & 0x1F) : extra == 2 ? (c & 0x0F) : (c & 0x07);
  for (int k = 0; k < extra && !atEnd() &&
                  (static_cast<unsigned char>(peek()) & 0xC0) == 0x80;
       k++) {
    cp = (cp << 6) | (static_cast<unsigned char>(advance()) & 0x3F);
  }
  return cp;
}

std::string Lexer::lexemeFrom(size_t startIdx) const {
  return _src.substr(startIdx, _i - startIdx);
}

TokenPtr Lexer::make(TokenKind kind, const Position &start, size_t startIdx) {
  return std::make_unique<Token>(kind, lexemeFrom(startIdx),
                                 Range(start, _pos));
}

TokenPtr Lexer::invalid(const Position &start, size_t startIdx,
                        const std::string &msg) {
  error(start, msg);
  return make(TokenKind::Invalid, start, startIdx);
}

void Lexer::error(const Position &at, const std::string &msg) {
  errors.push_back(at.toString() + ": " + msg);
}

TokenList Lexer::readTokens(std::string src) {
  TokenList tokens;

  _src = std::move(src);
  _i = 0;
  _pos = _startPos;
  errors.clear();

  while (!atEnd()) {
    char c = peek();

    if (c == ' ' || c == '\t' || c == '\r') {
      advance();
      continue;
    }

    if (c == '\n') {
      Position start = _pos;
      size_t idx = _i;
      advance();
      if (keepTrivia)
        tokens.push_back(make(TokenKind::Newline, start, idx));
      continue;
    }

    if (c == '/' && peek(1) == '/') {
      Position start = _pos;
      size_t idx = _i;
      while (!atEnd() && peek() != '\n')
        advance();
      if (keepTrivia)
        tokens.push_back(make(TokenKind::LineComment, start, idx));
      continue;
    }

    if (c == '/' && peek(1) == '*') {
      Position start = _pos;
      size_t idx = _i;
      advance();
      advance();
      bool closed = false;
      while (!atEnd()) {
        if (peek() == '*' && peek(1) == '/') {
          advance();
          advance();
          closed = true;
          break;
        }
        advance();
      }
      if (!closed)
        error(start, "unterminated block comment");
      if (keepTrivia)
        tokens.push_back(make(TokenKind::BlockComment, start, idx));
      continue;
    }

    if (isDigit(c)) {
      tokens.push_back(readNumber());
    } else if (isIdentStart(c)) {
      tokens.push_back(readIdentifier());
    } else if (c == '"') {
      tokens.push_back(readString());
    } else if (c == '\'') {
      tokens.push_back(readChar());
    } else if (auto op = readOperator()) {
      tokens.push_back(std::move(op));
    } else {
      Position start = _pos;
      size_t idx = _i;
      readCodePoint();
      tokens.push_back(invalid(
          start, idx, "unexpected character '" + lexemeFrom(idx) + "'"));
    }
  }

  tokens.push_back(
      std::make_unique<Token>(TokenKind::EndOfFile, "", Range(_pos, _pos)));
  return tokens;
}

TokenPtr Lexer::readIdentifier() {
  Position start = _pos;
  size_t idx = _i;
  while (!atEnd() && isIdentPart(peek()))
    advance();

  std::string text = lexemeFrom(idx);
  auto it = kKeywords.find(text);
  if (it != kKeywords.end())
    return make(it->second, start, idx);

  return std::make_unique<IdentifierToken>(std::move(text), Range(start, _pos));
}

TokenPtr Lexer::readNumber() {
  Position start = _pos;
  size_t idx = _i;

  auto fail = [&](const std::string &msg) {
    while (isIdentPart(peek()))
      advance();
    return invalid(start, idx, msg);
  };

  int base = 10;
  if (peek() == '0') {
    char n = peek(1);
    if (n == 'x' || n == 'X')
      base = 16;
    else if (n == 'b' || n == 'B')
      base = 2;
    else if (n == 'o' || n == 'O')
      base = 8;
    if (base != 10) {
      advance();
      advance();
    }
  }

  auto isBaseDigit = [&](char c) {
    switch (base) {
    case 16:
      return isHexDigit(c);
    case 2:
      return c == '0' || c == '1';
    case 8:
      return c >= '0' && c <= '7';
    default:
      return isDigit(c);
    }
  };

  std::string digits;
  while (isBaseDigit(peek()) || peek() == '_') {
    char c = advance();
    if (c != '_')
      digits += c;
  }

  if (base != 10) {
    if (digits.empty())
      return fail("missing digits after base prefix");
    if (isDigit(peek()))
      return fail("invalid digit for this base");
  }

  bool isFloat = false;
  if (base == 10) {
    if (peek() == '.' && isDigit(peek(1))) {
      isFloat = true;
      digits += advance();
      while (isDigit(peek()) || peek() == '_') {
        char c = advance();
        if (c != '_')
          digits += c;
      }
    }
    if ((peek() == 'e' || peek() == 'E') &&
        (isDigit(peek(1)) ||
         ((peek(1) == '+' || peek(1) == '-') && isDigit(peek(2))))) {
      isFloat = true;
      digits += advance();
      if (peek() == '+' || peek() == '-')
        digits += advance();
      while (isDigit(peek()) || peek() == '_') {
        char c = advance();
        if (c != '_')
          digits += c;
      }
    }
  }

  std::string suffix;
  if (isIdentStart(peek())) {
    while (isIdentPart(peek()))
      suffix += advance();
  }

  bool floatSuffix = !suffix.empty() && suffix[0] == 'f';

  if (isFloat || floatSuffix) {
    if (base != 10)
      return fail("float suffix on a non-decimal literal");
    if (!suffix.empty() && !kFloatSuffixes.count(suffix))
      return invalid(start, idx, "invalid float suffix '" + suffix + "'");
    double value = std::strtod(digits.c_str(), nullptr);
    return std::make_unique<FloatToken>(value, suffix, lexemeFrom(idx),
                                        Range(start, _pos));
  }

  if (!suffix.empty() && !kIntSuffixes.count(suffix))
    return invalid(start, idx, "invalid integer suffix '" + suffix + "'");

  uint64_t value = 0;
  for (char c : digits) {
    uint64_t d = static_cast<uint64_t>(hexValue(c));
    if (value > (UINT64_MAX - d) / static_cast<uint64_t>(base))
      return invalid(start, idx, "integer literal too large (max 64 bits)");
    value = value * static_cast<uint64_t>(base) + d;
  }

  return std::make_unique<IntegerToken>(value, base, suffix, lexemeFrom(idx),
                                        Range(start, _pos));
}

bool Lexer::readEscape(char32_t &out) {
  Position start = _pos;
  advance();
  if (atEnd()) {
    error(start, "unterminated escape sequence");
    return false;
  }

  char e = advance();
  switch (e) {
  case 'n':
    out = '\n';
    return true;
  case 't':
    out = '\t';
    return true;
  case 'r':
    out = '\r';
    return true;
  case '0':
    out = '\0';
    return true;
  case '\\':
  case '"':
  case '\'':
    out = static_cast<unsigned char>(e);
    return true;

  case 'x': {
    if (!isHexDigit(peek()) || !isHexDigit(peek(1))) {
      error(start, "invalid \\x escape, expected two hex digits");
      return false;
    }
    int hi = hexValue(advance());
    int lo = hexValue(advance());
    out = static_cast<char32_t>(hi * 16 + lo);
    if (out > 0x7F) {
      error(start, "\\x escape must be <= 0x7F");
      return false;
    }
    return true;
  }

  case 'u': {
    if (peek() != '{') {
      error(start, "invalid \\u escape, expected '{'");
      return false;
    }
    advance();
    uint32_t v = 0;
    int n = 0;
    while (isHexDigit(peek()) && n < 6) {
      v = v * 16 + static_cast<uint32_t>(hexValue(advance()));
      n++;
    }
    if (n == 0 || peek() != '}') {
      error(start, "invalid \\u escape, expected 1-6 hex digits and '}'");
      return false;
    }
    advance();
    if (v > 0x10FFFF || (v >= 0xD800 && v <= 0xDFFF)) {
      error(start, "invalid Unicode code point");
      return false;
    }
    out = v;
    return true;
  }

  default:
    error(start, std::string("unknown escape sequence '\\") + e + "'");
    return false;
  }
}

TokenPtr Lexer::readString() {
  Position start = _pos;
  size_t idx = _i;
  advance();

  std::string value;
  bool closed = false;
  bool ok = true;

  while (!atEnd()) {
    char c = peek();
    if (c == '"') {
      advance();
      closed = true;
      break;
    }
    if (c == '\n')
      break;
    if (c == '\\') {
      char32_t cp;
      if (readEscape(cp))
        appendUtf8(value, cp);
      else
        ok = false;
    } else {
      value += advance();
    }
  }

  if (!closed)
    return invalid(start, idx, "unterminated string literal");
  if (!ok)
    return make(TokenKind::Invalid, start, idx);

  return std::make_unique<StringToken>(std::move(value), lexemeFrom(idx),
                                       Range(start, _pos));
}

TokenPtr Lexer::readChar() {
  Position start = _pos;
  size_t idx = _i;
  advance();

  if (atEnd() || peek() == '\n')
    return invalid(start, idx, "unterminated char literal");
  if (peek() == '\'') {
    advance();
    return invalid(start, idx, "empty char literal");
  }

  char32_t value = 0;
  bool ok = true;
  if (peek() == '\\')
    ok = readEscape(value);
  else
    value = readCodePoint();

  if (peek() == '\'') {
    advance();
  } else {
    error(start, "char literal must contain exactly one character");
    while (!atEnd() && peek() != '\'' && peek() != '\n')
      advance();
    if (peek() == '\'')
      advance();
    ok = false;
  }

  if (!ok)
    return make(TokenKind::Invalid, start, idx);

  return std::make_unique<CharToken>(value, lexemeFrom(idx),
                                     Range(start, _pos));
}

TokenPtr Lexer::readOperator() {
  for (const OpEntry &op : kOperators) {
    size_t len = std::strlen(op.text);
    if (_src.compare(_i, len, op.text) != 0)
      continue;

    if (op.kind == TokenKind::QuestionDot && isDigit(peek(2)))
      continue;

    Position start = _pos;
    size_t idx = _i;
    for (size_t k = 0; k < len; k++)
      advance();
    return make(op.kind, start, idx);
  }
  return nullptr;
}
