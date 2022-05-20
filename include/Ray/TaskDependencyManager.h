#pragma once
#include "RayBaseDefination.h"
#include <type_traits>

namespace Ray{
    class TaskDependencyManager
    {
    public:
        /*搜集任务参数中的输入对象依赖*/
        static void gatherInputDependency(std::vector<ObjectID>& dependencyList){}

        template<typename FirstArgType, typename...RestArgs>
        static typename std::enable_if<std::is_same<std::remove_cv_t<std::remove_reference_t<FirstArgType>>, ObjectID>::value>::type
        gatherInputDependency(std::vector<ObjectID>& dependencyList,
            FirstArgType&& first,
            RestArgs&&...restArgs)
        {
            dependencyList.push_back(first);
            gatherInputDependency(dependencyList, std::forward<RestArgs>(restArgs)...);
        }

        template<typename FirstArgType, typename...RestArgs>
        static typename std::enable_if<std::is_same<std::remove_cv_t<std::remove_reference_t<FirstArgType>>, ObjectID>::value == false>::type
        gatherInputDependency(std::vector<ObjectID>& dependencyList,
            FirstArgType&& first,
            RestArgs&&...restArgs)
        {
            gatherInputDependency(dependencyList, std::forward<RestArgs>(restArgs)...);
        }

        /*启动依赖解析*/
        static void execWhenDependencyReady(ObjectTableManager* objTableManager, const std::vector<ObjectID>& dependencyList, std::function<void()> whenReady);
    };

}