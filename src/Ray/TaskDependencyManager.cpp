#include"TaskDependencyManager.h"
#include"ObjectTableManager.h"

void Ray::TaskDependencyManager::execWhenDependencyReady(ObjectTableManager* objTableManager, 
                                                        const std::vector<ObjectID>& dependencyList, 
                                                        std::function<void()> whenReady)
{
    for(auto objID:dependencyList){
        objTableManager->waitUntilReadable(objID,whenReady);
    }
    
}