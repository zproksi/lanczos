#include "stdafx.h"
#include "ThreadedDialog.h"
#include "resource.h"
#include <WinUser.h>

class CallAtExit
{
public:
    CallAtExit(std::function<void()> afun) :_fun(afun) {}
    ~CallAtExit()
    {
        _fun();
    }
    CallAtExit(const CallAtExit&) = delete;
    CallAtExit(CallAtExit&&) = delete;
    CallAtExit& operator =(const CallAtExit&) = delete;
    CallAtExit& operator =(CallAtExit&&) = delete;

protected:
    std::function<void()> _fun;
};



void Draw1vs1(const HDC hdc, const std::shared_ptr<Gdiplus::Bitmap>& _im, const Gdiplus::PointF origin, const RECT& whereInDC)
{
    if (_im == nullptr)
        return;

    using namespace Gdiplus;
    Graphics graphics(hdc);
    const RECT& rc = whereInDC;

    // TODO adjuth all math with image positioning
    const UINT v_width = _im->GetWidth();
    const UINT v_height = _im->GetHeight();

    RectF destRect(
        static_cast<REAL>(rc.left), static_cast<REAL>(rc.top),
        static_cast<REAL>(rc.right - rc.left), static_cast<REAL>(rc.bottom - rc.top));
    REAL atX = origin.X * static_cast<REAL>(v_width);
    if (atX + destRect.Width > v_width)
    {
        atX = v_width - destRect.Width;
    };
    if (atX < 0.)
    {
        atX = 0.;
    }
    REAL atY = origin.Y * static_cast<REAL>(v_height);
    if (atY + destRect.Height > v_height)
    {
        atY = v_height - destRect.Height;
    };
    if (atY < 0.)
    {
        atY = 0.;
    }

    graphics.DrawImage(_im.get(), destRect,
        atX, atY,
        destRect.Width, destRect.Height, Unit::UnitPixel, nullptr);

}

void ResizeBicubic(const std::shared_ptr<Gdiplus::Bitmap>& src, std::shared_ptr<Gdiplus::Bitmap>& target, const double scale)
{
    const SIZE& szTarget{static_cast<LONG>(static_cast<double>(src->GetWidth()) / scale),
        static_cast<LONG>(static_cast<double>(src->GetHeight()) / scale)};

    target = std::make_unique<Gdiplus::Bitmap>(static_cast<int>(szTarget.cx), static_cast<int>(szTarget.cy), src->GetPixelFormat());

    Gdiplus::Graphics graphics(target.get());
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.DrawImage(src.get(), 0, 0, static_cast<INT>(szTarget.cx), static_cast<INT>(szTarget.cy));
}


void DrawBicubic(const HDC hdc, const std::shared_ptr<Gdiplus::Bitmap>& _im, const Gdiplus::PointF origin, const RECT& whereInDC)
{
    if (_im == nullptr)
        return;

    using namespace Gdiplus;
    Graphics graphics(hdc);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    const RECT& rc = whereInDC;

    // TODO adjuth all math with image positioning
    const UINT v_width = _im->GetWidth();
    const UINT v_height = _im->GetHeight();

    RectF destRect(
        static_cast<REAL>(rc.left), static_cast<REAL>(rc.top),
        static_cast<REAL>(rc.right - rc.left), static_cast<REAL>(rc.bottom - rc.top));
    REAL atX = origin.X * static_cast<REAL>(v_width);
    if (atX + destRect.Width * ScaleProportion() > v_width)
    {
        atX = v_width - destRect.Width * ScaleProportion();
    };
    if (atX < 0.)
    {
        atX = 0.;
    }
    REAL atY = origin.Y * static_cast<REAL>(v_height);
    if (atY + destRect.Height * ScaleProportion() > v_height)
    {
        atY = v_height - destRect.Height * ScaleProportion();
    };
    if (atY < 0.)
    {
        atY = 0.;
    }

    std::cout << "bx:" << atX << "by:" << atY << std::endl;

    graphics.DrawImage(_im.get(), destRect,
        atX, atY,
        destRect.Width * ScaleProportion(), destRect.Height * ScaleProportion(),
        Unit::UnitPixel, nullptr);
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ThreadedDialog* const d = (ThreadedDialog*)::GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC paintDC = ::BeginPaint(hwndDlg, &ps);
        d->OnPaint(paintDC, wParam, lParam);
        ::EndPaint(hwndDlg, &ps);
        return 0;
    }

    case WM_INITDIALOG:
    {
        ::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

        ThreadedDialog* const d = reinterpret_cast<ThreadedDialog*>(lParam);

        if (d)
        {
            d->_hwnd = hwndDlg;
        }
        ::SetWindowText(hwndDlg, d->_title.c_str());
        ::ShowWindow(hwndDlg, SW_SHOW);
        ::UpdateWindow(hwndDlg);

        return TRUE;
    }

    case WM_CLOSE:
        if (d && d->_bEndAllowed)
        {
            ::EndDialog(d->_hwnd, IDOK);
        };

        return TRUE;
    case WM_SYSCOMMAND:
    {
        if (d && wParam == SC_CLOSE)
        {
            d->OnSysClose();
        };
        return FALSE;
    }

    case WM_COMMAND:

        if (d)
            return d->EvCommand(wParam, lParam);

        return TRUE;

    default:
        return d ? d->EvMessage(msg, wParam, lParam) : FALSE;
    };
};




ThreadedDialog::ThreadedDialog(const TCHAR *const atitle, DialogsSync::CommandSequence<DialogsSync::Command>& cmdQueue, TPainter apainter)
    :_title(atitle)
    , painter(apainter)
    , _cmdQueue(cmdQueue)

{
}



// thread specific
void ThreadedDialogProcedure(ThreadedDialog* const p)
{
    p->InThread();
}

bool ThreadedDialog::Execute()
{
    _thread = std::make_unique<std::thread>([this]()
    {
        ThreadedDialogProcedure(this);
    }

    );

    return false;
}

void ThreadedDialog::SetImageData(std::shared_ptr<Gdiplus::Bitmap>& toSet)
{
    _imToDraw = toSet;
    ::PostMessage(_hwnd, WM_USER, 0, 0);
}

void ThreadedDialog::Stop()
{
    if (0 == _hwnd)
    {
        return;
    }
    _bEndAllowed = true;
    ::PostMessage(_hwnd, WM_CLOSE, 0, 0);
}

void ThreadedDialog::Join()
{
    if (0 == _hwnd)
    {
        return;
    }

    _thread->join();
    _thread.reset();
    _hwnd = 0;
}

void ThreadedDialog::OnCommand(DialogsSync::CommandSequence<DialogsSync::Command>::TCommand& pCmd) noexcept
{
    if (pCmd->type() == DialogsSync::Command::cmdScrollTo)
    {
        //std::cout << "scroll_to ";
        //std::wcout << _title.c_str();
        //std::cout << " " << _imageOrigin.X << " " << _imageOrigin.Y << std::endl;
        _imageOrigin.X = pCmd->_imageOrigin.X;
        _imageOrigin.Y = pCmd->_imageOrigin.Y;
        assert(_imageOrigin.Y >= 0.);
        assert(_imageOrigin.X >= 0.);
        assert(_imageOrigin.Y <= 1.);
        assert(_imageOrigin.X <= 1.);
        ::PostMessage(_hwnd, WM_USER, 0, 0);
    }

}

BOOL ThreadedDialog::EvCommand(WPARAM wParam, LPARAM lParam)
{
    if (_bEndAllowed)
    if (IDCANCEL == wParam || IDOK == wParam)
    {
        ::EndDialog(_hwnd, IDOK);
        return TRUE;
    };

    return FALSE;
}

BOOL ThreadedDialog::EvMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (WM_USER == msg)
    {
//        ::RedrawWindow(_hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW);
       ::InvalidateRect(_hwnd, NULL, FALSE);
    }
    if (WM_LBUTTONDOWN == msg)
    {
        RECT rc;
        ::GetClientRect(_hwnd, &rc);
        DialogsSync::commands[DialogsSync::Command::cmdScrollTo]._imageOrigin = Gdiplus::PointF{
            static_cast<Gdiplus::REAL>(LOWORD(lParam)) / static_cast<Gdiplus::REAL>(rc.right - rc.left),
            static_cast<Gdiplus::REAL>(HIWORD(lParam)) / static_cast<Gdiplus::REAL>(rc.bottom - rc.top)
        };
        _cmdQueue.add(std::make_shared<DialogsSync::Command>(DialogsSync::commands[DialogsSync::Command::cmdScrollTo]));
    }
    return FALSE;
}

void ThreadedDialog::OnPaint(const HDC hdc, WPARAM& wParam, LPARAM& lParam)
{
    RECT rc;
    ::GetClientRect(_hwnd, &rc);
    (*painter)(hdc, _imToDraw, _imageOrigin, rc);
}

void ThreadedDialog::OnSysClose() const noexcept
{
    _cmdQueue.add(std::make_shared<DialogsSync::Command>(DialogsSync::commands[DialogsSync::Command::cmdClose]));
}

void ThreadedDialog::InThread()
{
    INITCOMMONCONTROLSEX iCCE;
    iCCE.dwSize = sizeof(iCCE);
    iCCE.dwICC = ICC_WIN95_CLASSES;
    ::InitCommonControlsEx(&iCCE);

    DialogBoxParam(nullptr, MAKEINTRESOURCE(IDD_DIALOGBAR), nullptr, (DLGPROC)DialogProc, reinterpret_cast<LPARAM>(this));
}

