#include "./jsonscanner.hh"

#include <utility/misc/string.hh>
#include <utility/debug/debug.hh>

#include <string>
#include <list>

namespace xtaro::parse
{
    std::list<JsonToken> JsonScanner::scan()
    {
        while (this->isEnd() == false)
            this->scanToken();
        this->addToken(JsonTokenType::ENDOFFILE);

        return std::move(this->_tokens);
    }

    void JsonScanner::scanToken()
    {
        this->_startIndex = this->_currentIndex;
        
        char ch = this->advanceChar();
        switch (ch)
        {
        // Empty character
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            break;
        case '{':
            this->addToken(JsonTokenType::LEFT_BRACE); break;
        case '}':
            this->addToken(JsonTokenType::RIGHT_BRACE); break;
        case '[':
            this->addToken(JsonTokenType::LEFT_BRACKET); break;
        case ']':
            this->addToken(JsonTokenType::RIGHT_BRACKET); break;
        case ',':
            this->addToken(JsonTokenType::COMMA); break;
        case ':':
            this->addToken(JsonTokenType::COLON); break;
        case 'n':
            this->scanNullToken(); break;
        case 't':
            this->scanTrueToken(); break;
        case 'f':
            this->scanFalseToken(); break;
        case '0': 
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            this->scanNumberToken(); break;
        case '"':
            this->scanStringToken(); break;
        default:
            debug::exceptionIn("Scan Json", std::format("Unkown character '{}'.", ch));
            break;
        }
    }

    void JsonScanner::scanNumberToken()
    {
        // Search until '.' or not digit
        while (true)
        {
            char ch = this->advanceChar();
            if (ch >= '0' && ch <= '9')
                continue;
            
            if (ch != '.')
            {
                this->_currentIndex -= 1;

                // Number is a integer
                int value = std::atoi( this->startPointer() );
                return this->addToken(value);
            }
            else break;
        }

        // Search until not digit
        while (true)
        {
            char ch = this->advanceChar();
            if (ch >= '0' && ch <= '9')
                continue;
            this->_currentIndex -= 1;

            double value = std::atof( this->startPointer() );
            return this->addToken(value);
        }
    }

    void JsonScanner::scanStringToken()
    {
        // Search util '"'
        while ( this->advanceChar() != '"') {}

        this->_startIndex += 1;
        std::string value(this->startPointer(), 
                          this->_currentIndex - this->_startIndex - 1);
        this->addToken(std::move(value));
    }

    void JsonScanner::scanNullToken()
    {
        if (this->advanceChar() != 'u' || this->advanceChar() != 'l' || this->advanceChar() != 'l')
            debug::exceptionfIn("Scan json", "Except 'null'");
        this->addToken(JsonTokenType::NONE);
    }

    void JsonScanner::scanTrueToken()
    {
        if (this->advanceChar() != 'r' || this->advanceChar() != 'u' || this->advanceChar() != 'e')
            debug::exceptionfIn("Scan json", "Except 'true'");
        this->addToken(true);
    }

    void JsonScanner::scanFalseToken()
    {
        if (this->advanceChar() != 'a' || this->advanceChar() != 'l' || this->advanceChar() != 's' || this->advanceChar() != 'e')
            debug::exceptionfIn("Scan json", "Except 'false'");
        this->addToken(false);
    }

    char JsonScanner::advanceChar()
    {
        if (this->isEnd())
            debug::exceptionfIn("Scan json", "File ended prematurely");
        char ch = this->peekChar();
        this->_currentIndex += 1;
        return ch;
    }
}