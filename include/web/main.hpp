/**
 * @file web/main.hpp
 * Defines the `main()` function so that client applications don't have to.
 */
#pragma once

#include <string>

namespace web {

    class Application;

    /**
     * The master function that runs the entire application.
     *
     * @param[in] app
     *      The application, which should be a subclass created by the user.
     * @param[in] argc
     *      Should be passed directly from `main()`.
     * @param[in] argv
     *      Should be passed directly from `main()`.
     * @note
     *      The application pointer belongs to the caller, and will not be
     *      deleted in the scope of this function.
     */
    int runApplication(Application& app, int argc, const char** argv);

    /**
     * @brief Server interface that any and all servers should implement in
     * this application.
     */
    class IServer
    {
    public:
        virtual ~IServer() = default;

        /**
         * To be overriden; the server should use this to process their
         * request, and then return their result. This function should not have
         * to talk to the client, as this is managed externally.
         */
        virtual std::string processRequest(const std::string& incoming) = 0;
    };

    /**
     * @brief Represents an entire application.
     */
    class Application
    {
    public:
        /**
         * @brief Pure virtual method that should return a constructed server
         * object.
         */
        virtual IServer& server() = 0;

    private:
        friend int runApplication(Application& app, int argc, const char** argv);

        /**
         * Runs the entire application and outputs a status code.
         */
        int run();

    };

}

