#include "fp16.h"
#include <cstdint>

/* we use the union trick i.s.o. reinterpreting between float and uint32_t because it does
 * not generate a strict-aliasing warning */
union Convert
{
    float f;
    uint32_t u;
};

__half jetnet::__float2half(float f)
{
    Convert c = {f};
    uint32_t x = c.u;
    uint32_t u = (x & 0x7fffffff);

    // Get rid of +NaN/-NaN case first.
    if (u > 0x7f800000)
        return bitwise_cast<__half, uint16_t>(uint16_t(0x7fff));

    uint16_t sign = ((x >> 16) & 0x8000);

    // Get rid of +Inf/-Inf, +0/-0.
    if (u > 0x477fefff)
        return bitwise_cast<__half, uint16_t>(sign | uint16_t(0x7c00));

    if (u < 0x33000001)
        return bitwise_cast<__half, uint16_t>(sign | uint16_t(0x0000));

    uint32_t exponent = ((u >> 23) & 0xff);
    uint32_t mantissa = (u & 0x7fffff);

    uint32_t shift;
    if (exponent > 0x70)
    {
        shift = 13;
        exponent -= 0x70;
    }
    else
    {
        shift = 0x7e - exponent;
        exponent = 0;
        mantissa |= 0x800000;
    }

    uint32_t lsb    = (1 << shift);
    uint32_t lsb_s1 = (lsb >> 1);
    uint32_t lsb_m1 = (lsb - 1);

    // Round to nearest even.
    uint32_t remainder = (mantissa & lsb_m1);
    mantissa >>= shift;
    if ( (remainder > lsb_s1) || ((remainder == lsb_s1) && (mantissa & 0x1)) )
    {
        ++mantissa;
        if (!(mantissa & 0x3ff))
        {
            ++exponent;
            mantissa = 0;
        }
    }

    return bitwise_cast<__half, uint16_t>(sign | uint16_t(exponent<<10) | uint16_t(mantissa));
}

float jetnet::__half2float(__half h)
{
    uint16_t x        = bitwise_cast<uint16_t,__half>(h);
    uint32_t sign     = ((x >> 15) & 1);
    uint32_t exponent = ((x >> 10) & 0x1f);
    uint32_t mantissa = (static_cast<uint32_t>(x & 0x3ff) << 13);

    if (exponent == 0x1f)
    {  /* NaN or Inf */
        if (mantissa != 0)
        {   // NaN
            sign     = 0;
            mantissa = 0x7fffff;
        }
        else // Inf
            mantissa = 0;
        exponent = 0xff;
    }
    else if (!exponent)
    {  /* Denorm or Zero */
        if (mantissa) {
            unsigned int msb;
            exponent = 0x71;
            do
            {
                msb = (mantissa & 0x400000);
                mantissa <<= 1; /* normalize */
                --exponent;
            }
            while (!msb);
            mantissa &= 0x7fffff; /* 1.mantissa is implicit */
        }
    }
    else
        exponent += 0x70;
    Convert c{.u = (sign<<31) | (exponent<<23) | mantissa };
    return c.f;
}
