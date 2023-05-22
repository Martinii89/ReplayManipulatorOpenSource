#include "pch.h"
#include "CarRotator.h"

void CarRotator::RotateCar(CarWrapper& car, float rotation_degrees)
{
    auto rot = car.GetRotation();
    rot.Yaw += static_cast<int>(rotation_degrees * CONST_DegToUnrRot);
    car.SetRotation(rot);
}

void CarRotator::RotateCarOfPri(PriWrapper& pri, float rotation_degrees)
{
    if (!pri)
        return;

    if (auto car = pri.GetCar())
    {
        RotateCar(car, rotation_degrees);
    }
}

void CarRotator::SpinCar(CarWrapper& car, float speed, float dt)
{
    auto rot = car.GetRotation();
    rot.Yaw += static_cast<int>(speed * dt);
    car.SetRotation(rot);
}
