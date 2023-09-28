/*
 * Useful utility functions
*/

#include <memory>

// helper functions for error handling
template<typename T>
std::unique_ptr<T> log_error(const char *str)
{
    fprintf(stderr, "Error: %s\n", str);
    return nullptr;
}