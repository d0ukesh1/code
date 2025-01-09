#include <string.h>
#include <assert.h>
#include <stddef.h>
    /* u32 это 32-битное слово,u8 - восьми  */
typedef unsigned int  u32;
typedef unsigned char u8;
    /* определение размера массива, задан через константы */
#define CAMELLIA_TABLE_BYTE_LEN 272
#define CAMELLIA_TABLE_WORD_LEN (CAMELLIA_TABLE_BYTE_LEN / 4)
    /*Преобразования в 32бит */
#if !defined(GETU32) && !defined(PUTU32)
# define GETU32(p)   (((u32)(p)[0] << 24) ^ ((u32)(p)[1] << 16) ^ ((u32)(p)[2] <<  8) ^ ((u32)(p)[3]))
# define PUTU32(p,v) ((p)[0] = (u8)((v) >> 24), (p)[1] = (u8)((v) >> 16), (p)[2] = (u8)((v) >>  8), (p)[3] = (u8)(v))
#endif    
typedef unsigned int KEY_TABLE_TYPE[CAMELLIA_TABLE_WORD_LEN];

    /*задаем блок*/
typedef void (*block128_f)(const unsigned char in[16],
                unsigned char out[16],
                const void *key);

    /*структура нашего ключа*/
struct camellia_key_st 
        {
        union   {
            double d;   /* обеспечивает 64-битное выравнивание */
            KEY_TABLE_TYPE rd_key;
            } u;
        int grand_rounds;
        };
typedef struct camellia_key_st CAMELLIA_KEY;

   /*создаем нашу сверхпосылку*/
void random_iv(char ivc[]){
   	int  h;
   	srandom(time(NULL));
	for(h = 0; h < 16; h++){
 	ivc[h] =random();
 	}
}
    /*S-блок*/
   static const unsigned char SBOX[256] = {
112,130, 44,236,179, 39,192,229,228,133, 87, 53,234, 12,174, 65,
 35,239,107,147, 69, 25,165, 33,237, 14, 79, 78, 29,101,146,189,
134,184,175,143,124,235, 31,206, 62, 48,220, 95, 94,197, 11, 26,
166,225, 57,202,213, 71, 93, 61,217,  1, 90,214, 81, 86,108, 77,
139, 13,154,102,251,204,176, 45,116, 18, 43, 32,240,177,132,153,
223, 76,203,194, 52,126,118,  5,109,183,169, 49,209, 23,  4,215,
 20, 88, 58, 97,222, 27, 17, 28, 50, 15,156, 22, 83, 24,242, 34,
254, 68,207,178,195,181,122,145, 36,  8,232,168, 96,252,105, 80,
170,208,160,125,161,137, 98,151, 84, 91, 30,149,224,255,100,210,
 16,196,  0, 72,163,247,117,219,138,  3,230,218,  9, 63,221,148,
135, 92,131,  2,205, 74,144, 51,115,103,246,243,157,127,191,226,
 82,155,216, 38,200, 55,198, 59,129,150,111, 75, 19,190, 99, 46,
233,121,167,140,159,110,188,142, 41,245,249,182, 47,253,180, 89,
120,152,  6,106,231, 70,113,186,212, 37,171, 66,136,162,141,250,
114,  7,185, 85,248,238,172, 10, 54, 73, 42,104, 60, 56,241,164,
 64, 40,211,123,187,201, 67,193, 21,227,173,244,119,199,128,158};

#define SBOX1(n) SBOX[(n)]
#define SBOX2(n) (char)((SBOX[(n)]>>7^SBOX[(n)]<<1)&0xff)
#define SBOX3(n) (char)((SBOX[(n)]>>1^SBOX[(n)]<<7)&0xff)
#define SBOX4(n) SBOX[((n)<<1^(n)>>7)&0xff]

/* константы для ключа */
#define SIGMA1L ((u32)0xA09E667FL)
#define SIGMA1R ((u32)0x3BCC908BL)
#define SIGMA2L ((u32)0xB67AE858L)
#define SIGMA2R ((u32)0x4CAA73B2L)
#define SIGMA3L ((u32)0xC6EF372FL)
#define SIGMA3R ((u32)0xE94F82BEL)
#define SIGMA4L ((u32)0x54FF53A5L)
#define SIGMA4R ((u32)0xF1D36F1CL)
//подключи
#define SubkeyL(n) (subkey[(n)*2])
#define SubkeyR(n) (subkey[(n)*2 + 1])

/* поворот вправо сдвиг 1 байт */
#define RR8(x) (((x) >> 8) + ((x) << 24))
/* поворот влево сдвиг 1 бит */
#define RL1(x) (((x) << 1) + ((x) >> 31))
/* поворот влево сдвиг 1 байт */
#define RL8(x) (((x) << 8) + ((x) >> 24))

/* Макросы сдвига для 128-битных строк с поворотом менее 32 бит */
#define ROLDQ(ll, lr, rl, rr, w0, w1, bits)	\
    do {						\
	w0 = ll;					\
	ll = (ll << bits) + (lr >> (32 - bits));	\
	lr = (lr << bits) + (rl >> (32 - bits));	\
	rl = (rl << bits) + (rr >> (32 - bits));	\
	rr = (rr << bits) + (w0 >> (32 - bits));	\
    } while(0)

#define ROLDQo32(ll, lr, rl, rr, w0, w1, bits)	\
    do {						\
	w0 = ll;					\
	w1 = lr;					\
	ll = (lr << (bits - 32)) + (rl >> (64 - bits));	\
	lr = (rl << (bits - 32)) + (rr >> (64 - bits));	\
	rl = (rr << (bits - 32)) + (w0 >> (64 - bits));	\
	rr = (w0 << (bits - 32)) + (w1 >> (64 - bits));	\
    } while(0)
/* Функция F */
#define F(xl, xr, kl, kr, yl, yr, il, ir, t0, t1)	\
    do {							\
	il = xl ^ kl;						\
	ir = xr ^ kr;						\
	t0 = il >> 16;						\
	t1 = ir >> 16;						\
	yl = SBOX1(ir & 0xff)				\
	    ^ SBOX2((t1 >> 8) & 0xff)			\
	    ^ SBOX3(t1 & 0xff)			\
	    ^ SBOX4((ir >> 8) & 0xff);		\
	yr = SBOX1((t0 >> 8) & 0xff)			\
	    ^ SBOX2(t0 & 0xff)			\
	    ^ SBOX3((il >> 8) & 0xff)			\
	    ^ SBOX4(il & 0xff);			\
	yl ^= yr;						\
	yr = RR8(yr);					\
	yr ^= yl;						\
    } while(0)

/* Функции FL и FL^-1*/
#define FLS(ll, lr, rl, rr, kll, klr, krl, krr, t0, t1, t2, t3) \
    do {								\
	t0 = kll;							\
	t0 &= ll;							\
	lr ^= RL1(t0);						\
	t1 = klr;							\
	t1 |= lr;							\
	ll ^= t1;							\
									\
	t2 = krr;							\
	t2 |= rr;							\
	rl ^= t2;							\
	t3 = krl;							\
	t3 &= rl;							\
	rr ^= RL1(t3);						\
    } while(0)
/* Для за-/рас-шифрования */
#define ROUNDSM(xl, xr, kl, kr, yl, yr, il, ir, t0, t1)	\
    do {								\
	ir = SBOX1(xr & 0xff)					\
	    ^ SBOX2((xr >> 24) & 0xff)			\
	    ^ SBOX3((xr >> 16) & 0xff)			\
	    ^ SBOX4((xr >> 8) & 0xff);			\
	il = SBOX1((xl >> 24) & 0xff)				\
	    ^ SBOX2((xl >> 16) & 0xff)			\
	    ^ SBOX3((xl >> 8) & 0xff)				\
	    ^ SBOX4(xl & 0xff);				\
	il ^= kl;							\
	ir ^= kr;							\
	ir ^= il;							\
	il = RR8(il);						\
	il ^= ir;							\
	yl ^= ir;							\
	yr ^= il;							\
    } while(0)

/* для ключа */
#define subl(x) subL[(x)]
#define subr(x) subR[(x)]

/* Описание алгоритма */
static int Camellia_func(const unsigned char *key, u32 *subkey)
{
    u32 kll, klr, krl, krr;
    u32 il, ir, t0, t1, w0, w1;
    u32 kw4l, kw4r, dw, tl, tr;
    u32 subL[26];
    u32 subR[26];

    /* k == kll || klr || krl || krr (дробим ключ) */
    kll = GETU32(key     );
    klr = GETU32(key +  4);
    krl = GETU32(key +  8);
    krr = GETU32(key + 12);

    /* создаем KL подключи */
    subl(0) = kll; subr(0) = klr;
    subl(1) = krl; subr(1) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 15);
    subl(4) = kll; subr(4) = klr;
    subl(5) = krl; subr(5) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 30);
    subl(10) = kll; subr(10) = klr;
    subl(11) = krl; subr(11) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 15);
    subl(13) = krl; subr(13) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 17);
    subl(16) = kll; subr(16) = klr;
    subl(17) = krl; subr(17) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 17);
    subl(18) = kll; subr(18) = klr;
    subl(19) = krl; subr(19) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 17);
    subl(22) = kll; subr(22) = klr;
    subl(23) = krl; subr(23) = krr;

    /* создаем KA */
    kll = subl(0); klr = subr(0);
    krl = subl(1); krr = subr(1);
    F(kll, klr,SIGMA1L,SIGMA1R,w0, w1, il, ir, t0, t1);
    krl ^= w0; krr ^= w1;
    F(krl, krr,SIGMA2L,SIGMA2R,kll, klr, il, ir, t0, t1);
    F(kll, klr,SIGMA3L,SIGMA3R,krl, krr, il, ir, t0, t1);
    krl ^= w0; krr ^= w1;
    F(krl, krr,SIGMA4L,SIGMA4R,w0, w1, il, ir, t0, t1);
    kll ^= w0; klr ^= w1;

    /* создаем KA подключи */
    subl(2) = kll; subr(2) = klr;
    subl(3) = krl; subr(3) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 15);
    subl(6) = kll; subr(6) = klr;
    subl(7) = krl; subr(7) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 15);
    subl(8) = kll; subr(8) = klr;
    subl(9) = krl; subr(9) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 15);
    subl(12) = kll; subr(12) = klr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 15);
    subl(14) = kll; subr(14) = klr;
    subl(15) = krl; subr(15) = krr;
    ROLDQo32(kll, klr, krl, krr, w0, w1, 34);
    subl(20) = kll; subr(20) = klr;
    subl(21) = krl; subr(21) = krr;
    ROLDQ(kll, klr, krl, krr, w0, w1, 17);
    subl(24) = kll; subr(24) = klr;
    subl(25) = krl; subr(25) = krr;

    /* постбеление kw2 в другие подключи */
    subl(3) ^= subl(1); subr(3) ^= subr(1);
    subl(5) ^= subl(1); subr(5) ^= subr(1);
    subl(7) ^= subl(1); subr(7) ^= subr(1);
    subl(1) ^= subr(1) & ~subr(9);
    dw = subl(1) & subl(9), subr(1) ^= RL1(dw);
    subl(11) ^= subl(1); subr(11) ^= subr(1);
    subl(13) ^= subl(1); subr(13) ^= subr(1);
    subl(15) ^= subl(1); subr(15) ^= subr(1);
    subl(1) ^= subr(1) & ~subr(17);
    dw = subl(1) & subl(17), subr(1) ^= RL1(dw);
    subl(19) ^= subl(1); subr(19) ^= subr(1);
    subl(21) ^= subl(1); subr(21) ^= subr(1);
    subl(23) ^= subl(1); subr(23) ^= subr(1);
    subl(24) ^= subl(1); subr(24) ^= subr(1);

    /* постбеление kw4 в другие подключи */
    kw4l = subl(25); kw4r = subr(25);
    subl(22) ^= kw4l; subr(22) ^= kw4r;
    subl(20) ^= kw4l; subr(20) ^= kw4r;
    subl(18) ^= kw4l; subr(18) ^= kw4r;
    kw4l ^= kw4r & ~subr(16);
    dw = kw4l & subl(16), kw4r ^= RL1(dw);
    subl(14) ^= kw4l; subr(14) ^= kw4r;
    subl(12) ^= kw4l; subr(12) ^= kw4r;
    subl(10) ^= kw4l; subr(10) ^= kw4r;
    kw4l ^= kw4r & ~subr(8);
    dw = kw4l & subl(8), kw4r ^= RL1(dw);
    subl(6) ^= kw4l; subr(6) ^= kw4r;
    subl(4) ^= kw4l; subr(4) ^= kw4r;
    subl(2) ^= kw4l; subr(2) ^= kw4r;
    subl(0) ^= kw4l; subr(0) ^= kw4r;

    /* XOR ключа в конце F-функции */
    SubkeyL(0) = subl(0) ^ subl(2);
    SubkeyR(0) = subr(0) ^ subr(2);
    SubkeyL(2) = subl(3);
    SubkeyR(2) = subr(3);
    SubkeyL(3) = subl(2) ^ subl(4);
    SubkeyR(3) = subr(2) ^ subr(4);
    SubkeyL(4) = subl(3) ^ subl(5);
    SubkeyR(4) = subr(3) ^ subr(5);
    SubkeyL(5) = subl(4) ^ subl(6);
    SubkeyR(5) = subr(4) ^ subr(6);
    SubkeyL(6) = subl(5) ^ subl(7);
    SubkeyR(6) = subr(5) ^ subr(7);
    tl = subl(10) ^ (subr(10) & ~subr(8));
    dw = tl & subl(8), tr = subr(10) ^ RL1(dw);
    SubkeyL(7) = subl(6) ^ tl;
    SubkeyR(7) = subr(6) ^ tr;
    SubkeyL(8) = subl(8);
    SubkeyR(8) = subr(8);
    SubkeyL(9) = subl(9);
    SubkeyR(9) = subr(9);
    tl = subl(7) ^ (subr(7) & ~subr(9));
    dw = tl & subl(9), tr = subr(7) ^ RL1(dw);
    SubkeyL(10) = tl ^ subl(11);
    SubkeyR(10) = tr ^ subr(11);
    SubkeyL(11) = subl(10) ^ subl(12);
    SubkeyR(11) = subr(10) ^ subr(12);
    SubkeyL(12) = subl(11) ^ subl(13);
    SubkeyR(12) = subr(11) ^ subr(13);
    SubkeyL(13) = subl(12) ^ subl(14);
    SubkeyR(13) = subr(12) ^ subr(14);
    SubkeyL(14) = subl(13) ^ subl(15);
    SubkeyR(14) = subr(13) ^ subr(15);
    tl = subl(18) ^ (subr(18) & ~subr(16));
    dw = tl & subl(16),	tr = subr(18) ^ RL1(dw);
    SubkeyL(15) = subl(14) ^ tl;
    SubkeyR(15) = subr(14) ^ tr;
    SubkeyL(16) = subl(16);
    SubkeyR(16) = subr(16);
    SubkeyL(17) = subl(17);
    SubkeyR(17) = subr(17);
    tl = subl(15) ^ (subr(15) & ~subr(17));
    dw = tl & subl(17),	tr = subr(15) ^ RL1(dw);
    SubkeyL(18) = tl ^ subl(19);
    SubkeyR(18) = tr ^ subr(19);
    SubkeyL(19) = subl(18) ^ subl(20);
    SubkeyR(19) = subr(18) ^ subr(20);
    SubkeyL(20) = subl(19) ^ subl(21);
    SubkeyR(20) = subr(19) ^ subr(21);
    SubkeyL(21) = subl(20) ^ subl(22);
    SubkeyR(21) = subr(20) ^ subr(22);
    SubkeyL(22) = subl(21) ^ subl(23);
    SubkeyR(22) = subr(21) ^ subr(23);
    SubkeyL(23) = subl(22);
    SubkeyR(23) = subr(22);
    SubkeyL(24) = subl(24) ^ subl(23);
    SubkeyR(24) = subr(24) ^ subr(23);

    /* инверсия последней половины P-функции */
    dw = SubkeyL(2) ^ SubkeyR(2), dw = RL8(dw);
    SubkeyR(2) = SubkeyL(2) ^ dw, SubkeyL(2) = dw;
    dw = SubkeyL(3) ^ SubkeyR(3), dw = RL8(dw);
    SubkeyR(3) = SubkeyL(3) ^ dw, SubkeyL(3) = dw;
    dw = SubkeyL(4) ^ SubkeyR(4), dw = RL8(dw);
    SubkeyR(4) = SubkeyL(4) ^ dw, SubkeyL(4) = dw;
    dw = SubkeyL(5) ^ SubkeyR(5), dw = RL8(dw);
    SubkeyR(5) = SubkeyL(5) ^ dw, SubkeyL(5) = dw;
    dw = SubkeyL(6) ^ SubkeyR(6), dw = RL8(dw);
    SubkeyR(6) = SubkeyL(6) ^ dw, SubkeyL(6) = dw;
    dw = SubkeyL(7) ^ SubkeyR(7), dw = RL8(dw);
    SubkeyR(7) = SubkeyL(7) ^ dw, SubkeyL(7) = dw;
    dw = SubkeyL(10) ^ SubkeyR(10), dw = RL8(dw);
    SubkeyR(10) = SubkeyL(10) ^ dw, SubkeyL(10) = dw;
    dw = SubkeyL(11) ^ SubkeyR(11), dw = RL8(dw);
    SubkeyR(11) = SubkeyL(11) ^ dw, SubkeyL(11) = dw;
    dw = SubkeyL(12) ^ SubkeyR(12), dw = RL8(dw);
    SubkeyR(12) = SubkeyL(12) ^ dw, SubkeyL(12) = dw;
    dw = SubkeyL(13) ^ SubkeyR(13), dw = RL8(dw);
    SubkeyR(13) = SubkeyL(13) ^ dw, SubkeyL(13) = dw;
    dw = SubkeyL(14) ^ SubkeyR(14), dw = RL8(dw);
    SubkeyR(14) = SubkeyL(14) ^ dw, SubkeyL(14) = dw;
    dw = SubkeyL(15) ^ SubkeyR(15), dw = RL8(dw);
    SubkeyR(15) = SubkeyL(15) ^ dw, SubkeyL(15) = dw;
    dw = SubkeyL(18) ^ SubkeyR(18), dw = RL8(dw);
    SubkeyR(18) = SubkeyL(18) ^ dw, SubkeyL(18) = dw;
    dw = SubkeyL(19) ^ SubkeyR(19), dw = RL8(dw);
    SubkeyR(19) = SubkeyL(19) ^ dw, SubkeyL(19) = dw;
    dw = SubkeyL(20) ^ SubkeyR(20), dw = RL8(dw);
    SubkeyR(20) = SubkeyL(20) ^ dw, SubkeyL(20) = dw;
    dw = SubkeyL(21) ^ SubkeyR(21), dw = RL8(dw);
    SubkeyR(21) = SubkeyL(21) ^ dw, SubkeyL(21) = dw;
    dw = SubkeyL(22) ^ SubkeyR(22), dw = RL8(dw);
    SubkeyR(22) = SubkeyL(22) ^ dw, SubkeyL(22) = dw;
    dw = SubkeyL(23) ^ SubkeyR(23), dw = RL8(dw);
    SubkeyR(23) = SubkeyL(23) ^ dw, SubkeyL(23) = dw;
    return 0;
}
static void encrypt128(const u32 *subkey, u32 *io)
{
    u32 il, ir, t0, t1;

    /* предварительное отбеливание и поглощение kw2*/
    io[0] ^= SubkeyL(0);
    io[1] ^= SubkeyR(0);

    /* основная итерация */
    ROUNDSM(io[0],io[1],SubkeyL(2),
        SubkeyR(2),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(3),
        SubkeyR(3),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(4),
        SubkeyR(4),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(5),
        SubkeyR(5),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(6),
        SubkeyR(6),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(7),
        SubkeyR(7),io[0],io[1],il,ir,t0,t1);

    FLS(io[0],io[1],io[2],io[3],
	SubkeyL(8),SubkeyR(8),
	SubkeyL(9),SubkeyR(9),
	t0,t1,il,ir);

    ROUNDSM(io[0],io[1],SubkeyL(10),
        SubkeyR(10),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(11),
        SubkeyR(11),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(12),
        SubkeyR(12),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(13),
        SubkeyR(13),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(14),
        SubkeyR(14),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(15),
        SubkeyR(15),io[0],io[1],il,ir,t0,t1);

    FLS(io[0],io[1],io[2],io[3],
        SubkeyL(16),SubkeyR(16),
        SubkeyL(17),SubkeyR(17),
        t0,t1,il,ir);

    ROUNDSM(io[0],io[1],SubkeyL(18),
        SubkeyR(18),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(19),
        SubkeyR(19),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(20),
        SubkeyR(20),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(21),
        SubkeyR(21),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(22),
        SubkeyR(22),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(23),
        SubkeyR(23),io[0],io[1],il,ir,t0,t1);

    /* после отбеливания kw4 */
    io[2] ^= SubkeyL(24);
    io[3] ^= SubkeyR(24);

    t0 = io[0];
    t1 = io[1];
    io[0] = io[2];
    io[1] = io[3];
    io[2] = t0;
    io[3] = t1;
	
    return;
}

static void decrypt128(const u32 *subkey, u32 *io)
{
    u32 il,ir,t0,t1;
    
    /* предварительное отбеливание и поглощение kw2*/
    io[0] ^= SubkeyL(24);
    io[1] ^= SubkeyR(24);

    /* основаня итерация */
    ROUNDSM(io[0],io[1],
	SubkeyL(23),SubkeyR(23),
	io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(22),
        SubkeyR(22),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(21),
        SubkeyR(21),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(20),
        SubkeyR(20),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(19),
        SubkeyR(19),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(18),
        SubkeyR(18),io[0],io[1],il,ir,t0,t1);

    FLS(io[0],io[1],io[2],io[3],
	SubkeyL(17),SubkeyR(17),
	SubkeyL(16),SubkeyR(16),
	t0,t1,il,ir);

    ROUNDSM(io[0],io[1],SubkeyL(15),
        SubkeyR(15),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(14),
        SubkeyR(14),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(13),
        SubkeyR(13),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(12),
        SubkeyR(12),io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],SubkeyL(11),
        SubkeyR(11),io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],SubkeyL(10),
        SubkeyR(10),io[0],io[1],il,ir,t0,t1);

    FLS(io[0],io[1],io[2],io[3],
	SubkeyL(9),SubkeyR(9),
	SubkeyL(8),SubkeyR(8),
	t0,t1,il,ir);

    ROUNDSM(io[0],io[1],
		     SubkeyL(7),SubkeyR(7),
		     io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],
		     SubkeyL(6),SubkeyR(6),
		     io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],
		     SubkeyL(5),SubkeyR(5),
		     io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],
		     SubkeyL(4),SubkeyR(4),
		     io[0],io[1],il,ir,t0,t1);
    ROUNDSM(io[0],io[1],
		     SubkeyL(3),SubkeyR(3),
		     io[2],io[3],il,ir,t0,t1);
    ROUNDSM(io[2],io[3],
		     SubkeyL(2),SubkeyR(2),
		     io[0],io[1],il,ir,t0,t1);

    /* после отбеливания kw4 */
    io[2] ^= SubkeyL(0);
    io[3] ^= SubkeyR(0);

    t0 = io[0];
    t1 = io[1];
    io[0] = io[2];
    io[1] = io[3];
    io[2] = t0;
    io[3] = t1;

    return;
}
/* функция зашифровки и расшифровки блока с  */
static void EncryptBlock_Rounds(const int keyBitLength,const unsigned char *plaintext,
        const KEY_TABLE_TYPE keyTable,unsigned char *ciphertext)
{
    u32 tmp[4];

    tmp[0] = GETU32(plaintext);
    tmp[1] = GETU32(plaintext + 4);
    tmp[2] = GETU32(plaintext + 8);
    tmp[3] = GETU32(plaintext + 12);

    switch (keyBitLength) {
    case 128:
	encrypt128(keyTable, tmp);
	break;
    }
    
    PUTU32(ciphertext, tmp[0]);
    PUTU32(ciphertext + 4, tmp[1]);
    PUTU32(ciphertext + 8, tmp[2]);
    PUTU32(ciphertext + 12, tmp[3]);
}

static void DecryptBlock_Rounds(const int keyBitLength,const unsigned char *ciphertext,
        const KEY_TABLE_TYPE keyTable,unsigned char *plaintext)
{
    u32 tmp[4];

    tmp[0] = GETU32(ciphertext);
    tmp[1] = GETU32(ciphertext + 4);
    tmp[2] = GETU32(ciphertext + 8);
    tmp[3] = GETU32(ciphertext + 12);

    switch (keyBitLength) {
    case 128:
	decrypt128(keyTable, tmp);
	break;
    }
    
    PUTU32(plaintext, tmp[0]);
    PUTU32(plaintext + 4, tmp[1]);
    PUTU32(plaintext + 8, tmp[2]);
    PUTU32(plaintext + 12, tmp[3]);
}
//за-/рас-шифрование блока   
void Camellia_encrypt(const unsigned char *in, unsigned char *out,
        const CAMELLIA_KEY *key)
        {
        EncryptBlock_Rounds(128, in , key->u.rd_key , out);
        }
    
void Camellia_decrypt(const unsigned char *in, unsigned char *out,
        const CAMELLIA_KEY *key)
        {
        DecryptBlock_Rounds(128, in , key->u.rd_key , out);
        }

// генерация ключа
int set_key(const unsigned char *userKey, const int bits,
        CAMELLIA_KEY *key)
        {
        if(!userKey || !key)
            return -1;
       if(bits != 128 && bits != 192 && bits != 256)
            return -2;
        key->grand_rounds = 3;
        memset(key->u.rd_key,0,sizeof(KEY_TABLE_TYPE));
        Camellia_func(userKey,key->u.rd_key);
        return 0;
        }

//описание зашифрования для режима CBC и 128б длины ключа
void cbc128_encrypt(const unsigned char *in, unsigned char *out,
                size_t len, const void *key,
                unsigned char ivec[16], block128_f block)
    {
        size_t n;
        const unsigned char *iv = ivec;
    
        assert(in && out && key && ivec);
    
        while (len) {
           for(n=0; n<16 && n<len; ++n)
               out[n] = in[n] ^ iv[n];
           for(; n<16; ++n)
               out[n] = iv[n];
           (*block)(out, out, key);
           iv = out;
           if (len<=16) break;
           len -= 16;
           in  += 16;
           out += 16;
       }
       memcpy(ivec,iv,16);
    }
   
void cbc128_decrypt(const unsigned char *in, unsigned char *out,
               size_t len, const void *key,
               unsigned char ivec[16], block128_f block)
    {
        size_t n;
        union { size_t align; unsigned char c[16]; } tmp;
   
        assert(in && out && key && ivec);
   
        while (len) {
            unsigned char c;
            (*block)(in, tmp.c, key);
            for(n=0; n<16 && n<len; ++n) {
               c = in[n];
               out[n] = tmp.c[n] ^ ivec[n];
               ivec[n] = c;
            }
            if (len<=16) {
               for (; n<16; ++n)
               ivec[n] = in[n];
            break;
        }
        len -= 16;
        in  += 16;
        out += 16;
        }
    }

//объединение двух режимов в удобную ф-цию
void camellia_cbc_alrorithm(const unsigned char *in, unsigned char *out,
        size_t len, const CAMELLIA_KEY *key,
        unsigned char *ivec, const int enc) 
    {
        if (enc)
            cbc128_encrypt(in,out,len,key,ivec,(block128_f)Camellia_encrypt);
        else
            cbc128_decrypt(in,out,len,key,ivec,(block128_f)Camellia_decrypt);
    }
