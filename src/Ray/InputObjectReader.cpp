#include"InputObjectReader.h"
#include"ObjectTableManager.h"


void Ray::InputObjectReader::execWhenAllInputObjectReady(ObjectTableManager* objTableManager, const std::vector<ObjectID>& inputObjList, std::function<void()> whenReady){
    std::shared_ptr<size_t> size=std::make_shared<size_t>(inputObjList.size());
    if((*size)==0){
        whenReady();
        return;
    }
    auto call=[size,whenReady](){
        (*size)--;
        if((*size)==0){
            whenReady();
        }
    };
    auto end=inputObjList.end();
    for(auto begin=inputObjList.begin();begin!=end;begin++){
        objTableManager->waitUntilReady(*begin,call);
    }
}