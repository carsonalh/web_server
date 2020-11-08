/**
 * @file web/uri.hpp
 * Defines the logic for parsing and constructing URIs as defined by [the
 * RFC](https://tools.ietf.org/html/rfc3986).
 */
#pragma once

#include "./text.hpp"

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <memory>

namespace uri {

    /**
     * Represents a URI.
     *
     * Can both parse and construct URIs, as the data are stored in one format.
     *
     * To read about the standard this implements, look at the
     * [RFC](https://tools.ietf.org/html/rfc3986).
     */
    class Uri
    {
    public:
        /**
         * Checks if the given string is a valid IPv4 string or not.
         *
         * @returns
         *      True if the string matches the IPv4 format, and all the given
         *      numbers are in range. False otherwise.
         */
        static bool isIpv4String(const std::string& string);

        /**
         * Checks if the given string is a valid IPv6 string or not.
         *
         * @returns
         *      True if the string matches the IPv6 format, and all the given
         *      numbers are in range. False otherwise.
         */
        static bool isIpv6String(const std::string& string);

        /**
         * @brief
         *     Gets a string that is not percent encoded, and replaces all the
         *     non-unreserved characters with their percent-encoded
         *     counterparts.
         *
         * If the string is already percent encoded, this function will not
         * detect this, and will instead percent-encoded the parts that have
         * already been encoded, so it is up to the user of this function to
         * manage what has and has not already been encoded.
         *
         * @param[in] string
         * @param[in] allowedCharacters
         *      A CharacterSet of all of the characters that do not have to be
         *      percent-encoded.
         */
        static std::string percentEncode(std::string_view string, const text::CharacterSet& allowedCharacters = text::UNRESERVED_CHARACTERS);

        /**
         * Decodes a percent-encoded string, and returns the decoded string.
         */
        static std::string percentDecode(std::string_view string);

    public:
        Uri();
        ~Uri();

        /**
         * @brief Parses the URI object from a URI string.
         *
         * @param[in] string
         *      The string to parse.
         * @returns
         *      True if the URI was valid and false otherwise.
         */
        bool parseFromString(std::string string);

        /**
         * @brief Gets the scheme of the URI.
         *
         * This is typcially something like `http` or `ftp`: the part before
         * the colon (`:`) if it has one.
         */
        const std::string& scheme() const;

        /**
         * @brief Gets the "user info" part of the URI.
         *
         * In a URI such as `https://john.doe:password@gmail.com`, the user
         * info part would be the `john.doe:password`. This will come as one
         * string, so the user must parse it himself.
         */
        const std::string& userInfo() const;

        /**
         * @brief Gets the "host" of the URI.
         */
        const std::string& host() const;

        /**
         * If the start of the path is an empty string (""), that means that
         * the path is absolute, and the rest of the data in the vector will be
         * the path data.
         */
        const std::vector<std::string>& path() const;

        /**
         * @brief Gets the "query" part of the URI, which will always be
         * prefixed with a question mark (`?`) character.
         */
        const std::string& query() const;

        /**
         * @brief Gets the "fragment" part of the URI, which will always be
         * prefixed with a pound sign (`#`) character.
         */
        const std::string& fragment() const;

        /**
         * @brief Whether or not the URI supplies a port, as this is optional.
         */
        bool hasPort() const;

        /**
         * @brief Returns the URI's port.
         *
         * @note
         *      The port returned by this method will only be valid if and only
         *      if the Uri::hasPort method returns true.
         */
        uint16_t port() const;

        /**
         * @brief Returns true for a URI with a relative path (only valid for
         * URIs which have a path in the first place).
         */
        bool containsRelativePath() const;

        std::string constructString() const;

        /**
         * @brief Merges the path currently held by the uri object with another
         * path.
         *
         * If the other path is relative, it will be resolved like any other
         * path resolution, but if it is absolute, the other path will
         * completely replace this uri's path.
         *
         * @param[in] otherPath
         *      The path to use to resolve this uri's path. Uses the same
         *      conventions as the "path" property of this uri class, with an
         *      exception: the values "." and ".." have the same meaning that
         *      they would in a normal file system; "." meaning to stay in the
         *      same directory, and ".." meaning to go up one level.
         */
        void resolvePath(const std::vector<std::string>& otherPath);

        /**
         * @brief Sets the scheme of the URI.
         *
         * This is typcially something like `http` or `ftp`: the part before
         * the colon (`:`) if it has one.
         */
        void setScheme(const std::string& scheme);

        /**
         * @brief Sets the "user info" part of the URI.
         *
         * In a URI such as `https://john.doe:password@gmail.com`, the user
         * info part would be the `john.doe:password`. The user must construct
         * this string from its sub-components; this class will only insert the
         * entire thing into the constructed URI.
         */
        void setUserInfo(const std::string& userInfo);

        /** @brief Sets the "host" part of the URI. */
        void setHost(const std::string& host);

        /** @brief Sets the "path" part of the URI. */
        void setPath(const std::vector<std::string>& path);

        /**
         * @brief Sets the query as active, and gives it the passed value.
         */
        void setQuery(const std::string& query);

        /** @brief Sets the query as inactive. */
        void clearQuery();

        /**
         * @brief Sets the fragment as active, and gives it the passed value.
         */
        void setFragment(const std::string& fragment);

        /** @brief Sets the fragment as inactive. */
        void clearFragment();

        /**
         * @brief Sets whether or not the given port should be displayed when
         * the URI is constructed.
         */
        void setHasPort(bool hasPort);

        /** @brief Sets the port of the URI. */
        void setPort(uint16_t port);

    protected:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

