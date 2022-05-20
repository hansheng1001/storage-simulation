#pragma once
#include "RayBaseDefination.h"
#include <functional>

namespace Ray {
class ObjectTableManager;
class InputObjectReader {
public:
    static void execWhenAllInputObjectReady(ObjectTableManager*          objTableManager,
                                            const std::vector<ObjectID>& inputObjList, std::function<void()> whenReady);
};
}  // namespace Ray