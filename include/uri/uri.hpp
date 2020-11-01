#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Uri {

    /**Represents a URI.
     * To read more about the standard this implements, read the [RFC](https://tools.ietf.org/html/rfc3986)
     */
    class Uri
    {
    public:
        Uri();
        ~Uri();

        /**Parses the URI object from a URI string.
         * @param[in]
         *      The string to parse.
         * @returns
         *      True if the URI was valid and false otherwise.
         */
        bool ParseFromString(std::string string);

        const std::string& GetScheme() const;

        const std::string& GetUserInfo() const;

        const std::string& GetHost() const;

        /**If the start of the path is an empty string (""), that means that
         * the path is absolute, and the rest of the data in the vector will be
         * the path data.
         */
        const std::vector<std::string>& GetPath() const;

        const std::string& GetQuery() const;

        const std::string& GetFragment() const;

        /**Whether or not the URI supplies a port, as this is optional. */
        bool HasPort() const;

        /**@note
         *      The port returned by this method will only be valid if and only
         *      if the HasPort method returns true.
         */
        uint16_t GetPort() const;

        /**Returns true for a URI with a relative path (only valid for URIs
         * which have a path in the first place).
         */
        bool ContainsRelativePath() const;

        std::string ConstructString() const;

        void SetScheme(const std::string& scheme);

        void SetUserInfo(const std::string& userInfo);

        void SetHost(const std::string& host);

        void SetPath(const std::vector<std::string>& path);

        void SetQuery(const std::string& query);

        void SetFragment(const std::string& fragment);

        void SetHasPort(bool hasPort);

        void SetPort(uint16_t port);

    protected:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

