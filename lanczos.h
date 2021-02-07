#pragma once
class lanczos
{
public:
    lanczos();
    ~lanczos();

    enum class eresize : int
    {
        initial = 3,
        type2 = initial + 1,
        type3 = type2 + 1,
        type4 = type3 + 1,
        type5 = type4 + 1,
        mythreads = type5 + 1,
        secondcacheline = mythreads + 1
    };
    static void Resize(const std::shared_ptr<Gdiplus::Bitmap>& src,
        std::shared_ptr<Gdiplus::Bitmap>& target,
        const double scale,
        eresize how);

};



