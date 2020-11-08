#pragma once

#include <initializer_list>
#include <unordered_set>

namespace text {

    /**
     * Represents a set of characters which is intended to be used primarily
     * for URIs and HTTP to match the specification, but can also be used for
     * other purposes.
     */
    class CharacterSet
    {
    public:
        CharacterSet(std::initializer_list<char> init);

        /**
         * Tells whether or not the set contains the given character.
         */
        bool contains(char c) const;

    protected:
        const std::unordered_set<char> m_Characters;

    };

    /**
     * All of the unreserved characters defined in the [URI
     * RFC](https://tools.ietf.org/html/rfc3986).
     */
    extern const CharacterSet UNRESERVED_CHARACTERS;

    /**
     * All of the unreserved characters defined in the [HTTP
     * RFC](https://tools.ietf.org/html/rfc2616).
     */
    extern const CharacterSet HTTP_HEADER_SEPARATORS;

}

