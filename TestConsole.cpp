// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ThreadedDialog.h"
#include "GDIPlusController.h"
#include "lanczos.h"

namespace
{
    constexpr const size_t szSquare = 16;
}


struct Randomer
{
    Randomer(const size_t topEdge):gen(static_cast<unsigned int>(time(0))), uid(0, static_cast<uint32_t>(topEdge - 1))
    {
    }
    uint32_t get()
    {
        return uid(gen);
    }
    std::mt19937 gen;
    std::uniform_int_distribution<uint32_t> uid;
};


/// https://deadlockempire.github.io/


int main()
{
    system("mode con cols=80 lines=25");
    HANDLE hOutput = (HANDLE)::GetStdHandle(STD_OUTPUT_HANDLE);

    Randomer rr(szSquare);
    std::atomic_bool bStop{ false };

goto dialogekTest;
    if (true) // introduction
    {

        auto threadProcedure = [&rr, &hOutput, &bStop](const WORD color, uint32_t x, uint32_t y, const char c, const uint32_t dxLegend)
        {
            ConnsoleCharWriter wr(hOutput, color, c);
            while (!bStop.load())
            {
                /// write 1 char
                wr.WriteChar(x + rr.get(), y + rr.get());
                /// clear 1 char
                const uint32_t xClear = x + rr.get();
                const uint32_t yClear = y + rr.get();
                ::Sleep(100);
                char toRead = 0;
                if (wr.GetChar(xClear, yClear, toRead) && wr.MyChar() == toRead)
                {
                    wr.WriteChar(xClear, yClear, FOREGROUND_INTENSITY, ' ');
                }
                /// output count of our characters in square
                size_t n = 0;
                for (uint32_t i = 0; i < szSquare; ++i)
                    for (uint32_t j = 0; j < szSquare; ++j)
                    {
                        if (wr.GetChar(x + i, y + j, toRead) && wr.MyChar() == toRead)
                            ++n;
                    }
                std::string s = std::to_string(n % 1000);
                for (uint32_t i = 0; i < 3; ++i)
                {
                    wr.WriteChar(x + i + dxLegend, y + szSquare + 1, color, s.length() > i ? s.at(i) : ' ');
                }
            }
        };


        std::thread a001(threadProcedure, COLOR_FOR_US_001, 10, 5, 'X', 0);
        std::thread a002(threadProcedure, COLOR_FOR_US_002, 10, 5, '@', 5);
        std::thread a003(threadProcedure, COLOR_FOR_US_003, 30, 5, '#', 0);
        std::thread a004(threadProcedure, COLOR_FOR_US_004, 37, 5, '*', 0);
        std::thread a005(threadProcedure, COLOR_FOR_US_005, 55, 5, 'H', 0);
        system("pause");
        bStop.store(true);
        a001.join();
        a002.join();
        a003.join();
        a004.join();
        a005.join();

        system("cls");
        system("mode con cols=100 lines=48");
        bStop.store(false);

        /// #pragma omp parallel   https://www.openmp.org/specifications/  [~ 646 pages of specification ;-) ]
        std::thread tFill([&]
        {
            ConnsoleCharWriter wr(hOutput, COLOR_FOR_US_005, '~');
            const int nThreadsOMP = omp_get_max_threads() - 1;
            Randomer r100(100);
            {
    #pragma omp parallel for num_threads(nThreadsOMP)

                for (int j = 0; j < 32; ++j)
                {
                    if (bStop.load()) break;
                    for (int i = 0; i < 96; ++i)
                    {
                        wr.WriteChar(i + 1, j + 1);
                        if (bStop.load())
                        {
                            break;
                        }
                        ::Sleep(20 + rr.get());
                    }
                }
            }
        }
        );
        system("pause");
        system("cls");
        bStop.store(true);
        tFill.join(); // DO not Forget!


        system("mode con cols=100 lines=48");
        system("cls");
        ///@feature usage
        std::thread tFuture([&]()
        {
            bStop.store(false);
            Randomer r100(100);
            ConnsoleCharWriter wr1(hOutput, COLOR_FOR_US_005, '1');
            ConnsoleCharWriter wr2(hOutput, COLOR_FOR_US_001, '2');
            ConnsoleCharWriter wr3(hOutput, COLOR_FOR_US_002, '3');
            ConnsoleCharWriter wr4(hOutput, COLOR_FOR_US_003, '4');
            ConnsoleCharWriter wr5(hOutput, COLOR_FOR_US_004, '5');
            auto lineFiller = [&r100, &bStop](const int y, ConnsoleCharWriter& wr)->bool
            {
                for (int i = 10; i < 90; ++i)
                {
                    wr.WriteChar(i, y);
                    if (bStop.load())
                    {
                        break;
                    }
                    ::Sleep(20 + r100.get());
                }
                return true;
            };

            std::future<bool> rFut1 = std::async(std::launch::async, lineFiller, 8, wr1);
            std::future<bool> rFut2 = std::async(std::launch::deferred, lineFiller, 9, wr2);
            std::future<bool> rFut3 = std::async(std::launch::async, lineFiller, 10, wr3);
            std::future<bool> rFut4 = std::async(lineFiller, 11, wr4);
            std::future<bool> rFut5 = std::async(std::launch::deferred, lineFiller, 12, wr5);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            const bool bRez = rFut1.get()
                && rFut2.get()
                && rFut3.get()
                && rFut4.get()
                && rFut5.get();
        });
        system("pause");
        system("cls");
        bStop.store(true);
        tFuture.join(); // DO not Forget!

    }
dialogekTest:
    if (true)
    {
        GDIPlusController gdiC;
        DialogsSync::CommandSequence<DialogsSync::Command> cmdQueue; // our commands queue

        ThreadedDialog dialozhekHigh(_T("High Resolution Image"), cmdQueue);
        ThreadedDialog dialozhekResized(_T("Lanzos Resized Image"), cmdQueue);
        ThreadedDialog dialozhekBicubic(_T("Bicubic Resized Image"), cmdQueue);

        dialozhekHigh.Execute();
        dialozhekResized.Execute();
        dialozhekBicubic.Execute();
        {
            /// @brief 1: load image
            /// 2: execute resize with time measurements - no threading
            /// 3: pass image to the biggest image display
            /// 4: pass the image to the dialog with bicubic resampling
            /// 5: execute the same procedure with omp
            /// 6: execute manually optimised procedure

            //1: load image
            GDIPlusController gdiAccess;

            //  4096x3072
            //"D:\\Work\\TestConsole\\4096x3072MM.jpg"
            //"D:\\Work\\TestConsole\\cat365133.jpg"

            // 3000x1980
            //"D:\\Work\\TestConsole\\liziantus_rassela_rozy_cvety_buket_kapli_svezhest_listya_3000x1980.jpg"
            std::basic_string<TCHAR> imName(_T("C:\\Users\\Alexey_Zaytsev\\OneDrive - EPAM\\Attachments\\TestConsole\\ImageResizer\\TestConsole\\TestConsole\\resimg\\4096x3072MM.jpg"));
//            std::basic_string<TCHAR> imName(_T("C:\\Users\\Alexey_Zaytsev\\OneDrive - EPAM\\Attachments\\TestConsole\\ImageResizer\\TestConsole\\liziantus_rassela_rozy_cvety_buket_kapli_svezhest_listya_3000x1980.jpg"));
            std::shared_ptr<Gdiplus::Bitmap> im(Gdiplus::Bitmap::FromFile(imName.c_str(), FALSE));
            std::shared_ptr<Gdiplus::Bitmap> imresized;
            std::shared_ptr<Gdiplus::Bitmap> imLanczos;
            system("cls");
            ResizeBicubic(im, imresized, 3.14159265);
//            lanczos::Resize(im, imLanczos, 3.14159265, lanczos::eresize::initial);
//            lanczos::Resize(im, imLanczos, 5/*3.14159265*/, lanczos::eresize::type2);
//            lanczos::Resize(im, imLanczos, 3.14159265, lanczos::eresize::type3);

            lanczos::Resize(im, imLanczos, 3.14159265, lanczos::eresize::type4);
//            lanczos::Resize(im, imLanczos, 3.14159265, lanczos::eresize::type5);
//            lanczos::Resize(im, imLanczos, 3.14159265, lanczos::eresize::mythreads);
//            lanczos::Resize(im, imLanczos, 3.14159265, lanczos::eresize::secondcacheline);

            dialozhekHigh.SetImageData(im);
            dialozhekBicubic.SetImageData(imresized);
            dialozhekResized.SetImageData(imLanczos);
            system("echo \" Close one of dialogs to continue\"");
            DialogsSync::CommandSequence<DialogsSync::Command>::TCommand pCmd;
            do
            {
                if (!cmdQueue.next(pCmd))
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                    continue;
                }

                if (pCmd->type() == DialogsSync::Command::cmdScrollTo)
                {
                    dialozhekHigh.OnCommand(pCmd);
                    dialozhekBicubic.OnCommand(pCmd);
                    dialozhekResized.OnCommand(pCmd);
                }
            } while (pCmd == nullptr || pCmd->type() != DialogsSync::Command::cmdClose);
        }
        dialozhekBicubic.Stop();
        dialozhekResized.Stop();
        dialozhekHigh.Stop();

        dialozhekBicubic.Join();
        dialozhekResized.Join();
        dialozhekHigh.Join();

    }

    bStop.store(false);
    return 0;
}
