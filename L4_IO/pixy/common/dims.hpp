#ifndef DIMS_HPP
#define DIMS_HPP

namespace team9
{
namespace pixy
{

struct Dims_t
{
    uint16_t usRows;
    uint16_t usCols;
    Dims_t(uint16_t usRows_arg = 0, uint16_t usCols_arg = 0)
    {
        std::tie(usRows, usCols) = std::tie(usRows_arg, usCols_arg);
    }
};

} // namespace pixy
} // namespace team9

#endif
