#ifndef FUNC_MAP_HPP
#define FUNC_MAP_HPP

namespace team9
{
namespace pixy
{

template<typename KEY_T, typename FUN_T, typename ... ARG_T>
struct FuncMap_t
{
    std::map<KEY_T, std::function<FUN_T(ARG_T ... xArgs)>> fpMap;

    void vSetHandler(KEY_T xElem,
                     std::function<FUN_T(ARG_T ... xArgs)> fnHandler)
    {
        fpMap[xElem] = fnHandler;
    }

    std::function<FUN_T(ARG_T ... xArgs)>& vResponse(KEY_T xElem)
    {
        return fpMap[xElem];
    }
};

} // namespace pixy
} // namespace team9

#endif
