#ifndef CLI_STYLE_HPP_
#define CLI_STYLE_HPP_

#include <string_view>

#include <string>
namespace style {

bool colorEnabled();

std::string_view red();
std::string_view green();
std::string_view yellow();
std::string_view bold();
std::string_view dim();
std::string_view reset();

std::string colored(std::string_view text, std::string_view color);
} // namespace style

#endif // CLI_STYLE_HPP_
