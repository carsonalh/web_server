#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Uri {

    /**
     * Represents a URI.
     * To read more about the standard this implements, read the [RFC](https://tools.ietf.org/html/rfc3986)
     */
    class Uri
    {
    public:

        /**
         * Checks if the given string is an IPv4 string or not.
         * @returns
         *      True if the string matches the IPv4 format, and all the given
         *      numbers are in range. False otherwise.
         */
        static bool isIpv4String(const std::string& string);

        /**
         * Checks if the given string is an IPv6 string or not.
         * @returns
         *      True if the string matches the IPv6 format, and all the given
         *      numbers are in range. False otherwise.
         */
        static bool isIpv6String(const std::string& string);

    public:
        Uri();
        ~Uri();

        /**
         * Parses the URI object from a URI string.
         * @param[in]
         *      The string to parse.
         * @returns
         *      True if the URI was valid and false otherwise.
         */
        bool parseFromString(std::string string);

        const std::string& scheme() const;

        const std::string& userInfo() const;

        const std::string& host() const;

        /**
         * If the start of the path is an empty string (""), that means that
         * the path is absolute, and the rest of the data in the vector will be
         * the path data.
         */
        const std::vector<std::string>& path() const;

        const std::string& query() const;

        const std::string& fragment() const;

        /** Whether or not the URI supplies a port, as this is optional. */
        bool hasPort() const;

        /**
         * @note
         *      The port returned by this method will only be valid if and only
         *      if the hasPort method returns true.
         */
        uint16_t port() const;

        /**
         * Returns true for a URI with a relative path (only valid for URIs
         * which have a path in the first place).
         */
        bool containsRelativePath() const;

        std::string constructString() const;

        /**
         * Merges the path currently held by the uri object with another path.
         * If the other path is relative, it will be resolved like any other
         * path resolution, but if it is absolute, the other path will
         * completely replace this uri's path.
         * @param[in] otherPath
         *      The path to use to resolve this uri's path. Uses the same
         *      conventions as the "path" property of this uri class, with an
         *      exception: the values "." and ".." have the same meaning that
         *      they would in a normal file system; "." meaning to stay in the
         *      same directory, and ".." meaning to go up one level.
         */
        void resolvePath(const std::vector<std::string>& otherPath);

        void setScheme(const std::string& scheme);

        void setUserInfo(const std::string& userInfo);

        void setHost(const std::string& host);

        void setPath(const std::vector<std::string>& path);

        /** Sets the query as active, and gives it the passed value. */
        void setQuery(const std::string& query);

        /** Sets the query as inactive. */
        void clearQuery();

        /** Sets the fragment as active, and gives it the passed value. */
        void setFragment(const std::string& fragment);

        /** Sets the fragment as inactive. */
        void clearFragment();

        void setHasPort(bool hasPort);

        void setPort(uint16_t port);

    protected:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

