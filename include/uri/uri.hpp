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
         * 
         * @param[in]
         *      The string to parse.
         *
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

    protected:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

