#pragma once

#include "CommandQueue.h"

///@brief we draw at hdc, image
/// wich left top corner is in [0..1, 0..1] coordinates of whole image.
/// and we draw only wight and height of whereInDC, in scale from paint function
typedef void(*TPainter)(const HDC hdc, const std::shared_ptr<Gdiplus::Bitmap>& _im,
    const Gdiplus::PointF origin, const RECT& whereInDC);

void Draw1vs1(const HDC hdc, const std::shared_ptr<Gdiplus::Bitmap>& _im,
    const Gdiplus::PointF origin, const RECT& whereInDC);

///@brief we are resizing src to target, in scale. Bicubic scaling is used
void ResizeBicubic(const std::shared_ptr<Gdiplus::Bitmap>& src, std::shared_ptr<Gdiplus::Bitmap>& target, const double scale);

// resize /ScaleProportion() - both directions
void DrawBicubic(const HDC hdc, const std::shared_ptr<Gdiplus::Bitmap>& _im,
    const Gdiplus::PointF origin, const RECT& whereInDC);

///@brief here we call the function to paint
class ThreadedDialog
{
public:
    ThreadedDialog(const TCHAR *const atitle, DialogsSync::CommandSequence<DialogsSync::Command>& cmdQueue, TPainter apainter = &Draw1vs1);
    ~ThreadedDialog() = default;

    ///@brief execute the thread and create dialog to draw images
    /// @return true if created and executed
    ///    false if already executed
    bool Execute();

    ///@brief Set image data for draw
    void SetImageData(std::shared_ptr<Gdiplus::Bitmap>& toSet);

    ///@brief set the Dialog to stop
    /// we just send the command to stop. Join must be done afterwards.
    void Stop();


    ///@brief stop the Dialog and thread inside
    void Join();

    ///@brief process command from main thread
    void OnCommand(DialogsSync::CommandSequence<DialogsSync::Command>::TCommand& pCmd) noexcept;

protected:
    BOOL EvCommand(WPARAM wParam, LPARAM lParam);
    BOOL EvMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    /// @brief: begin paint already has been done for this function
    /// end paint will be done after. Just paint inside
    void OnPaint(const HDC hdc, WPARAM& wParam, LPARAM& lParam);

    /// @brief: when user press close button
    void OnSysClose() const noexcept;

    friend void ThreadedDialogProcedure(ThreadedDialog* const p);
    friend BOOL CALLBACK DialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    std::unique_ptr<std::thread> _thread; // we are working within this thread

private:
    void InThread();
protected:

    bool _bEndAllowed = false; // set to true to allow close of the window
    HWND _hwnd = 0;
    std::basic_string<TCHAR> _title; // we are using the string as a source of data in the title for the window

    std::shared_ptr<Gdiplus::Bitmap> _imToDraw;
    Gdiplus::PointF _imageOrigin = {0, 0};// we are drawing the image from this point. So for image 10 on 10  we can draw 1 pixel from 9:9
    TPainter painter;
    DialogsSync::CommandSequence<DialogsSync::Command>& _cmdQueue;
};


//class Painter1vs1 : public ThreadedDialog<Painter1vs1>
//{
//public:
//    using ThreadedDialog::ThreadedDialog;
//
//    void Draw(const HDC hdc, WPARAM& wParam, LPARAM& lParam)
//    {
//        if (_imToDraw == nullptr)
//            return;
//
//        using namespace Gdiplus;
//        Gdiplus::Graphics graphics(hdc);
//        RECT rc;
//        ::GetClientRect(_hwnd, &rc);
//        // TODO adjuth all math with image positioning
//        const UINT v_width = _imToDraw->GetWidth();
//        const UINT v_height = _imToDraw->GetHeight();
//
//        Gdiplus::RectF destRect(0., 0., static_cast<REAL>(rc.right - rc.left), static_cast<REAL>(rc.bottom - rc.top));
//
//        graphics.DrawImage(_imToDraw.get(), destRect,
//            static_cast<REAL>(_imageOrigin.x), static_cast<REAL>(_imageOrigin.y),
//            destRect.Width, destRect.Height, Gdiplus::Unit::UnitPixel, nullptr);
//    }
//};
//
//class PainterBicubic : public ThreadedDialog<PainterBicubic>
//{
//public:
//    using ThreadedDialog::ThreadedDialog;
//
//    void Draw(const HDC hdc, WPARAM& wParam, LPARAM& lParam)
//    {
//        if (_imToDraw == nullptr)
//            return;
//
//        using namespace Gdiplus;
//        Gdiplus::Graphics graphics(hdc);
//        RECT rc;
//        ::GetClientRect(_hwnd, &rc);
//        // TODO adjuth all math with image positioning
//        const UINT v_width = _imToDraw->GetWidth();
//        const UINT v_height = _imToDraw->GetHeight();
//
//        Gdiplus::RectF destRect(0., 0., static_cast<REAL>(rc.right - rc.left), static_cast<REAL>(rc.bottom - rc.top));
//
//        graphics.DrawImage(_imToDraw.get(), destRect,
//            static_cast<REAL>(_imageOrigin.x), static_cast<REAL>(_imageOrigin.y),
//            destRect.Width, destRect.Height, Gdiplus::Unit::UnitPixel, nullptr);
//    }
//};
//class PainterLanzos : public ThreadedDialog<PainterLanzos>
//{
//public:
//    using ThreadedDialog::ThreadedDialog;
//
//    void Draw(const HDC hdc, WPARAM& wParam, LPARAM& lParam)
//    {
//        if (_imToDraw == nullptr)
//            return;
//
//        using namespace Gdiplus;
//        Gdiplus::Graphics graphics(hdc);
//        RECT rc;
//        ::GetClientRect(_hwnd, &rc);
//        // TODO adjuth all math with image positioning
//        const UINT v_width = _imToDraw->GetWidth();
//        const UINT v_height = _imToDraw->GetHeight();
//
//        Gdiplus::RectF destRect(0., 0., static_cast<REAL>(rc.right - rc.left), static_cast<REAL>(rc.bottom - rc.top));
//
//        graphics.DrawImage(_imToDraw.get(), destRect,
//            static_cast<REAL>(_imageOrigin.x), static_cast<REAL>(_imageOrigin.y),
//            destRect.Width, destRect.Height, Gdiplus::Unit::UnitPixel, nullptr);
//    }
//};
