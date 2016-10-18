#include <assert.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../endian.h"
#include "../inline.h"
#include "bitrot.h"
#include "hash.h"

//------------------------------------------------------------------------------
//---- Pearson Hash ------------------------------------------------------------
//------------------------------------------------------------------------------
static const uint8_t hash_pearson_table[256] =
{
    0x27, 0x9F, 0xB4, 0xFC, 0x47, 0x06, 0x0D, 0xA4,
    0xE8, 0x23, 0xE2, 0x9B, 0x62, 0x78, 0x9A, 0x45,
    0x9D, 0x18, 0x89, 0x1D, 0x93, 0x4E, 0x79, 0x55,
    0x70, 0x08, 0xF8, 0x82, 0x37, 0x75, 0xBE, 0xA0,
    0xB0, 0x83, 0xE4, 0x40, 0xD3, 0x6A, 0x26, 0x1B,
    0x8C, 0x1E, 0x58, 0xD2, 0xE3, 0x68, 0x54, 0x4D,
    0x4B, 0x6B, 0xA9, 0x8A, 0xC3, 0xB8, 0x46, 0x5A,
    0x3D, 0xA6, 0x07, 0xF4, 0xA5, 0x6C, 0xDB, 0x33,
    0x09, 0x8B, 0xD1, 0x28, 0x1F, 0xCA, 0x3A, 0xB3,
    0x74, 0x21, 0xCF, 0x92, 0x4C, 0x3C, 0xF2, 0x7C,
    0xFE, 0xC5, 0x50, 0xA7, 0x99, 0x91, 0x81, 0xE9,
    0x84, 0x30, 0xF6, 0x56, 0x9C, 0xB1, 0x24, 0xBB,
    0x2D, 0x01, 0x60, 0x12, 0x13, 0x3E, 0xB9, 0xEA,
    0x63, 0x10, 0xDA, 0x5F, 0x80, 0xE0, 0x7B, 0xFD,
    0x2A, 0x6D, 0x04, 0xF7, 0x48, 0x05, 0x97, 0x88,
    0x00, 0x98, 0x94, 0x7F, 0xCC, 0x85, 0x11, 0x0E,
    0xB6, 0xD9, 0x36, 0xC7, 0x77, 0xAE, 0x52, 0x39,
    0xD7, 0x29, 0x72, 0xD0, 0xCE, 0x6E, 0xEF, 0x17,
    0xBD, 0x0F, 0x03, 0x16, 0xBC, 0x4F, 0x71, 0xAC,
    0x1C, 0x02, 0xDE, 0x15, 0xFB, 0xE1, 0xED, 0x69,
    0x66, 0x20, 0x38, 0xB5, 0x7E, 0x53, 0xE6, 0x35,
    0x9E, 0x34, 0x3B, 0xD5, 0x76, 0x64, 0x43, 0x8E,
    0xDC, 0xAA, 0x90, 0x73, 0xCD, 0x1A, 0x7D, 0xA8,
    0xF9, 0x42, 0xAF, 0x61, 0xFF, 0x5C, 0xE5, 0x5B,
    0xD6, 0xEC, 0xB2, 0xF3, 0x2E, 0x2C, 0xC9, 0xFA,
    0x87, 0xBA, 0x96, 0xDD, 0xA3, 0xD8, 0xA2, 0x2B,
    0x0B, 0x65, 0x22, 0x25, 0xC2, 0x19, 0x32, 0x0C,
    0x57, 0xC6, 0xAD, 0xF0, 0xC1, 0xAB, 0x8F, 0xE7,
    0x6F, 0x8D, 0xBF, 0x67, 0x4A, 0xF5, 0xDF, 0x14,
    0xA1, 0xEB, 0x7A, 0x3F, 0x59, 0x95, 0x49, 0xEE,
    0x86, 0x44, 0x5D, 0xB7, 0xF1, 0x51, 0xC4, 0x31,
    0xC0, 0x41, 0xD4, 0x5E, 0xCB, 0x0A, 0xC8, 0x2F
};
//------------------------------------------------------------------------------
uint8_t hash_pearson(const void* RESTRICT src, size_t size)
{
    /**
     * @brief   Pearson hashing.
     * @details A fast and simple hash function.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @return The hash number calculated.
     *
     * @see http://en.wikipedia.org/wiki/Pearson_hashing
     */
    const uint8_t *buf = src;
    uint8_t        res = 0;

    assert( src );

    while( size-- )
        res = hash_pearson_table[ res ^ *(buf++) ];

    return res;
}
//------------------------------------------------------------------------------
//---- CRC-16-CCITT ------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef UNDEF
// 當需要節省記憶體用量時可以使用這個函式來取代下面的表格
static
uint16_t hash_crc16_ccitt_table(uint8_t ind)
{
    /*
     * CRC-16-CCITT polynomial can be :
     *   0x1021 : Normal
     *   0x8408 : Reversed
     *   0x8810 : Reversed Reciprocal
     */
    static const uint16_t polynomial = 0x1021;
    uint16_t res = ind;
    int      i;

    for(i=8; i; --i)
    {
        if( res & 0x01 ) res   = ( res >> 1 ) ^ polynomial;
        else             res >>= 1;
    }

    return res;
}
#else
// 當需要加速運算時可以使用這個表格來取代上面的函式
static const uint16_t hash_crc16_ccitt_table[256] =
{
    0x0000, 0x17CE, 0x0FDF, 0x1811, 0x1FBE, 0x0870, 0x1061, 0x07AF,
    0x1F3F, 0x08F1, 0x10E0, 0x072E, 0x0081, 0x174F, 0x0F5E, 0x1890,
    0x1E3D, 0x09F3, 0x11E2, 0x062C, 0x0183, 0x164D, 0x0E5C, 0x1992,
    0x0102, 0x16CC, 0x0EDD, 0x1913, 0x1EBC, 0x0972, 0x1163, 0x06AD,
    0x1C39, 0x0BF7, 0x13E6, 0x0428, 0x0387, 0x1449, 0x0C58, 0x1B96,
    0x0306, 0x14C8, 0x0CD9, 0x1B17, 0x1CB8, 0x0B76, 0x1367, 0x04A9,
    0x0204, 0x15CA, 0x0DDB, 0x1A15, 0x1DBA, 0x0A74, 0x1265, 0x05AB,
    0x1D3B, 0x0AF5, 0x12E4, 0x052A, 0x0285, 0x154B, 0x0D5A, 0x1A94,

    0x1831, 0x0FFF, 0x17EE, 0x0020, 0x078F, 0x1041, 0x0850, 0x1F9E,
    0x070E, 0x10C0, 0x08D1, 0x1F1F, 0x18B0, 0x0F7E, 0x176F, 0x00A1,
    0x060C, 0x11C2, 0x09D3, 0x1E1D, 0x19B2, 0x0E7C, 0x166D, 0x01A3,
    0x1933, 0x0EFD, 0x16EC, 0x0122, 0x068D, 0x1143, 0x0952, 0x1E9C,
    0x0408, 0x13C6, 0x0BD7, 0x1C19, 0x1BB6, 0x0C78, 0x1469, 0x03A7,
    0x1B37, 0x0CF9, 0x14E8, 0x0326, 0x0489, 0x1347, 0x0B56, 0x1C98,
    0x1A35, 0x0DFB, 0x15EA, 0x0224, 0x058B, 0x1245, 0x0A54, 0x1D9A,
    0x050A, 0x12C4, 0x0AD5, 0x1D1B, 0x1AB4, 0x0D7A, 0x156B, 0x02A5,

    0x1021, 0x07EF, 0x1FFE, 0x0830, 0x0F9F, 0x1851, 0x0040, 0x178E,
    0x0F1E, 0x18D0, 0x00C1, 0x170F, 0x10A0, 0x076E, 0x1F7F, 0x08B1,
    0x0E1C, 0x19D2, 0x01C3, 0x160D, 0x11A2, 0x066C, 0x1E7D, 0x09B3,
    0x1123, 0x06ED, 0x1EFC, 0x0932, 0x0E9D, 0x1953, 0x0142, 0x168C,
    0x0C18, 0x1BD6, 0x03C7, 0x1409, 0x13A6, 0x0468, 0x1C79, 0x0BB7,
    0x1327, 0x04E9, 0x1CF8, 0x0B36, 0x0C99, 0x1B57, 0x0346, 0x1488,
    0x1225, 0x05EB, 0x1DFA, 0x0A34, 0x0D9B, 0x1A55, 0x0244, 0x158A,
    0x0D1A, 0x1AD4, 0x02C5, 0x150B, 0x12A4, 0x056A, 0x1D7B, 0x0AB5,

    0x0810, 0x1FDE, 0x07CF, 0x1001, 0x17AE, 0x0060, 0x1871, 0x0FBF,
    0x172F, 0x00E1, 0x18F0, 0x0F3E, 0x0891, 0x1F5F, 0x074E, 0x1080,
    0x162D, 0x01E3, 0x19F2, 0x0E3C, 0x0993, 0x1E5D, 0x064C, 0x1182,
    0x0912, 0x1EDC, 0x06CD, 0x1103, 0x16AC, 0x0162, 0x1973, 0x0EBD,
    0x1429, 0x03E7, 0x1BF6, 0x0C38, 0x0B97, 0x1C59, 0x0448, 0x1386,
    0x0B16, 0x1CD8, 0x04C9, 0x1307, 0x14A8, 0x0366, 0x1B77, 0x0CB9,
    0x0A14, 0x1DDA, 0x05CB, 0x1205, 0x15AA, 0x0264, 0x1A75, 0x0DBB,
    0x152B, 0x02E5, 0x1AF4, 0x0D3A, 0x0A95, 0x1D5B, 0x054A, 0x1284
};
#endif
//------------------------------------------------------------------------------
uint16_t hash_crc16_ccitt(const void* RESTRICT src, size_t size)
{
    /**
     * CRC-16-CCITT hashing function.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @return The hash number calculated.
     *
     * @see http://www.scadacore.com/field-applications/miscellaneous/online-checksum-calculator.html
     */
    const byte_t *buf = src;
    uint16_t      res = 0;

    while( size-- )
        res = hash_crc16_ccitt_table[ ( res & 0xFF ) ^ *buf++ ] ^ ( res >> 8 );

    return res;
}
//------------------------------------------------------------------------------
//---- CRC-16-IBM --------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef UNDEF
// 當需要節省記憶體用量時可以使用這個函式來取代下面的表格
static
uint16_t hash_crc16_ibm_table(uint8_t ind)
{
    /*
     * CRC-16-IBM polynomial can be :
     *   0x8005 : Normal
     *   0xA001 : Reversed
     *   0xC002 : Reversed Reciprocal
     */
    static const uint16_t polynomial = 0x8005;
    uint16_t res = ind;
    int      i;

    for(i=8; i; --i)
    {
        if( res & 0x01 ) res   = ( res >> 1 ) ^ polynomial;
        else             res >>= 1;
    }

    return res;
}
#else
// 當需要加速運算時可以使用這個表格來取代上面的函式
static const uint16_t hash_crc16_ibm_table[256] =
{
    0x0000, 0x9705, 0x2E01, 0xB904, 0x5C02, 0xCB07, 0x7203, 0xE506,
    0xB804, 0x2F01, 0x9605, 0x0100, 0xE406, 0x7303, 0xCA07, 0x5D02,
    0x7003, 0xE706, 0x5E02, 0xC907, 0x2C01, 0xBB04, 0x0200, 0x9505,
    0xC807, 0x5F02, 0xE606, 0x7103, 0x9405, 0x0300, 0xBA04, 0x2D01,
    0xE006, 0x7703, 0xCE07, 0x5902, 0xBC04, 0x2B01, 0x9205, 0x0500,
    0x5802, 0xCF07, 0x7603, 0xE106, 0x0400, 0x9305, 0x2A01, 0xBD04,
    0x9005, 0x0700, 0xBE04, 0x2901, 0xCC07, 0x5B02, 0xE206, 0x7503,
    0x2801, 0xBF04, 0x0600, 0x9105, 0x7403, 0xE306, 0x5A02, 0xCD07,

    0xC007, 0x5702, 0xEE06, 0x7903, 0x9C05, 0x0B00, 0xB204, 0x2501,
    0x7803, 0xEF06, 0x5602, 0xC107, 0x2401, 0xB304, 0x0A00, 0x9D05,
    0xB004, 0x2701, 0x9E05, 0x0900, 0xEC06, 0x7B03, 0xC207, 0x5502,
    0x0800, 0x9F05, 0x2601, 0xB104, 0x5402, 0xC307, 0x7A03, 0xED06,
    0x2001, 0xB704, 0x0E00, 0x9905, 0x7C03, 0xEB06, 0x5202, 0xC507,
    0x9805, 0x0F00, 0xB604, 0x2101, 0xC407, 0x5302, 0xEA06, 0x7D03,
    0x5002, 0xC707, 0x7E03, 0xE906, 0x0C00, 0x9B05, 0x2201, 0xB504,
    0xE806, 0x7F03, 0xC607, 0x5102, 0xB404, 0x2301, 0x9A05, 0x0D00,

    0x8005, 0x1700, 0xAE04, 0x3901, 0xDC07, 0x4B02, 0xF206, 0x6503,
    0x3801, 0xAF04, 0x1600, 0x8105, 0x6403, 0xF306, 0x4A02, 0xDD07,
    0xF006, 0x6703, 0xDE07, 0x4902, 0xAC04, 0x3B01, 0x8205, 0x1500,
    0x4802, 0xDF07, 0x6603, 0xF106, 0x1400, 0x8305, 0x3A01, 0xAD04,
    0x6003, 0xF706, 0x4E02, 0xD907, 0x3C01, 0xAB04, 0x1200, 0x8505,
    0xD807, 0x4F02, 0xF606, 0x6103, 0x8405, 0x1300, 0xAA04, 0x3D01,
    0x1000, 0x8705, 0x3E01, 0xA904, 0x4C02, 0xDB07, 0x6203, 0xF506,
    0xA804, 0x3F01, 0x8605, 0x1100, 0xF406, 0x6303, 0xDA07, 0x4D02,

    0x4002, 0xD707, 0x6E03, 0xF906, 0x1C00, 0x8B05, 0x3201, 0xA504,
    0xF806, 0x6F03, 0xD607, 0x4102, 0xA404, 0x3301, 0x8A05, 0x1D00,
    0x3001, 0xA704, 0x1E00, 0x8905, 0x6C03, 0xFB06, 0x4202, 0xD507,
    0x8805, 0x1F00, 0xA604, 0x3101, 0xD407, 0x4302, 0xFA06, 0x6D03,
    0xA004, 0x3701, 0x8E05, 0x1900, 0xFC06, 0x6B03, 0xD207, 0x4502,
    0x1800, 0x8F05, 0x3601, 0xA104, 0x4402, 0xD307, 0x6A03, 0xFD06,
    0xD007, 0x4702, 0xFE06, 0x6903, 0x8C05, 0x1B00, 0xA204, 0x3501,
    0x6803, 0xFF06, 0x4602, 0xD107, 0x3401, 0xA304, 0x1A00, 0x8D05,
};
#endif
//------------------------------------------------------------------------------
uint16_t hash_crc16_ibm(const void* RESTRICT src, size_t size)
{
    /**
     * CRC-16-IBM hashing function.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @return The hash number calculated.
     *
     * @see http://www.scadacore.com/field-applications/miscellaneous/online-checksum-calculator.html
     */
    const byte_t *buf = src;
    uint16_t      res = 0;

    while( size-- )
        res = hash_crc16_ibm_table[ ( res & 0xFF ) ^ *buf++ ] ^ ( res >> 8 );

    return res;
}
//------------------------------------------------------------------------------
//---- CRC-32 ------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef UNDEF
// 當需要節省記憶體用量時可以使用這個函式來取代下面的表格
static
uint32_t hash_crc32_table(uint8_t ind)
{
    /*
     * CRC-32 polynomial can be :
     *   0x04C11DB7 : Normal
     *   0xEDB88320 : Reversed
     *   0x82608EDB : Reversed Reciprocal
     */
    static const uint32_t polynomial = 0x04C11DB7;
    uint32_t res = ind;
    int      i;

    for(i=8; i; --i)
    {
        if( res & 0x01 ) res   = ( res >> 1 ) ^ polynomial;
        else             res >>= 1;
    }
    return res;
}
#else
// 當需要加速運算時可以使用這個表格來取代上面的函式
static const uint32_t hash_crc32_table[256] =
{
    0x00000000, 0x06233697, 0x05C45641, 0x03E760D6, 0x020A97ED, 0x0429A17A, 0x07CEC1AC, 0x01EDF73B,
    0x04152FDA, 0x0236194D, 0x01D1799B, 0x07F24F0C, 0x061FB837, 0x003C8EA0, 0x03DBEE76, 0x05F8D8E1,
    0x01A864DB, 0x078B524C, 0x046C329A, 0x024F040D, 0x03A2F336, 0x0581C5A1, 0x0666A577, 0x004593E0,
    0x05BD4B01, 0x039E7D96, 0x00791D40, 0x065A2BD7, 0x07B7DCEC, 0x0194EA7B, 0x02738AAD, 0x0450BC3A,
    0x0350C9B6, 0x0573FF21, 0x06949FF7, 0x00B7A960, 0x015A5E5B, 0x077968CC, 0x049E081A, 0x02BD3E8D,
    0x0745E66C, 0x0166D0FB, 0x0281B02D, 0x04A286BA, 0x054F7181, 0x036C4716, 0x008B27C0, 0x06A81157,
    0x02F8AD6D, 0x04DB9BFA, 0x073CFB2C, 0x011FCDBB, 0x00F23A80, 0x06D10C17, 0x05366CC1, 0x03155A56,
    0x06ED82B7, 0x00CEB420, 0x0329D4F6, 0x050AE261, 0x04E7155A, 0x02C423CD, 0x0123431B, 0x0700758C,

    0x06A1936C, 0x0082A5FB, 0x0365C52D, 0x0546F3BA, 0x04AB0481, 0x02883216, 0x016F52C0, 0x074C6457,
    0x02B4BCB6, 0x04978A21, 0x0770EAF7, 0x0153DC60, 0x00BE2B5B, 0x069D1DCC, 0x057A7D1A, 0x03594B8D,
    0x0709F7B7, 0x012AC120, 0x02CDA1F6, 0x04EE9761, 0x0503605A, 0x032056CD, 0x00C7361B, 0x06E4008C,
    0x031CD86D, 0x053FEEFA, 0x06D88E2C, 0x00FBB8BB, 0x01164F80, 0x07357917, 0x04D219C1, 0x02F12F56,
    0x05F15ADA, 0x03D26C4D, 0x00350C9B, 0x06163A0C, 0x07FBCD37, 0x01D8FBA0, 0x023F9B76, 0x041CADE1,
    0x01E47500, 0x07C74397, 0x04202341, 0x020315D6, 0x03EEE2ED, 0x05CDD47A, 0x062AB4AC, 0x0009823B,
    0x04593E01, 0x027A0896, 0x019D6840, 0x07BE5ED7, 0x0653A9EC, 0x00709F7B, 0x0397FFAD, 0x05B4C93A,
    0x004C11DB, 0x066F274C, 0x0588479A, 0x03AB710D, 0x02468636, 0x0465B0A1, 0x0782D077, 0x01A1E6E0,

    0x04C11DB7, 0x02E22B20, 0x01054BF6, 0x07267D61, 0x06CB8A5A, 0x00E8BCCD, 0x030FDC1B, 0x052CEA8C,
    0x00D4326D, 0x06F704FA, 0x0510642C, 0x033352BB, 0x02DEA580, 0x04FD9317, 0x071AF3C1, 0x0139C556,
    0x0569796C, 0x034A4FFB, 0x00AD2F2D, 0x068E19BA, 0x0763EE81, 0x0140D816, 0x02A7B8C0, 0x04848E57,
    0x017C56B6, 0x075F6021, 0x04B800F7, 0x029B3660, 0x0376C15B, 0x0555F7CC, 0x06B2971A, 0x0091A18D,
    0x0791D401, 0x01B2E296, 0x02558240, 0x0476B4D7, 0x059B43EC, 0x03B8757B, 0x005F15AD, 0x067C233A,
    0x0384FBDB, 0x05A7CD4C, 0x0640AD9A, 0x00639B0D, 0x018E6C36, 0x07AD5AA1, 0x044A3A77, 0x02690CE0,
    0x0639B0DA, 0x001A864D, 0x03FDE69B, 0x05DED00C, 0x04332737, 0x021011A0, 0x01F77176, 0x07D447E1,
    0x022C9F00, 0x040FA997, 0x07E8C941, 0x01CBFFD6, 0x002608ED, 0x06053E7A, 0x05E25EAC, 0x03C1683B,

    0x02608EDB, 0x0443B84C, 0x07A4D89A, 0x0187EE0D, 0x006A1936, 0x06492FA1, 0x05AE4F77, 0x038D79E0,
    0x0675A101, 0x00569796, 0x03B1F740, 0x0592C1D7, 0x047F36EC, 0x025C007B, 0x01BB60AD, 0x0798563A,
    0x03C8EA00, 0x05EBDC97, 0x060CBC41, 0x002F8AD6, 0x01C27DED, 0x07E14B7A, 0x04062BAC, 0x02251D3B,
    0x07DDC5DA, 0x01FEF34D, 0x0219939B, 0x043AA50C, 0x05D75237, 0x03F464A0, 0x00130476, 0x063032E1,
    0x0130476D, 0x071371FA, 0x04F4112C, 0x02D727BB, 0x033AD080, 0x0519E617, 0x06FE86C1, 0x00DDB056,
    0x052568B7, 0x03065E20, 0x00E13EF6, 0x06C20861, 0x072FFF5A, 0x010CC9CD, 0x02EBA91B, 0x04C89F8C,
    0x009823B6, 0x06BB1521, 0x055C75F7, 0x037F4360, 0x0292B45B, 0x04B182CC, 0x0756E21A, 0x0175D48D,
    0x048D0C6C, 0x02AE3AFB, 0x01495A2D, 0x076A6CBA, 0x06879B81, 0x00A4AD16, 0x0343CDC0, 0x0560FB57
};
#endif
//------------------------------------------------------------------------------
uint32_t hash_crc32(const void* RESTRICT src, size_t size)
{
    /**
     * CRC-32 hashing function.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @return The hash number calculated.
     *
     * @see http://www.scadacore.com/field-applications/miscellaneous/online-checksum-calculator.html
     */
    const byte_t *buf = src;
    uint32_t      res = ~0;

    while( size-- )
        res = hash_crc32_table[ ( res & 0xFF ) ^ *buf++ ] ^ ( res >> 8 );
    res ^= ~0;

    return res;
}
//------------------------------------------------------------------------------
//---- Jenkins Hash ------------------------------------------------------------
//------------------------------------------------------------------------------
uint32_t hash_jenkins(const void* RESTRICT src, size_t size)
{
    /**
     * Jenkins hashing function.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @return The hash number calculated.
     *
     * @see http://en.wikipedia.org/wiki/Jenkins_hash_function
     */
    const uint8_t *buf = src;
    uint32_t       res = 0;

    assert( src );

    while( size-- )
    {
        res += *(buf++);
        res += ( res << 10 );
        res ^= ( res >>  6 );
    }
    res += ( res <<  3 );
    res ^= ( res >> 11 );
    res += ( res << 15 );

    return res;
}
//------------------------------------------------------------------------------
//---- FNV Hash ----------------------------------------------------------------
//------------------------------------------------------------------------------
uint32_t hash_fnv32(const void* RESTRICT src, size_t size)
{
    /**
     * Fowler-Noll-Vo 32 bit hashing function (FNV-1a).
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @return The hash number calculated.
     *
     * @see http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
     */
    static const uint32_t offset_basis_32 = 0x811C9DC5;
    static const uint32_t prime_32        = 0x01000193;
    const uint8_t *buf = src;
    uint32_t       res = offset_basis_32;

    assert( src );

    while( size-- )
    {
        res ^= *(buf++);  // FNV-1 和 FNV-1a 的差異在於這兩行的順序，
        res *= prime_32;  // FNV-1 先執行 multiply 而 FNV-1a 先執行 XOR。
    }

    return res;
}
//------------------------------------------------------------------------------
//---- Murmur Hash -------------------------------------------------------------
//------------------------------------------------------------------------------
INLINE uint32_t hash_murmur3_fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85EBCA6B;
    h ^= h >> 13;
    h *= 0xC2B2AE35;
    h ^= h >> 16;

    return h;
}
//------------------------------------------------------------------------------
INLINE uint64_t hash_murmur3_fmix64(uint64_t h)
{
  h ^= h >> 33;
  h *= 0xFF51AFD7ED558CCDLL;
  h ^= h >> 33;
  h *= 0xC4CEB9FE1A85EC53LL;
  h ^= h >> 33;

  return h;
}
//------------------------------------------------------------------------------
uint32_t hash_murmur3_32(const void* RESTRICT src, size_t size, uint32_t seed)
{
    /**
     * Murmur Hash 3 - 32 bits for X86 version.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @param seed A seed number that is needed for Murmur hash.
     * @return The hash number calculated.
     */
    static const uint32_t c1 = 0xCC9E2D51;
    static const uint32_t c2 = 0x1B873593;
    size_t          nbks;
    const uint32_t *body;
    const uint8_t  *tail;
    uint32_t        k1;
    uint32_t        h1 = seed;

    // body
    body = (const uint32_t*)src;
    nbks = size >> 2;             // count of 32-bits blocks
    for( ((void)0) ; nbks ; --nbks, ++body )
    {
        k1  = ( endian_is_big_endian() )?( endian_swap_32(body[0]) ):( body[0] );

        k1 *= c1;
        k1  = bitrot32_left(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1  = bitrot32_left(h1,13);
        h1  = 5*h1 + 0xE6546B64;
    }

    // tail
    tail = (const uint8_t *)( (const uint8_t*)src + ( size & ~3 ) );
    k1   = 0;
    switch( size & 3 )
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
            k1 *= c1;
            k1  = bitrot32_left(k1,15);
            k1 *= c2;
            h1 ^= k1;
    };

    // finalization
    h1 ^= size;
    h1  = hash_murmur3_fmix32(h1);

    return h1;
}
//------------------------------------------------------------------------------
uint64_t hash_murmur3_64(const void* RESTRICT src, size_t size, uint32_t seed)
{
    /**
     * Murmur Hash 3 - 64 bits for X64 version.
     *
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @param seed A seed number that is needed for Murmur hash.
     * @return The hash number calculated.
     *
     * @remarks This is a simplification from the "128 bits for X64" version.
     */
    static const uint64_t c1 = 0x87C37B91114253D5LL;
    static const uint64_t c2 = 0x4CF5AD432745937FLL;
    size_t          nbks;
    const uint64_t *body;
    const uint8_t  *tail;
    uint64_t        k1;
    uint64_t        h1 = seed;

    // body
    body = (const uint64_t*)src;
    nbks = size >> 3;             // count of 64-bits blocks
    for( ((void)0) ; nbks ; --nbks, ++body )
    {
        k1 = ( endian_is_big_endian() )?( endian_swap_64(body[0]) ):( body[0] );

        k1 *= c1;
        k1  = bitrot64_left(k1,31);
        k1 *= c2;

        h1 ^= k1;
        h1  = bitrot64_left(h1,27);
        h1  = 5*h1 + 0x52DCE729;
    }

    // tail
    tail = (const uint8_t *)( (const uint8_t*)src + ( size & ~7 ) );
    k1   = 0;
    switch( size & 7 )
    {
    case  7: k1 ^= (uint64_t)tail[ 6] << 48;
    case  6: k1 ^= (uint64_t)tail[ 5] << 40;
    case  5: k1 ^= (uint64_t)tail[ 4] << 32;
    case  4: k1 ^= (uint64_t)tail[ 3] << 24;
    case  3: k1 ^= (uint64_t)tail[ 2] << 16;
    case  2: k1 ^= (uint64_t)tail[ 1] <<  8;
    case  1: k1 ^= (uint64_t)tail[ 0] <<  0;
             k1 *= c1;
             k1  = bitrot64_left(k1,31);
             k1 *= c2;
             h1 ^= k1;
    };

    // finalization
    h1 ^= size;
    h1 = hash_murmur3_fmix64(h1);

    return h1;
}
//------------------------------------------------------------------------------
void hash_murmur3_128(byte_t dest[16], const void* RESTRICT src, size_t size, uint32_t seed)
{
    /**
     * Murmur Hash 3 - 128 bits for X64 version.
     *
     * @param dest A buffer to receive the 16-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     * @param seed A seed number that is needed for Murmur hash.
     */
    static const uint64_t c1 = 0x87C37B91114253D5LL;
    static const uint64_t c2 = 0x4CF5AD432745937FLL;
    size_t          nbks;
    const uint64_t *body;
    const uint8_t  *tail;
    uint64_t        k1;
    uint64_t        k2;
    uint64_t        h1 = seed;
    uint64_t        h2 = seed;

    // body

    body = (const uint64_t*)src;
    nbks = size >> 4;             // count of 128-bits blocks
    for( ((void)0) ; nbks ; --nbks, body+=2 )
    {
        k1 = ( endian_is_big_endian() )?( endian_swap_64(body[0]) ):( body[0] );
        k2 = ( endian_is_big_endian() )?( endian_swap_64(body[1]) ):( body[1] );

        k1 *= c1;
        k1  = bitrot64_left(k1,31);
        k1 *= c2;
        h1 ^= k1;

        h1  = bitrot64_left(h1,27);
        h1 += h2;
        h1  = 5*h1 + 0x52DCE729;

        k2 *= c2;
        k2  = bitrot64_left(k2,33);
        k2 *= c1;
        h2 ^= k2;

        h2  = bitrot64_left(h2,31);
        h2 += h1;
        h2  = 5*h2 + 0x38495AB5;
    }

    // tail

    tail = (const uint8_t *)( (const uint8_t*)src + ( size & ~15 ) );
    k1   = 0;
    k2   = 0;

    switch( size & 15 )
    {
    case 15: k2 ^= (uint64_t)tail[14] << 48;
    case 14: k2 ^= (uint64_t)tail[13] << 40;
    case 13: k2 ^= (uint64_t)tail[12] << 32;
    case 12: k2 ^= (uint64_t)tail[11] << 24;
    case 11: k2 ^= (uint64_t)tail[10] << 16;
    case 10: k2 ^= (uint64_t)tail[ 9] <<  8;
    case  9: k2 ^= (uint64_t)tail[ 8] <<  0;
             k2 *= c2;
             k2  = bitrot64_left(k2,33);
             k2 *= c1;
             h2 ^= k2;

    case  8: k1 ^= (uint64_t)tail[ 7] << 56;
    case  7: k1 ^= (uint64_t)tail[ 6] << 48;
    case  6: k1 ^= (uint64_t)tail[ 5] << 40;
    case  5: k1 ^= (uint64_t)tail[ 4] << 32;
    case  4: k1 ^= (uint64_t)tail[ 3] << 24;
    case  3: k1 ^= (uint64_t)tail[ 2] << 16;
    case  2: k1 ^= (uint64_t)tail[ 1] <<  8;
    case  1: k1 ^= (uint64_t)tail[ 0] <<  0;
             k1 *= c1;
             k1  = bitrot64_left(k1,31);
             k1 *= c2;
             h1 ^= k1;
    };

    // finalization

    h1 ^= size;
    h2 ^= size;

    h1 += h2;
    h2 += h1;

    h1 = hash_murmur3_fmix64(h1);
    h2 = hash_murmur3_fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64_t*)dest)[0] = ( endian_is_big_endian() )?( endian_swap_64(h1) ):( h1 );
    ((uint64_t*)dest)[1] = ( endian_is_big_endian() )?( endian_swap_64(h2) ):( h2 );
}
//------------------------------------------------------------------------------
