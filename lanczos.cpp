#include "stdafx.h"
#include "lanczos.h"
#include "timemeasurer.h"


namespace
{
    constexpr const double lanczos_size_d = 3.0;
    constexpr const int32_t lanczos_size_i = 3;
    constexpr const double pi = 3.1415926535897932384626433832795028841971;

    struct u_argb
    {
        uint8_t m_N[4];
    };
}

inline double sinc(register double x) {
    x = (x * pi);
    if (x < 0.01 && x > -0.01)
        return 1.0 + x * x* (-1.0 / 6.0 + x * x * 1.0 / 120.0);
    return sin(x) / x;
}
inline double sincNew(double x) {
    if (x < 0.000000001 && x > -0.000000001) return 1.;
//    x = (x * pi);
    return sin(x) / x;
}

inline double LanczosFilter(double x) {
    if (fabs(x) < lanczos_size_d) {
        return sinc(x)*sinc(x / lanczos_size_d);
    }
    else {
        return 0.0;
    }
}



void ResizeDD(
    double* const pixelsSrc,
    const int32_t src_width, const int32_t src_height,
    double* const pixelsTarget,
    int32_t const new_width, int32_t const new_height)
{
    const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
    const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);

    // Now apply a filter to the image.
    for (int32_t r = 0; r < new_height; ++r)
    {
        const double row_within = static_cast<double>(r)* row_ratio;
        const int floor_row = static_cast<int>(row_within);
        for (int32_t c = 0; c < new_width; ++c)
        {
            // x is the new col in terms of the old col coordinates.
            double col_within = static_cast<double>(c)* col_ratio;
            // The old col corresponding to the closest new col.
            const int floor_col = static_cast<int>(col_within);

            double& v_toSet = pixelsTarget[r * new_width + c] = 0.;
            double weight = 0.0;
            for (int32_t i = floor_row - lanczos_size_i + 1; i <= floor_row + lanczos_size_i; ++i)
            {
                for (int32_t j = floor_col - lanczos_size_i + 1; j <= floor_col + lanczos_size_i; ++j)
                {
                    if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                    {
                        const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);
                        v_toSet += pixelsSrc[i * src_width + j] * lanc_term;
                        weight += lanc_term;
                    }
                }
            }

            v_toSet /= weight;
            if (v_toSet > 1.0) v_toSet = 1.0;
            if (v_toSet < 0.0) v_toSet = 0.0;
        }
    }
}

void Resize_(u_argb* const pixelsSrc, const unsigned int src_width, const unsigned int src_height,
             u_argb* const pixelsTarget, const unsigned int new_width, const unsigned int new_height)
{
    const auto src_N = src_width * src_height;
    std::vector<double> srcVec(src_N);

    const auto nNew = new_width * new_height;
    std::vector<double> trgVec(nNew);

    for (auto i = 0; i < 4; ++i)
    {
        unsigned int k = 0;
        for (; k < src_N; ++k)
        {
            srcVec[k] = static_cast<double>(pixelsSrc[k].m_N[i]) / 255.0;
        }
        ResizeDD(srcVec.data(), src_width, src_height, trgVec.data(), new_width, new_height);

        for (k = 0; k < nNew; ++k)
        {
            pixelsTarget[k].m_N[i] = static_cast<uint8_t>(trgVec[k] * 255.0);
        }
    }
}

void ResizeDD_2(
    std::function<double(const unsigned int atX, const unsigned int atY)>& funSrc,
    const int32_t src_width, const int32_t src_height,
    std::function<void (const unsigned int atX, const unsigned int atY, const uint8_t set)>& funTrg,
    int32_t const new_width, int32_t const new_height)
{
    const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
    const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);

    // Now apply a filter to the image.
    for (int32_t r = 0; r < new_height; ++r)
    {
        const double row_within = static_cast<double>(r) * row_ratio;
        const int floor_row = static_cast<int>(row_within);
        for (int32_t c = 0; c < new_width; ++c)
        {
            // x is the new col in terms of the old col coordinates.
            double col_within = static_cast<double>(c) * col_ratio;
            // The old col corresponding to the closest new col.
            const int floor_col = static_cast<int>(col_within);

            double v_toSet = 0.;

            double weight = 0.0;
            for (int32_t i = floor_row - lanczos_size_i + 1; i <= floor_row + lanczos_size_i; ++i)
            {
                for (int32_t j = floor_col - lanczos_size_i + 1; j <= floor_col + lanczos_size_i; ++j)
                {
                    if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                    {
                        const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);
                        v_toSet += funSrc(j, i) * lanc_term;
                        weight += lanc_term;
                    }
                }
            }

            v_toSet /= weight;
            v_toSet = (v_toSet > 1.0) ? 1.0 : v_toSet;
            v_toSet = (v_toSet < 0.0) ? 0.0 : v_toSet;
            funTrg(c, r, static_cast<uint8_t>(v_toSet * 255.));
        }
    }
}

void ResizeDD_3(
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funSrc,
    const int32_t src_width, const int32_t src_height,
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funTrg,
    int32_t const new_width, int32_t const new_height)
{
    const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
    const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);

    // Now apply a filter to the image.
    for (int32_t r = 0; r < new_height; ++r)
    {
        const double row_within = static_cast<double>(r) * row_ratio;
        const int floor_row = static_cast<int>(row_within);
        for (int32_t c = 0; c < new_width; ++c)
        {
            // x is the new col in terms of the old col coordinates.
            const double col_within = static_cast<double>(c) * col_ratio;
            // The old col corresponding to the closest new col.
            const int floor_col = static_cast<int>(col_within);


            double v_toSet[4] = { 0. };
            double weight[sizeof(v_toSet) / sizeof(*v_toSet)] = {0.0};
            for (int32_t i = floor_row - lanczos_size_i + 1; i <= floor_row + lanczos_size_i; ++i)
            {
                for (int32_t j = floor_col - lanczos_size_i + 1; j <= floor_col + lanczos_size_i; ++j)
                {
                    if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                    {
                        const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);
                        double stepValue[sizeof(v_toSet) / sizeof(*v_toSet)] = {0.};
                        funSrc(j, i, stepValue);
                        for (int zz = 0; zz < sizeof(v_toSet) / sizeof(*v_toSet); ++zz)
                        {
                            v_toSet[zz] += stepValue[zz] * lanc_term;
                            weight[zz] += lanc_term;
                        }
                    }
                }
            }

            for (int zz = 0; zz < sizeof(v_toSet) / sizeof(*v_toSet); ++zz)
            {
                v_toSet[zz] /= weight[zz];
                if (v_toSet[zz] > 1.0) v_toSet[zz] = 1.0;
                if (v_toSet[zz] < 0.0) v_toSet[zz] = 0.0;
            }
            funTrg(c, r, v_toSet);
#undef v_toSet
#undef weight
        }
    }
}

void ResizeDD_4_X(
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funSrc,
    const int32_t src_width, const int32_t src_height,
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funTrg,
    int32_t const new_width, int32_t const new_height)
{
    double d[8];
    d[4] = static_cast<double>(src_width) / static_cast<double>(new_width);
    d[5] = static_cast<double>(src_height) / static_cast<double>(new_height);
#define col_ratio d[4]
#define row_ratio d[5]

    // Now apply a filter to the image.
    for (int32_t r = 0; r < new_height; ++r)
    {
        const double row_within = static_cast<double>(r) * row_ratio;
#define stepValue d
#define row_within d[6]
#define col_within d[7]
        row_within = static_cast<double>(r) * row_ratio;

        for (int32_t c = 0; c < new_width; ++c)
        {
            // x is the new col in terms of the old col coordinates.
            col_within = static_cast<double>(c) * col_ratio;

            double active_data[8] = {0.};
#define v_toSet active_data
#define weight (&active_data[4])

            for (int32_t i = static_cast<int32_t>(row_within) - lanczos_size_i + 1; i <= static_cast<int32_t>(row_within) + lanczos_size_i; ++i)
            {
                for (int32_t j = static_cast<int32_t>(col_within) - lanczos_size_i + 1; j <= static_cast<int32_t>(col_within) + lanczos_size_i; ++j)
                {
                    if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                    {
                        const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);

                        funSrc(j, i, stepValue);
                        for (int zz = 0; zz < 4; ++zz)
                        {
                            v_toSet[zz] += stepValue[zz] * lanc_term;
                            weight[zz] += lanc_term;
                        }
#undef col_within
#undef row_within
#undef stepValue
                    }
                }
            }

            for (int zz = 0; zz < 4; ++zz)
            {
                v_toSet[zz] /= weight[zz];
                if (v_toSet[zz] > 1.0) v_toSet[zz] = 1.0;
                if (v_toSet[zz] < 0.0) v_toSet[zz] = 0.0;
            }
            funTrg(c, r, v_toSet);
#undef v_toSet
#undef weight
        }
    }
#undef row_ratio
#undef col_ratio
}


void ResizeDD_4(
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funSrc,
    const int32_t src_width, const int32_t src_height,
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funTrg,
    int32_t const new_width, int32_t const new_height)
{
    const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
    const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);

    // Now apply a filter to the image.
//#pragma omp parallel for
    for (int32_t r = 0; r < new_height; ++r)
    {
        const double row_within = static_cast<double>(r) * row_ratio;
        const int floor_row = static_cast<int>(row_within);
        for (int32_t c = 0; c < new_width; ++c)
        {
            // x is the new col in terms of the old col coordinates.
            const double col_within = static_cast<double>(c) * col_ratio;
            // The old col corresponding to the closest new col.
            const int floor_col = static_cast<int>(col_within);

            double active_data[8] = {0.};

            for (int32_t i = floor_row - lanczos_size_i + 1; i <= floor_row + lanczos_size_i; ++i)
            {
                for (int32_t j = floor_col - lanczos_size_i + 1; j <= floor_col + lanczos_size_i; ++j)
                {
                    if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                    {
                        const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);
                        double stepValue[4] = {0.};
                        funSrc(j, i, stepValue);
                        for (int zz = 0; zz < 4; ++zz)
                        {
                            active_data[zz] += stepValue[zz] * lanc_term;
                            (&active_data[4])[zz] += lanc_term;
                        }

//                        std::for_each(&active_data[0], &active_data[4], [&](double& d) {d += stepValue[&d - active_data] * lanc_term; });
//                        std::for_each(&active_data[4], &active_data[8], [=](double& d) {d+= lanc_term;});
                    }
                }
            }

            for (int zz = 0; zz < 4; ++zz)
            {
                active_data[zz] /= (&active_data[4])[zz];
                if (active_data[zz] > 1.0) active_data[zz] = 1.0;
                if (active_data[zz] < 0.0) active_data[zz] = 0.0;
            }
            funTrg(c, r, active_data);
        }
    }
}


void ResizeDD_5(
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funSrc,
    const int32_t src_width, const int32_t src_height,
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funTrg,
    int32_t const new_width, int32_t const new_height)
{
    const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
    const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);

    // Now apply a filter to the image.
#pragma omp parallel for
    for (int32_t r = 0; r < new_height; ++r)
    {
        const double row_within = static_cast<double>(r) * row_ratio;
        const int floor_row = static_cast<int>(row_within);
        for (int32_t c = 0; c < new_width; ++c)
        {
            // x is the new col in terms of the old col coordinates.
            const double col_within = static_cast<double>(c) * col_ratio;
            // The old col corresponding to the closest new col.
            const int floor_col = static_cast<int>(col_within);

            double active_data[8] = {0.};
#define v_toSet active_data
#define weight (&active_data[4])

            for (int32_t i = floor_row - lanczos_size_i + 1; i <= floor_row + lanczos_size_i; ++i)
            {
                for (int32_t j = floor_col - lanczos_size_i + 1; j <= floor_col + lanczos_size_i; ++j)
                {
                    if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                    {
                        const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);
                        //                        const double lanc_term = LanczosFilter(row_within - i + col_within - j);
                        double stepValue[4] = {0.};
                        funSrc(j, i, stepValue);
                        for (int zz = 0; zz < 4; ++zz)
                        {
                            v_toSet[zz] += stepValue[zz] * lanc_term;
                            weight[zz] += lanc_term;
                        }
                    }
                }
            }

            for (int zz = 0; zz < 4; ++zz)
            {
                v_toSet[zz] /= weight[zz];
                if (v_toSet[zz] > 1.0) v_toSet[zz] = 1.0;
                if (v_toSet[zz] < 0.0) v_toSet[zz] = 0.0;
            }
            funTrg(c, r, v_toSet);
#undef v_toSet
#undef weight
        }
    }
}

void ResizeDD_6(
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funSrc,
    const int32_t src_width, const int32_t src_height,
    std::function<void(const unsigned int atX, const unsigned int atY, double* const)>& funTrg,
    int32_t const new_width, int32_t const new_height)
{
    // Now apply a filter to the image.
    const int nThreadsOMP = omp_get_max_threads();
    std::vector<std::unique_ptr<std::thread>> threads(nThreadsOMP);
    for (int threadpos = 0; threadpos < nThreadsOMP; ++threadpos)
    {
        threads[threadpos] = std::make_unique<std::thread>(
            [=]()
            {
                const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
                const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);
                for (int32_t r = threadpos; r < new_height; r += nThreadsOMP)
                {
                    const double row_within = static_cast<double>(r) * row_ratio;
                    const int floor_row = static_cast<int>(row_within);
                    for (int32_t c = 0; c < new_width; ++c)
                    {
                        // x is the new col in terms of the old col coordinates.
                        const double col_within = static_cast<double>(c) * col_ratio;
                        // The old col corresponding to the closest new col.
                        const int floor_col = static_cast<int>(col_within);

                        double active_data[8] = {0.};
#define v_toSet active_data
#define weight (&active_data[4])

                        for (int32_t i = floor_row - lanczos_size_i + 1; i <= floor_row + lanczos_size_i; ++i)
                        {
                            for (int32_t j = floor_col - lanczos_size_i + 1; j <= floor_col + lanczos_size_i; ++j)
                            {
                                if (i >= 0 && i < src_height && j >= 0 && j < src_width)
                                {
                                    const double lanc_term = LanczosFilter(row_within - i) * LanczosFilter(col_within - j);
                                    //                        const double lanc_term = LanczosFilter(row_within - i + col_within - j);
                                    double stepValue[4] = {0.};
                                    funSrc(j, i, stepValue);
                                    for (int zz = 0; zz < 4; ++zz)
                                    {
                                        v_toSet[zz] += stepValue[zz] * lanc_term;
                                        weight[zz] += lanc_term;
                                    }
                                }
                            }
                        }

                        for (int zz = 0; zz < 4; ++zz)
                        {
                            v_toSet[zz] /= weight[zz];
                            if (v_toSet[zz] > 1.0) v_toSet[zz] = 1.0;
                            if (v_toSet[zz] < 0.0) v_toSet[zz] = 0.0;
                        }
                        funTrg(c, r, v_toSet);
#undef v_toSet
#undef weight
                    } // for c
                } // for r
            } // [] for thread
        ); // array of threads
    } // for threadpos

    for (auto& th : threads) th->join();
}

void Resize_2(u_argb* const pixelsSrc, const unsigned int src_width, const unsigned int src_height,
    u_argb* const pixelsTarget, const unsigned int new_width, const unsigned int new_height)
{
    for (auto i = 0; i < 4; ++i)
    {
        std::function<double(const unsigned int, const unsigned int)>funSrc =
            [&](const unsigned int atX, const unsigned int atY) -> double
        {
            return static_cast<double>(pixelsSrc[atY * src_width + atX].m_N[i]) / 255.0;
        };
        std::function<void(const unsigned int, const unsigned int, const uint8_t)> funTrg =
            [&](const unsigned int atX, const unsigned int atY, const uint8_t set) -> void
        {
            pixelsTarget[atX + atY * new_width].m_N[i] = set;
        };
        ResizeDD_2(funSrc, src_width, src_height, funTrg, new_width, new_height);
    }
}

void Resize_3(uint32_t* const pixelsSrc, const unsigned int src_width, const unsigned int src_height,
    uint32_t* const pixelsTarget, const unsigned int new_width, const unsigned int new_height,
    lanczos::eresize how)
{

    std::function<void(const unsigned int, const unsigned int, double* const)>funSrc =
        [&](const unsigned int atX, const unsigned int atY, double* const tofill) -> void
    {
        uint32_t n = pixelsSrc[atY * src_width + atX];
        for (int i = 0; i < 4; ++i)
        {
            tofill[i] = static_cast<double>(n & 0x0ff) / 255.0;
            n >>= 8;
        }
    };
    std::function<void(const unsigned int, const unsigned int, double* const)> funTrg =
        [&](const unsigned int atX, const unsigned int atY, double* const toset) -> void
    {
        uint32_t n = 0;
        for (int i = 3; i > -1; --i)
        {
            n <<= 8;
            n += static_cast<uint32_t>(toset[i] * 255.);
        }
        pixelsTarget[atX + atY * new_width] = n;
    };

    if (how == lanczos::eresize::type3)
    {
        ResizeDD_3(funSrc, src_width, src_height, funTrg, new_width, new_height);
    }
    else if (how == lanczos::eresize::type4)
    {
        ResizeDD_4(funSrc, src_width, src_height, funTrg, new_width, new_height);
    }
    else if (how == lanczos::eresize::type5)
    {
        std::cout << "Number of omp threads is " << omp_get_max_threads() << std::endl;
        ResizeDD_5(funSrc, src_width, src_height, funTrg, new_width, new_height);
    }
    else if (how == lanczos::eresize::mythreads)
    {
        ResizeDD_6(funSrc, src_width, src_height, funTrg, new_width, new_height);
    }

}

void Resize_4(uint32_t* const pixelsSrc, const unsigned int src_width, const unsigned int src_height,
    uint32_t* const pixelsTarget, const unsigned int new_width, const unsigned int new_height)
{

    std::function<void(const unsigned int, const unsigned int, double* const)>funSrc =
        [&](const unsigned int atX, const unsigned int atY, double* const tofill) -> void
    {
        uint32_t n = pixelsSrc[atY * src_width + atX];
        for (int i = 0; i < 4; ++i)
        {
            tofill[i] = static_cast<double>(n & 0x0ff) / 255.;
            n >>= 8;
        }
    };
    std::function<void(const unsigned int, const unsigned int, double* const)> funTrg =
        [&](const unsigned int atX, const unsigned int atY, double* const toset) -> void
    {
        uint32_t n = 0;
        for (int i = 3; i > -1; --i)
        {
            n <<= 8;
            n += static_cast<uint32_t>(toset[i] * 255.);
        }
        pixelsTarget[atX + atY * new_width] = n;
    };
    ResizeDD_4(funSrc, src_width, src_height, funTrg, new_width, new_height);
}

void Resize_5(uint32_t* const pixelsSrc, const unsigned int src_width, const unsigned int src_height,
    uint32_t* const pixelsTarget, const unsigned int new_width, const unsigned int new_height)
{

    std::function<void(const unsigned int, const unsigned int, double* const)>funSrc =
        [&](const unsigned int atX, const unsigned int atY, double* const tofill) -> void
    {
        uint32_t n = pixelsSrc[atY * src_width + atX];
        for (int i = 0; i < 4; ++i)
        {
            tofill[i] = static_cast<double>(n & 0x0ff) / 255.;
            n >>= 8;
        }
    };
    std::function<void(const unsigned int, const unsigned int, double* const)> funTrg =
        [&](const unsigned int atX, const unsigned int atY, double* const toset) -> void
    {
        uint32_t n = 0;
        for (int i = 3; i > -1; --i)
        {
            n <<= 8;
            n += static_cast<uint32_t>(toset[i] * 255.);
        }
        pixelsTarget[atX + atY * new_width] = n;
    };
    ResizeDD_5(funSrc, src_width, src_height, funTrg, new_width, new_height);
}

void Resize_6(uint32_t* const pixelsSrc, const unsigned int src_width, const unsigned int src_height,
    uint32_t* const pixelsTarget, const unsigned int new_width, const unsigned int new_height)
{

    std::function<void(const unsigned int, const unsigned int, double* const)>funSrc =
        [&](const unsigned int atX, const unsigned int atY, double* const tofill) -> void
    {
        uint32_t n = pixelsSrc[atY * src_width + atX];
        for (int i = 0; i < 4; ++i)
        {
            tofill[i] = static_cast<double>(n & 0x0ff) / 255.;
            n >>= 8;
        }
    };
    std::function<void(const unsigned int, const unsigned int, double* const)> funTrg =
        [&](const unsigned int atX, const unsigned int atY, double* const toset) -> void
    {
        uint32_t n = 0;
        for (int i = 3; i > -1; --i)
        {
            n <<= 8;
            n += static_cast<uint32_t>(toset[i] * 255.);
        }
        pixelsTarget[atX + atY * new_width] = n;
    };
    ResizeDD_6(funSrc, src_width, src_height, funTrg, new_width, new_height);
}

namespace
{
    struct cacheline
    {
        uint32_t* const src;// = pixelsSrc; // 8
        uint32_t* const trg;// = pixelsTarget; // 8
        uint32_t new_width; // 4
        uint32_t new_height; // 4
        uint32_t src_width; // 4
        uint32_t src_height; // 4
        int32_t r;
        int32_t c; // column
        int32_t i;
        int32_t j;
        uint32_t n;
        int32_t zz;
        double lanc_term;
        
        void functionSource(double* const toset)
        {
            n = src[i * src_width + j];
            for (zz = 0; zz < 4; ++zz)
            {
                toset[zz] = static_cast<double>(n & 0x0ff) / 255.;
                n >>= 8;
            }
        }
        void functionTarget(double* const toset)
        {
            n = 0;
            for (zz = 3; zz > -1; --zz)
            {
                n <<= 8;
                n += static_cast<uint32_t>(toset[zz] * 255.);
            }
            trg[c + r * new_width] = n;
        }
    };
    static_assert(sizeof(cacheline) == 64, "it must be so");
}


void Resize_CacheLines(uint32_t* const pixelsSrc, const unsigned int src_widthP, const unsigned int src_heightP,
    uint32_t* const pixelsTarget, const unsigned int new_widthP, const unsigned int new_heightP)
{
    cacheline c = {pixelsSrc, pixelsTarget, new_widthP, new_heightP, src_widthP, src_heightP,
                   0, 0, 0, 0, 0, 0};
    // const double col_ratio = static_cast<double>(src_width) / static_cast<double>(new_width);
    //const double row_ratio = static_cast<double>(src_height) / static_cast<double>(new_height);

    double stepValue[8] = {0., 0., 0., 0.,
        static_cast<double>(src_widthP) / static_cast<double>(new_widthP), // col_ratio
        static_cast<double>(src_heightP) / static_cast<double>(new_heightP), // row_ratio
        0., // col_within   6
        0. // row_within  7
    };

    // Now apply a filter to the image.
    for (c.r = 0; c.r < static_cast<int32_t>(c.new_height); ++c.r)
    {
        stepValue[7] = static_cast<double>(c.r) * stepValue[5];
        for (c.c = 0; c.c < static_cast<int32_t>(c.new_width); ++c.c)
        {
            // x is the new col in terms of the old col coordinates.
            stepValue[6] = static_cast<double>(c.c) * stepValue[4];

            double active_data[8] = {0.};
#define v_toSet active_data
#define weight (&active_data[4])

            for (c.i = static_cast<int32_t>(stepValue[7]) - lanczos_size_i + 1; c.i <= static_cast<int32_t>(stepValue[7]) + lanczos_size_i; ++c.i)
            {
                for (c.j = static_cast<int32_t>(stepValue[6]) - lanczos_size_i + 1; c.j <= static_cast<int32_t>(stepValue[6]) + lanczos_size_i; ++c.j)
                {
                    if (c.i >= 0 && c.i < static_cast<int32_t>(c.src_height) && c.j >= 0 && c.j < static_cast<int32_t>(c.src_width))
                    {
                        c.lanc_term = LanczosFilter(stepValue[7] - c.i) * LanczosFilter(stepValue[6] - c.j);
                        c.functionSource(stepValue);
                        for (c.zz = 0; c.zz < 4; ++c.zz)
                        {
                            v_toSet[c.zz] += stepValue[c.zz] * c.lanc_term;
                            weight[c.zz] += c.lanc_term;
                        }
                    }
                }
            }

            for (c.zz = 0; c.zz < 4; ++c.zz)
            {
                v_toSet[c.zz] /= weight[c.zz];
                if (v_toSet[c.zz] > 1.0) v_toSet[c.zz] = 1.0;
                if (v_toSet[c.zz] < 0.0) v_toSet[c.zz] = 0.0;
            }
            c.functionTarget(v_toSet);
#undef v_toSet
#undef weight
        }
    }

}



void lanczos::Resize(const std::shared_ptr<Gdiplus::Bitmap>& src,
    std::shared_ptr<Gdiplus::Bitmap>& target,
    const double scale,
    eresize how)
{
    const UINT v_width = src->GetWidth();
    const UINT v_height = src->GetHeight();

    Gdiplus::BitmapData bitmapData{ 0 };
    Gdiplus::Rect rect(0, 0, v_width, v_height);
    // Lock a rectangular portion of the bitmap for reading.
    src->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
    u_argb* const pixels = static_cast<u_argb*>(bitmapData.Scan0);

    Gdiplus::Rect rectTarget(0, 0,
        static_cast<INT>(static_cast<double>(v_width) / scale),
        static_cast<INT>(static_cast<double>(v_height) / scale));
    target = std::make_shared<Gdiplus::Bitmap>(rectTarget.Width, rectTarget.Height);//  IN PixelFormat format = PixelFormat32bppARGB;

    // Lock a rectangular portion of the bitmap for writing.
    Gdiplus::BitmapData targetBitmapData{0};
    target->LockBits(
        &rectTarget,
        Gdiplus::ImageLockModeWrite,
        PixelFormat32bppARGB,
        &targetBitmapData);

    //    assert(v_width * v_height == bitmapData.Stride);
    //    assert(rectTarget.Width * rectTarget.Height == targetBitmapData.Stride);

    timemeasurer measure;
    if (how == eresize::initial)
    {
        Resize_(static_cast<u_argb* const>(bitmapData.Scan0), v_width, v_height,
            static_cast<u_argb* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height);
    }
    else if (how == eresize::type2)
    {
        Resize_2(static_cast<u_argb* const>(bitmapData.Scan0), v_width, v_height,
            static_cast<u_argb* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height);
    }
    else if (how == eresize::secondcacheline)
    {
        Resize_CacheLines(static_cast<uint32_t* const>(bitmapData.Scan0), v_width, v_height,
            static_cast<uint32_t* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height);
    }
    else //if (how == eresize::type3)
    {
        Resize_3(static_cast<uint32_t* const>(bitmapData.Scan0), v_width, v_height,
            static_cast<uint32_t* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height,
            how);
    }
    //else if (how == eresize::type4)
    //{
    //    Resize_4(static_cast<uint32_t* const>(bitmapData.Scan0), v_width, v_height,
    //        static_cast<uint32_t* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height);
    //}
    //else if (how == eresize::type5)
    //{
    //    std::cout << "Number of omp threads is " << omp_get_max_threads() << std::endl;
    //    Resize_5(static_cast<uint32_t* const>(bitmapData.Scan0), v_width, v_height,
    //        static_cast<uint32_t* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height);
    //}
    //else if (how == eresize::mythreads)
    //{
    //    std::cout << "Number of omp threads is " << omp_get_max_threads() << std::endl;
    //    Resize_6(static_cast<uint32_t* const>(bitmapData.Scan0), v_width, v_height,
    //        static_cast<uint32_t* const>(targetBitmapData.Scan0), rectTarget.Width, rectTarget.Height);
    //}
    target->UnlockBits(&targetBitmapData);
    src->UnlockBits(&bitmapData);
}

