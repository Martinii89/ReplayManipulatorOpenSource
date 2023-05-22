#pragma once

class CarRotator
{
public:
    static void RotateCar(CarWrapper& car, float rotation_degrees);
    static void RotateCarOfPri(PriWrapper& pri, float rotation_degrees);
    static void SpinCar(CarWrapper& car, float speed, float dt);
};
