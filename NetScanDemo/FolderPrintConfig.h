#pragma once

#include <locale>
#include <sstream>
#include <string>

// A folder config is a flat JSON object with optional numeric print overrides.
// Keep it independent of SDK structs so loading never changes live settings.
struct FolderPrintConfig
{
    bool hasXDpi;
    bool hasYDpi;
    bool hasInkRatio;
    float xdpi;
    long ydpi;
    unsigned int inkRatio;

    FolderPrintConfig()
        : hasXDpi(false), hasYDpi(false), hasInkRatio(false),
          xdpi(0), ydpi(0), inkRatio(0) {}

    bool HasOverrides() const { return hasXDpi || hasYDpi || hasInkRatio; }
};

class FolderPrintConfigParser
{
public:
    static bool Parse(const std::string& text, FolderPrintConfig& result,
                      std::string& error)
    {
        FolderPrintConfigParser parser(text);
        FolderPrintConfig parsed;
        if (!parser.ReadObject(parsed))
        {
            error = parser.m_error;
            return false;
        }
        result = parsed;
        error.clear();
        return true;
    }

private:
    const std::string& m_text;
    size_t m_pos;
    std::string m_error;

    explicit FolderPrintConfigParser(const std::string& text) : m_text(text), m_pos(0)
    {
        if (text.compare(0, 3, "\xEF\xBB\xBF") == 0)
            m_pos = 3;
    }

    bool Fail(const char* message) { m_error = message; return false; }
    void WhiteSpace()
    {
        while (m_pos < m_text.size() && (m_text[m_pos] == ' ' ||
               m_text[m_pos] == '\t' || m_text[m_pos] == '\r' || m_text[m_pos] == '\n'))
            ++m_pos;
    }
    bool Take(char ch)
    {
        WhiteSpace();
        if (m_pos == m_text.size() || m_text[m_pos] != ch)
            return false;
        ++m_pos;
        return true;
    }
    bool Digit() const
    {
        return m_pos < m_text.size() && m_text[m_pos] >= '0' && m_text[m_pos] <= '9';
    }
    // Validate integral fields using their exact decimal representation, not
    // a rounded double (e.g. 8.0000000000000001 must not become inkRatio=8).
    static bool PositiveInteger(const std::string& literal, unsigned long maximum,
                                unsigned long& result)
    {
        if (literal.empty() || literal[0] == '-') return false;
        std::string digits;
        long long fractionDigits = 0;
        bool fraction = false;
        size_t pos = 0;
        for (; pos < literal.size() && literal[pos] != 'e' && literal[pos] != 'E'; ++pos)
        {
            if (literal[pos] == '.') { fraction = true; continue; }
            digits += literal[pos];
            if (fraction) ++fractionDigits;
        }
        long long exponent = 0;
        const long long limit = static_cast<long long>(literal.size()) + 16;
        if (pos < literal.size())
        {
            ++pos;
            bool negative = false;
            if (literal[pos] == '+' || literal[pos] == '-')
                negative = literal[pos++] == '-';
            for (; pos < literal.size(); ++pos)
            {
                const int digit = literal[pos] - '0';
                if (exponent > (limit - digit) / 10) exponent = limit;
                else exponent = exponent * 10 + digit;
            }
            if (negative) exponent = -exponent;
        }
        const size_t first = digits.find_first_not_of('0');
        if (first == std::string::npos) return false;
        digits.erase(0, first);
        const long long shift = exponent - fractionDigits;
        if (shift < 0)
        {
            const size_t remove = static_cast<size_t>(-shift);
            if (remove >= digits.size()) return false;
            for (size_t i = digits.size() - remove; i < digits.size(); ++i)
                if (digits[i] != '0') return false;
            digits.resize(digits.size() - remove);
        }
        else
        {
            if (shift > 10 || digits.size() + static_cast<size_t>(shift) > 10) return false;
            digits.append(static_cast<size_t>(shift), '0');
        }
        result = 0;
        for (size_t i = 0; i < digits.size(); ++i)
        {
            const unsigned long digit = digits[i] - '0';
            if (digit > maximum || result > (maximum - digit) / 10) return false;
            result = result * 10 + digit;
        }
        return result > 0;
    }
    bool Number(double& value, std::string& literal)
    {
        WhiteSpace();
        const size_t start = m_pos;
        if (m_pos < m_text.size() && m_text[m_pos] == '-') ++m_pos;
        if (!Digit()) return Fail("Expected a JSON number.");
        if (m_text[m_pos] == '0') ++m_pos;
        else while (Digit()) ++m_pos;
        if (m_pos < m_text.size() && m_text[m_pos] == '.')
        {
            ++m_pos;
            if (!Digit()) return Fail("Missing digits after decimal point.");
            while (Digit()) ++m_pos;
        }
        if (m_pos < m_text.size() && (m_text[m_pos] == 'e' || m_text[m_pos] == 'E'))
        {
            ++m_pos;
            if (m_pos < m_text.size() && (m_text[m_pos] == '+' || m_text[m_pos] == '-'))
                ++m_pos;
            if (!Digit()) return Fail("Missing exponent digits.");
            while (Digit()) ++m_pos;
        }
        literal = m_text.substr(start, m_pos - start);
        std::istringstream stream(literal);
        stream.imbue(std::locale::classic());
        stream >> value;
        return !stream.fail() || Fail("Number is out of range.");
    }
    bool ReadObject(FolderPrintConfig& config)
    {
        if (!Take('{')) return Fail("Expected a JSON object.");
        if (!Take('}'))
        {
            for (;;)
            {
                if (!Take('"')) return Fail("Expected a quoted setting name.");
                const size_t start = m_pos;
                while (m_pos < m_text.size() && m_text[m_pos] != '"')
                {
                    const unsigned char ch = static_cast<unsigned char>(m_text[m_pos]);
                    if (ch < 0x20 || ch == '\\')
                        return Fail("Use literal xdpi, ydpi and inkRatio setting names.");
                    ++m_pos;
                }
                if (m_pos == m_text.size()) return Fail("Unterminated setting name.");
                const std::string key = m_text.substr(start, m_pos - start);
                ++m_pos;
                if (!Take(':')) return Fail("Expected ':' after setting name.");
                double value = 0;
                std::string literal;
                if (!Number(value, literal)) return false;
                if (key == "xdpi")
                {
                    if (config.hasXDpi) return Fail("Duplicate xdpi setting.");
                    if (!(value > 0 && value <= 2147483647.0))
                        return Fail("xdpi must be positive and <= 2147483647.");
                    config.xdpi = static_cast<float>(value);
                    if (!(config.xdpi > 0 && static_cast<double>(config.xdpi) <= 2147483647.0))
                        return Fail("xdpi cannot be represented within the supported range.");
                    config.hasXDpi = true;
                }
                else if (key == "ydpi")
                {
                    if (config.hasYDpi) return Fail("Duplicate ydpi setting.");
                    unsigned long exact = 0;
                    if (!PositiveInteger(literal, 2147483647UL, exact))
                        return Fail("ydpi must be an integer in [1, 2147483647].");
                    config.ydpi = static_cast<long>(exact);
                    config.hasYDpi = true;
                }
                else if (key == "inkRatio")
                {
                    if (config.hasInkRatio) return Fail("Duplicate inkRatio setting.");
                    unsigned long exact = 0;
                    if (!PositiveInteger(literal, 8UL, exact))
                        return Fail("inkRatio must be an integer in [1, 8].");
                    config.inkRatio = static_cast<unsigned int>(exact);
                    config.hasInkRatio = true;
                }
                else return Fail("Only xdpi, ydpi and inkRatio settings are supported.");
                if (Take('}')) break;
                if (!Take(',')) return Fail("Expected ',' or '}'.");
            }
        }
        WhiteSpace();
        return m_pos == m_text.size() || Fail("Unexpected data after JSON object.");
    }
};
