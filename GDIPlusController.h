#pragma once
class GDIPlusController
{
public:
    GDIPlusController();
    ~GDIPlusController();

    ULONG_PTR Token();
protected:
    ULONG_PTR gdiplusToken = 0;
};

