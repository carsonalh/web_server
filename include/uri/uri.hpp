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
        std::string GetScheme() const;
        std::string GetHost() const;
        const std::vector<std::string>& GetPath() const;
        std::string GetQuery() const;
        std::string GetFragment() const;
        bool HasPort() const;
        uint16_t GetPort() const;

    protected:
        struct Impl;
        Impl* m_Impl;

    };

}

