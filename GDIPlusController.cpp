#include "stdafx.h"
#include "GDIPlusController.h"


GDIPlusController::GDIPlusController()
{
    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}


GDIPlusController::~GDIPlusController()
{
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

ULONG_PTR GDIPlusController::Token()
{
    return gdiplusToken;
}
