#ifndef EXCEPTIONS_00_H
#define EXCEPTIONS_00_H
#include <exception>
#include <string>
#include <utility>

/**
 * Base exception class containing a message.
 */
class base_exception : std::exception {
public:
    std::string msg;

    explicit base_exception(std::string msg) : msg(std::move(msg)) {};
};

/**
 * image_load_exception, for when loading an image fails.
 */
class image_load_exception : public base_exception {
public:
    explicit image_load_exception(std::string msg) : base_exception(std::move(msg)) {};

};

/**
 * logic_exception, an exception that indicates a programming error.
 */
class logic_exception : public base_exception {
public:
    explicit logic_exception(std::string msg) : base_exception(std::move(msg)) {};
};

/**
 * SDL_exception, for when loading a part of the SDL library fails.
 */
class SDL_exception : public base_exception {
public:
    explicit SDL_exception(std::string msg) : base_exception(std::move(msg)) {};

};

#endif