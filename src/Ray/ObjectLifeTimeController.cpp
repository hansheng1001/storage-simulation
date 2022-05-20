#include"RayBaseDefination.h"
#include"RayAPI.h"

Ray::ObjectLifeTimeController::~ObjectLifeTimeController(){
    //service_->deleteObj(objKey_);
}

Ray::ObjectHandle Ray::ObjectLifeTimeController::getObj(Ray::ObjectID id)
{
    return service_->getObj(id);
}