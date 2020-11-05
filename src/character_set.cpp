#include "web/uri.hpp"

namespace uri {

    CharacterSet::CharacterSet(std::initializer_list<char> initializerList)
        : m_Characters(initializerList.begin(), initializerList.end())
    {
    }

    bool CharacterSet::contains(char c) const
    {
        return m_Characters.find(c) != m_Characters.end();
    }

    const CharacterSet UNRESERVED_CHARACTERS = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B',
        'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '-', '.', '_', '~',
    };

}

