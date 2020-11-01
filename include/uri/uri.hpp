#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Uri {

    class Uri
    {
    public:
        Uri();
        ~Uri();
        bool ParseFromString(std::string string);
        const std::string& GetScheme() const;
        const std::string& GetHost() const;
        const std::vector<std::string>& GetPath() const;
        const std::string& GetQuery() const;
        const std::string& GetFragment() const;
        bool HasPort() const;
        uint16_t GetPort() const;

    protected:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    };

}

