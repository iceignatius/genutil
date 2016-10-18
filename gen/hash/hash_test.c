#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "hash.h"
#include "md5.h"
#include "sha.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

//------------------------------------------------------------------------------
void bin_to_str(char *dest, size_t destsz, void *src, size_t srcsz)
{
    int i;

    assert( dest && src );
    assert( destsz >= 2*srcsz + 1 );

    for(i=0; i<srcsz; ++i, dest+=2)
        sprintf(dest, "%2.2X", ((byte_t*)src)[i]);
}
//------------------------------------------------------------------------------
bool check_result(const char *name,
                  const char *src,
                  const char *res,
                  const char *tar)
{
    bool succeed;

    succeed = !strcmp(res, tar);
    printf("%s : %s(%s)\n", (succeed?"OK":"Failed"), name, src);
    if( !succeed )
    {
        printf("  Result=%s\n", res);
        printf("  Target=%s\n", tar);
    }

    return succeed;
}
//------------------------------------------------------------------------------
bool hash_pearson_test(const char *str, const char *target)
{
    uint8_t res;
    char    strbuf[1024];

    res = hash_pearson(str, strlen(str));

    sprintf(strbuf, "%2.2X", res);
    return check_result("Pearson", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_crc16_ccitt_test(const char *str, const char *target)
{
    uint16_t res;
    char     strbuf[1024];

    res = hash_crc16_ccitt(str, strlen(str));

    sprintf(strbuf, "%4.4X", res);
    return check_result("CRC-16-CCITT", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_crc16_ibm_test(const char *str, const char *target)
{
    uint16_t res;
    char     strbuf[1024];

    res = hash_crc16_ibm(str, strlen(str));

    sprintf(strbuf, "%4.4X", res);
    return check_result("CRC-16-IBM", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_crc32_test(const char *str, const char *target)
{
    uint32_t res;
    char     strbuf[1024];

    res = hash_crc32(str, strlen(str));

    sprintf(strbuf, "%8.8X", res);
    return check_result("CRC-32", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_jenkins_test(const char *str, const char *target)
{
    uint32_t res;
    char     strbuf[1024];

    res = hash_jenkins(str, strlen(str));

    sprintf(strbuf, "%8.8X", res);
    return check_result("Jenkins", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_fnv32_test(const char *str, const char *target)
{
    uint32_t res;
    char     strbuf[1024];

    res = hash_fnv32(str, strlen(str));

    sprintf(strbuf, "%8.8X", res);
    return check_result("FNV-32", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_murmur3_32_test(const char *str, const char *target)
{
    uint32_t res;
    char     strbuf[1024];

    res = hash_murmur3_32(str, strlen(str), 0);

    sprintf(strbuf, "%8.8X", res);
    return check_result("Murmur3-32", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_murmur3_64_test(const char *str, const char *target)
{
    uint64_t res;
    char     strbuf[1024];

    res = hash_murmur3_64(str, strlen(str), 0);

    sprintf(strbuf, "%16.16llX", res);
    return check_result("Murmur3-64", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool hash_murmur3_128_test(const char *str, const char *target)
{
    byte_t res[16];
    char   strbuf[1024];

    hash_murmur3_128(res, str, strlen(str), 0);

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("Murmur3-128", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool md5_test(const char *str, const char *target)
{
    byte_t res[16];
    char   strbuf[1024];

    md5(res, str, strlen(str));

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("MD5", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool sha_1_test(const char *str, const char *target)
{
    byte_t res[20];
    char   strbuf[1024];

    sha_1(res, str, strlen(str));

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("SHA-1", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool sha_224_test(const char *str, const char *target)
{
    byte_t res[28];
    char   strbuf[1024];

    sha_224(res, str, strlen(str));

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("SHA-224", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool sha_256_test(const char *str, const char *target)
{
    byte_t res[32];
    char   strbuf[1024];

    sha_256(res, str, strlen(str));

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("SHA-256", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool sha_384_test(const char *str, const char *target)
{
    byte_t res[48];
    char   strbuf[1024];

    sha_384(res, str, strlen(str));

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("SHA-384", str, strbuf, target);
}
//------------------------------------------------------------------------------
bool sha_512_test(const char *str, const char *target)
{
    byte_t res[64];
    char   strbuf[1024];

    sha_512(res, str, strlen(str));

    bin_to_str(strbuf, sizeof(strbuf), res, sizeof(res));
    return check_result("SHA-512", str, strbuf, target);
}
//------------------------------------------------------------------------------
int main(void)
{
    assert( hash_pearson_test(""   ,"00") );
    assert( hash_pearson_test("aaa","4C") );
    assert( hash_pearson_test("aab","7C") );
    assert( hash_pearson_test("aac","F2") );

    assert( hash_crc16_ccitt_test(""   , "0000") );
    assert( hash_crc16_ccitt_test("aaa", "006B") );
    assert( hash_crc16_ccitt_test("aab", "187A") );
    assert( hash_crc16_ccitt_test("aac", "0FB4") );

    assert( hash_crc16_ibm_test(""   , "0000") );
    assert( hash_crc16_ibm_test("aaa", "30EA") );
    assert( hash_crc16_ibm_test("aab", "89EE") );
    assert( hash_crc16_ibm_test("aac", "1EEB") );

    assert( hash_crc32_test(""   , "00000000") );
    assert( hash_crc32_test("aaa", "FB17A559") );
    assert( hash_crc32_test("aab", "F8F0C58F") );
    assert( hash_crc32_test("aac", "FED3F318") );

    assert( hash_jenkins_test(""   , "00000000") );
    assert( hash_jenkins_test("aaa", "AE4F22EC") );
    assert( hash_jenkins_test("aab", "A35C0D06") );
    assert( hash_jenkins_test("aac", "6ABF1BCD") );

    assert( hash_fnv32_test(""   , "811C9DC5") );
    assert( hash_fnv32_test("aaa", "3445D362") );
    assert( hash_fnv32_test("aab", "3345D1CF") );
    assert( hash_fnv32_test("aac", "3245D03C") );

    assert( hash_murmur3_32_test(""       , "00000000") );
    assert( hash_murmur3_32_test("aaaabbc", "F359052F") );
    assert( hash_murmur3_32_test("aaaabbd", "71C3B1FF") );
    assert( hash_murmur3_32_test("aaaabbe", "A055253B") );

    assert( hash_murmur3_64_test(""           , "0000000000000000") );
    assert( hash_murmur3_64_test("aaaabbbbccd", "C09D28D867F3E39A") );
    assert( hash_murmur3_64_test("aaaabbbbcce", "EBD9695AA85252A4") );
    assert( hash_murmur3_64_test("aaaabbbbccf", "5CADF350058DA1B3") );

    assert( hash_murmur3_128_test(""                   , "00000000000000000000000000000000") );
    assert( hash_murmur3_128_test("aaaaaaaabbbbbbbbccd", "C05DEA40C32721706E79764E59C1A564") );
    assert( hash_murmur3_128_test("aaaaaaaabbbbbbbbcce", "8CCC00BA9D9A0171B683F96F61AB3F26") );
    assert( hash_murmur3_128_test("aaaaaaaabbbbbbbbccf", "ACBB2FF4A2EACF96D17508D31D60E123") );

    assert( md5_test("",
                     "D41D8CD98F00B204E9800998ECF8427E") );

    assert( md5_test("0123456789",
                     "781E5E245D69B566979B86E28D23F2C7") );

    assert( md5_test("01234567890123456789",
                     "BE497C2168E374F414A351C49379C01A") );

    assert( md5_test("012345678901234567890123456789",
                     "4F7223EBADEE9FB57B6796570D60638F") );

    assert( md5_test("0123456789012345678901234567890123456789",
                     "9F0AE0380ED27DBF6B852843D2EECE1F") );

    assert( md5_test("01234567890123456789012345678901234567890123456789",
                     "BAED005300234F3D1503C50A48CE8E6F") );

    assert( md5_test("012345678901234567890123456789012345678901234567890123456789",
                     "1CED811AF47EAD374872FCCA9D73DD71") );

    assert( md5_test("0123456789012345678901234567890123456789012345678901234567890123456789",
                     "109EB12C10C45D94DDAC8ECA7B818BED") );

    assert( sha_1_test("",
                       "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709") );

    assert( sha_1_test("0123456789",
                       "87ACEC17CD9DCD20A716CC2CF67417B71C8A7016") );

    assert( sha_1_test("01234567890123456789",
                       "88A7B59D2E9172960B72B65F7839B9DA2453F3E9") );

    assert( sha_1_test("012345678901234567890123456789",
                       "8D3A014000038725D6DAF8771B42A0784253688F") );

    assert( sha_1_test("0123456789012345678901234567890123456789",
                       "394F28745CC4992D8D43DD5F788C3366535226E3") );

    assert( sha_1_test("01234567890123456789012345678901234567890123456789",
                       "9578F951955D37F20B601C26591E260C1E5389BF") );

    assert( sha_1_test("012345678901234567890123456789012345678901234567890123456789",
                       "F52E3C2732DE7BEA28F216D877D78DAE1AA1AC6A") );

    assert( sha_1_test("0123456789012345678901234567890123456789012345678901234567890123456789",
                       "194522B2BDB1F1838A2D2D24A248202001AC6838") );

    assert( sha_224_test("",
                         "D14A028C2A3A2BC9476102BB288234C415A2B01F828EA62AC5B3E42F") );

    assert( sha_224_test("0123456789",
                         "F28AD8ECD48BA6F914C114821685AD08F0D6103649FF156599A90426") );

    assert( sha_224_test("01234567890123456789",
                         "082D4DF7B358712CB4CC4555D35049B14EAC92127C5E5AA46CA42F85") );

    assert( sha_224_test("012345678901234567890123456789",
                         "3FEB659716F642E862DC865D1C53379FDF0B0C6F3005EE64254A97F4") );

    assert( sha_224_test("0123456789012345678901234567890123456789",
                         "B23DD874AB8D4933E46E6069370FCF6E4EBF7844252893378AE6BF8B") );

    assert( sha_224_test("01234567890123456789012345678901234567890123456789",
                         "05FC4E9633F87A778E95E72198CDB526885A53325B06E29EEBFD51DC") );

    assert( sha_224_test("012345678901234567890123456789012345678901234567890123456789",
                         "AE5C0D27FE120752911C994718296A3BCCC77000AAC07B8810714932") );

    assert( sha_224_test("0123456789012345678901234567890123456789012345678901234567890123456789",
                         "BEE892A99EF946DE0A469CD2FBCE5DAF7104BDF7D1FD9E1EAA45DE38") );

    assert( sha_256_test("",
                         "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855") );

    assert( sha_256_test("0123456789",
                         "84D89877F0D4041EFB6BF91A16F0248F2FD573E6AF05C19F96BEDB9F882F7882") );

    assert( sha_256_test("01234567890123456789",
                         "4E76AD8354461437C04EF9B9B242540B6406D782FF2C3FB28AFDAB5B423F88FE") );

    assert( sha_256_test("012345678901234567890123456789",
                         "276FADFC9EDC49F5F9AF96D97636731DEF7525D4BFA16BC07699534873A474CC") );

    assert( sha_256_test("0123456789012345678901234567890123456789",
                         "FB526CD4AD0EC978C1A9E78F7C0728711139978424D618EB228BE59E21188970") );

    assert( sha_256_test("01234567890123456789012345678901234567890123456789",
                         "FB871FF8CCE8FEA83DFAEAB41784305A1461E008DC02A371ED26D856C766C903") );

    assert( sha_256_test("012345678901234567890123456789012345678901234567890123456789",
                         "5E43C8704AC81F33D701C1ACE046BA9F257062B4D17E78F3254CBF243177E4F2") );

    assert( sha_256_test("0123456789012345678901234567890123456789012345678901234567890123456789",
                         "57445FA40B08C60CDCBA7CA39C756DE614D11482E3F15A925F548688C19E58F4") );

    assert( sha_384_test("",
                         "38B060A751AC96384CD9327EB1B1E36A21FDB71114BE07434C0CC7BF63F6E1DA274EDEBFE76F65FBD51AD2F14898B95B") );

    assert( sha_384_test("0123456789",
                         "90AE531F24E48697904A4D0286F354C50A350EBB6C2B9EFCB22F71C96CEAEFFC11C6095E9CA0DF0EC30BF685DCF2E5E5") );

    assert( sha_384_test("01234567890123456789",
                         "7C80FF43A7AB351F70598C4A3803E16AFB64B836B6F15BAA9AEB5A928AB31E60EAEA5DD4F06F47A418FC41B8247DFB1B") );

    assert( sha_384_test("012345678901234567890123456789",
                         "C12CCC1B8911AEB15F01D464E9B4EB695850DA37A511FB0B16A4B4FD05A50CDB1418B704C8796DD4C5B2026FBC2A10EF") );

    assert( sha_384_test("0123456789012345678901234567890123456789",
                         "8CD3501E8117D7F1C69F2DF7AEEA6E65575EF945F6BE6E498F2D4CEA856B72CCE155FA463ADA5D87C608FC6CC3DDFE52") );

    assert( sha_384_test("01234567890123456789012345678901234567890123456789",
                         "BA1D8B63C68A06EE76E383E4F5295313248CA181FC79E55D6CA6769FE56F12B524DDA3E2F3FDE0DB445EE1C1D585816D") );

    assert( sha_384_test("012345678901234567890123456789012345678901234567890123456789",
                         "CE6BEBCE38AAD0FD35805B50F77F3E1814D46DF8E930356EC905A5D7B94BFA615FCE4C3B6CAF50EB4A7F1A1164887470") );

    assert( sha_384_test("0123456789012345678901234567890123456789012345678901234567890123456789",
                         "A913AA1DA4B49AAB503CA08E5C60CAF9325E551853F7FADAA7CF0A6D07A10B4D1534B16781861C64AD3EBEF066E02D5A") );

    assert( sha_512_test("",
                         "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E") );

    assert( sha_512_test("0123456789",
                         "BB96C2FC40D2D54617D6F276FEBE571F623A8DADF0B734855299B0E107FDA32CF6B69F2DA32B36445D73690B93CBD0F7BFC20E0F7F28553D2A4428F23B716E90") );

    assert( sha_512_test("01234567890123456789",
                         "CEF44A30B1F11D277B6E72C39B9DC61C4F8FC3BD264E771AA0EED7384A691FC7E410F489317AD2AFB74F4D7AC84A2BE7216AB4D0FFD18057DFF7BDC5E3B67B84") );

    assert( sha_512_test("012345678901234567890123456789",
                         "4F1F50282331161BE460E31C042E3ED1C36D003B7DAF08FE9E31596CA8E3508C9D15A6BC416602684DE32392A44C9D604A22A42A6611C198C223BAD9C36730DE") );

    assert( sha_512_test("0123456789012345678901234567890123456789",
                         "83311DC9BE62DA08ED1378F8B4D2188A7677E66EC196926C3CA557EA203DF3BCE9C2EFB6C986D74978C59CA1584E33E0325543EA9C48E2E4044DD65BD0068CCB") );

    assert( sha_512_test("01234567890123456789012345678901234567890123456789",
                         "2709EFA0D4B375AFD1D73C33F34728B11BE559F7797270A13E520DCA3000A6482DC84E99B38D3CDF15319CB0A76DC124D5244F41141888BB48A8D40A9DAED5FC") );

    assert( sha_512_test("012345678901234567890123456789012345678901234567890123456789",
                         "E3E33E00EEC4753EA01C134B21C52BADC44D364648BA2321FF18AA213902759B04F7F0DBFFF426ACEC097C09476ADCD0666D2D86E8CC2FCD4F7C549ACBFBFD94") );

    assert( sha_512_test("0123456789012345678901234567890123456789012345678901234567890123456789",
                         "BBB6610A951A5C7879961B47CF20052D155B88E05E4ACD9490E77496ECF8784F8F0A42707B10D743C06D5879F296047D6372EA344029E0830FF849202258815B") );

    return 0;
}
//------------------------------------------------------------------------------
