// ISO-Designer ISO 11783   Version 5.2.0.3904 Jetter AG
// Do not change!

#include "M2GPlus.iop.h"
#include "M2GPlus.c.h"

#define WORD(w)  (unsigned char)w, (unsigned char)(w >> 8)
#define LONG(l)  (unsigned char)l, \
					(unsigned char)((unsigned long)l >> 8), \
					(unsigned char)((unsigned long)l >> 16), \
					(unsigned char)((unsigned long)l >> 24)
#define ID(id)           WORD(id)
#define REF(id)          WORD(id)
#define XYREF(id, x, y)  WORD(id), WORD(x), WORD(y)
#define MACRO(ev, id)    ev, id
#define COLOR(c)         c

const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_pt[] = {
	17,
	ID(OutputString_11001), TYPEID_OUTSTR, WORD(78), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 'C', 'o', 'n', 'f', 'i', 'g', 0,
	ID(OutputString_11002), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(10), 'I', 'n', 's', 't', 'a', 'l', 'a', '�', '�', 'o', 0,
	ID(OutputString_11003), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(12), 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', '�', '�', 'o', 0,
	ID(OutputString_11004), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(10), 'M', 'o', 'd', 'o', ' ', 'T', 'e', 's', 't', 'e', 0,
	ID(OutputString_11005), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(8), 'A', 'r', 'r', 'e', 'm', 'a', 't', 'e', 0,
	ID(OutputString_11006), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(7), 'S', 'i', 's', 't', 'e', 'm', 'a', 0,
	ID(OutputString_11007), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(11), 'P', 'l', 'a', 'n', 't', 'a', 'd', 'e', 'i', 'r', 'a', 0,
	ID(OutputString_11008), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(6), 'I', 'n', '�', 'c', 'i', 'o', 0,
	ID(OutputString_11009), TYPEID_OUTSTR, WORD(104), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(11), 'P', 'l', 'a', 'n', 't', 'a', 'd', 'e', 'i', 'r', 'a', 0,
	ID(OutputString_11010), TYPEID_OUTSTR, WORD(104), WORD(24), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(17), 'L', 'i', 'n', 'h', 'a', 's', ' ', 'A', 'l', 't', 'e', 'r', 'n', 'a', 'd', 'a', 's', 0,
	ID(OutputString_11011), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(3), 'G', 'P', 'S', 0,
	ID(OutputString_11012), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(3), 'C', 'A', 'N', 0,
	ID(OutputString_11013), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(8), 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', 0,
	ID(OutputString_11014), TYPEID_OUTSTR, WORD(104), WORD(24), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(7), 'M', 'e', 'm', '�', 'r', 'i', 'a', 0,
	ID(OutputString_11015), TYPEID_OUTSTR, WORD(74), WORD(26), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(12), 'A', 'p', 'a', 'g', 'a', 'r', ' ', 'd', 'a', 'd', 'o', 's', 0,
	ID(OutputString_11052), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 's', 't', 'a', 't', 'u', 's', 0,
	ID(OutputString_11053), TYPEID_OUTSTR, WORD(68), WORD(26), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(13), 'R', 'e', 'p', 'e', 't', 'i', 'r', ' ', 'T', 'e', 's', 't', 'e', 0,
	ID(OutputString_11054), TYPEID_OUTSTR, WORD(90), WORD(28), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(17), 'A', 'p', 'a', 'g', 'a', 'r', ' ', 'I', 'n', 's', 't', 'a', 'l', 'a', '�', '�', 'o', 0,
	ID(OutputString_11056), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(20), 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', ' ', 'I', 'n', 's', 't', 'a', 'l', 'a', 'd', 'o', 's', ':', 0,
	ID(OutputString_11057), TYPEID_OUTSTR, WORD(286), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(22), 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', ' ', 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 'd', 'o', 's', ':', 0,
	ID(OutputString_11058), TYPEID_OUTSTR, WORD(204), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(12), 'Z', 'e', 'r', 'a', 'r', ' ', 'T', 'o', 't', 'a', 'i', 's', 0,
	ID(OutputString_11059), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(45), 'T', 'e', 'm', ' ', 'c', 'e', 'r', 't', 'e', 'z', 'a', ' ', 'q', 'u', 'e', ' ', 'd', 'e', 's', 'e', 'j', 'a', ' ', 'z', 'e', 'r', 'a', 'r', ' ', 'o', ' ', 't', 'o', 't', 'a', 'l', ' ', 'p', 'a', 'r', 'c', 'i', 'a', 'l', '?', 0,
	ID(OutputString_11060), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(37), 'T', 'e', 'm', ' ', 'c', 'e', 'r', 't', 'e', 'z', 'a', ' ', 'q', 'u', 'e', ' ', 'd', 'e', 's', 'e', 'j', 'a', ' ', 'z', 'e', 'r', 'a', 'r', ' ', 'o', ' ', 't', 'o', 't', 'a', 'l', '?', 0,
	ID(OutputString_11061), TYPEID_OUTSTR, WORD(100), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(8), 'C', 'a', 'n', 'c', 'e', 'l', 'a', 'r', 0,
	ID(OutputString_11062), TYPEID_OUTSTR, WORD(84), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(7), 'A', 'c', 'e', 'i', 't', 'a', 'r', 0,
	ID(OutputString_11063), TYPEID_OUTSTR, WORD(340), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(20), 'A', 'l', 't', 'e', 'r', 'a', 'r', ' ', 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', '�', '�', 'o', 0,
	ID(OutputString_11064), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(48), 'T', 'e', 'm', ' ', 'c', 'e', 'r', 't', 'e', 'z', 'a', ' ', 'q', 'u', 'e', ' ', 'd', 'e', 's', 'e', 'j', 'a', ' ', 'a', 'l', 't', 'e', 'r', 'a', 'r', ' ', 'a', 's', ' ', 'c', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', '�', '�', 'e', 's', '?', 0,
	ID(OutputString_11065), TYPEID_OUTSTR, WORD(392), WORD(68), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(102), 'E', 's', 't', 'a', ' ', 'a', 'l', 't', 'e', 'r', 'a', '�', '�', 'o', ' ', 'a', 'p', 'a', 'g', 'a', 'r', '�', ' ', 'o', 's', ' ', 'a', 'c', 'u', 'm', 'u', 'l', 'a', 'd', 'o', 's', ' ', 'p', 'a', 'r', 'c', 'i', 'a', 'l', ' ', 'e', ' ', 't', 'o', 't', 'a', 'l', '.', ' ', 'T', 'e', 'm', ' ', 'c', 'e', 'r', 't', 'e', 'z', 'a', ' ', 'q', 'u', 'e', ' ', 'd', 'e', 's', 'e', 'j', 'a', ' ', 'a', 'l', 't', 'e', 'r', 'a', 'r', ' ', 'a', 's', ' ', 'c', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', '�', '�', 'e', 's', '?', 0,
	ID(OutputString_11066), TYPEID_OUTSTR, WORD(91), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(7), 'P', 'l', 'a', 'n', 't', 'i', 'o', 0,
	ID(OutputString_11067), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(8), 'A', 'r', 'r', 'e', 'm', 'a', 't', 'e', 0,
	ID(OutputString_11068), TYPEID_OUTSTR, WORD(80), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(11), 'Z', 'e', 'r', 'a', 'r', ' ', 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11069), TYPEID_OUTSTR, WORD(96), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(13), 'Z', 'e', 'r', 'a', 'r', ' ', 'P', 'a', 'r', 'c', 'i', 'a', 'l', 0,
	ID(OutputString_11070), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(10), 'M', 'o', 'd', 'o', ' ', 'T', 'e', 's', 't', 'e', 0,
	ID(OutputString_11071), TYPEID_OUTSTR, WORD(91), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(7), 'S', 'i', 's', 't', 'e', 'm', 'a', 0,
	ID(OutputString_11072), TYPEID_OUTSTR, WORD(94), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(13), 'T', 'r', 'o', 'c', 'a', 'r', ' ', 'S', 'e', 'n', 's', 'o', 'r', 0,
	ID(OutputString_11073), TYPEID_OUTSTR, WORD(78), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 'V', 'o', 'l', 't', 'a', 'r', 0,
	ID(OutputString_11074), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(8), 'I', 'n', 's', 't', 'a', 'l', 'a', '.', 0,
	ID(OutputString_11075), TYPEID_OUTSTR, WORD(118), WORD(35), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(15), 'F', 'i', 'n', 'a', 'l', 'i', 'z', 'a', 'r', ' ', 'T', 'e', 's', 't', 'e', 0,
	ID(OutputString_11076), TYPEID_OUTSTR, WORD(306), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 1, WORD(18), 'C', 'o', 'n', 't', 'r', 'o', 'l', 'e', ' ', 'd', 'e', ' ', 'a', 'c', 'e', 's', 's', 'o', 0,
	ID(OutputString_11077), TYPEID_OUTSTR, WORD(160), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(17), 'T', 'a', 'x', 'a', ' ', 'C', 'o', 'm', 'u', 'n', 'i', 'c', 'a', '�', '�', 'o', ':', 0,
	ID(OutputString_11078), TYPEID_OUTSTR, WORD(378), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(22), 'T', 'o', 't', 'a', 'l', ' ', 'd', 'e', ' ', 'm', 'e', 'n', 's', 'a', 'g', 'e', 'n', 's', ' ', 'T', 'X', ':', 0,
	ID(OutputString_11079), TYPEID_OUTSTR, WORD(378), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(22), 'T', 'o', 't', 'a', 'l', ' ', 'd', 'e', ' ', 'm', 'e', 'n', 's', 'a', 'g', 'e', 'n', 's', ' ', 'R', 'X', ':', 0,
	ID(OutputString_11080), TYPEID_OUTSTR, WORD(286), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 2, WORD(17), 'C', 'o', 'n', 't', 'a', 'd', 'o', 'r', ' ', 'd', 'e', ' ', 'E', 'r', 'r', 'o', ':', 0,
	ID(OutputString_11081), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(20), 'C', 'o', 'n', 't', '.', ' ', 'E', 'r', 'r', 'o', 's', ' ', 'O', 'v', 'e', 'r', 'r', 'u', 'n', ':', 0,
	ID(OutputString_11082), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(19), 'C', 'o', 'n', 't', '.', ' ', 'E', 'r', 'r', 'o', ' ', 'P', 'a', 's', 's', 'i', 'v', 'o', ':', 0,
	ID(OutputString_11083), TYPEID_OUTSTR, WORD(378), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(17), 'C', 'o', 'n', 't', '.', ' ', 'A', 'l', 'e', 'r', 't', 'a', ' ', 'B', 'u', 's', ':', 0,
	ID(OutputString_11084), TYPEID_OUTSTR, WORD(216), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(18), 'C', '�', 'd', 'i', 'g', 'o', ' ', 'd', 'o', ' ', 'V', 'e', '�', 'c', 'u', 'l', 'o', ':', 0,
	ID(OutputString_11085), TYPEID_OUTSTR, WORD(74), WORD(26), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(13), 'A', 'l', 't', 'e', 'r', 'a', 'r', ' ', 's', 'e', 'n', 'h', 'a', 0,
	ID(OutputString_11086), TYPEID_OUTSTR, WORD(255), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(15), 'M', 'o', 'n', 'i', 't', 'o', 'r', ' ', 'd', 'e', ' ', '�', 'r', 'e', 'a', 0,
	ID(OutputString_11087), TYPEID_OUTSTR, WORD(272), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(16), 'L', 'a', 'r', 'g', '.', ' ', 'i', 'm', 'p', 'l', 'e', 'm', 'e', 'n', 't', 'o', 0,
	ID(OutputString_11091), TYPEID_OUTSTR, WORD(272), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(16), 'N', '�', 'm', 'e', 'r', 'o', ' ', 'd', 'e', ' ', 'l', 'i', 'n', 'h', 'a', 's', 0,
	ID(OutputString_11092), TYPEID_OUTSTR, WORD(135), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(15), '(', '1', ' ', '-', ' ', '3', '6', ' ', 'l', 'i', 'n', 'h', 'a', 's', ')', 0,
	ID(OutputString_11093), TYPEID_OUTSTR, WORD(357), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(21), 'L', 'a', 'd', 'o', ' ', 'd', 'a', ' ', 'l', 'i', 'n', 'h', 'a', ' ', 'c', 'e', 'n', 't', 'r', 'a', 'l', 0,
	ID(OutputString_11094), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(18), 'E', 's', 'p', 'a', '�', 'a', 'm', 'e', 'n', 't', 'o', ' ', 'l', 'i', 'n', 'h', 'a', 's', 0,
	ID(OutputString_11096), TYPEID_OUTSTR, WORD(323), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(19), 'D', 'i', 's', 't', '�', 'n', 'c', 'i', 'a', ' ', 'a', 'v', 'a', 'l', 'i', 'a', '�', '�', 'o', 0,
	ID(OutputString_11098), TYPEID_OUTSTR, WORD(170), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(10), 'T', 'o', 'l', 'e', 'r', '�', 'n', 'c', 'i', 'a', 0,
	ID(OutputString_11099), TYPEID_OUTSTR, WORD(255), WORD(16), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(10), '(', '0', ' ', '-', ' ', '9', '9', ' ', '%', ')', 0,
	ID(OutputString_11100), TYPEID_OUTSTR, WORD(289), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(17), 'V', 'e', 'l', 'o', 'c', 'i', 'd', 'a', 'd', 'e', ' ', 'm', '�', 'x', 'i', 'm', 'a', 0,
	ID(OutputString_11102), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'N', '�', 'o', 0,
	ID(OutputString_11103), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'S', 'i', 'm', 0,
	ID(OutputString_11104), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'E', 's', 'q', 0,
	ID(OutputString_11105), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'D', 'i', 'r', 0,
	ID(OutputString_11106), TYPEID_OUTSTR, WORD(255), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(15), 'A', 'l', 't', 'e', 'r', 'n', 'a', 'r', ' ', 'l', 'i', 'n', 'h', 'a', 's', 0,
	ID(OutputString_11107), TYPEID_OUTSTR, WORD(289), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(17), 'L', 'i', 'n', 'h', 'a', 's', ' ', 'l', 'e', 'v', 'a', 'n', 't', 'a', 'd', 'a', 's', 0,
	ID(OutputString_11108), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'N', '�', 'o', 0,
	ID(OutputString_11109), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'S', 'i', 'm', 0,
	ID(OutputString_11110), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'P', 'a', 'r', 0,
	ID(OutputString_11111), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(5), '�', 'm', 'p', 'a', 'r', 0,
	ID(OutputString_11112), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 2, WORD(20), 'I', 'n', 'f', 'o', 'r', 'm', 'a', '�', '�', 'e', 's', ' ', 'd', 'a', ' ', 'L', 'i', 'n', 'h', 'a', 0,
	ID(OutputString_11114), TYPEID_OUTSTR, WORD(85), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(5), 'x', '1', '0', '0', '0', 0,
	ID(OutputString_11116), TYPEID_OUTSTR, WORD(51), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(3), 's', 'e', 'm', 0,
	ID(OutputString_11117), TYPEID_OUTSTR, WORD(164), WORD(39), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(17), 'P', 'o', 'p', 'u', 'l', 'a', '�', '�', 'o', ' ', 'P', 'a', 'r', 'c', 'i', 'a', 'l', 0,
	ID(OutputString_11118), TYPEID_OUTSTR, WORD(156), WORD(56), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(23), '�', 'r', 'e', 'a', ' ', 'T', 'r', 'a', 'b', 'a', 'l', 'h', 'a', 'd', 'a', ' ', 'P', 'a', 'r', 'c', 'i', 'a', 'l', 0,
	ID(OutputString_11119), TYPEID_OUTSTR, WORD(85), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(5), 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11120), TYPEID_OUTSTR, WORD(170), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(10), 'R', 'e', 'n', 'd', 'i', 'm', 'e', 'n', 't', 'o', 0,
	ID(OutputString_11121), TYPEID_OUTSTR, WORD(174), WORD(36), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(16), 'T', 'e', 'm', 'p', 'o', ' ', 'T', 'r', 'a', 'b', 'a', 'l', 'h', 'a', 'd', 'o', 0,
	ID(OutputString_11122), TYPEID_OUTSTR, WORD(142), WORD(30), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(14), 'S', 'e', 'm', 'e', 'n', 't', 'e', 's', ' ', 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11130), TYPEID_OUTSTR, WORD(16), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 's', ' ', ' ', ' ', 0,
	ID(OutputString_11131), TYPEID_OUTSTR, WORD(36), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(3), 's', 'e', 'm', 0,
	ID(OutputString_11132), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(5), 'x', '1', '0', '0', '0', 0,
	ID(OutputString_11133), TYPEID_OUTSTR, WORD(84), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(7), 'A', 'c', 'e', 'i', 't', 'a', 'r', 0,
	ID(OutputString_11134), TYPEID_OUTSTR, WORD(100), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(8), 'C', 'a', 'n', 'c', 'e', 'l', 'a', 'r', 0,
	ID(OutputString_11135), TYPEID_OUTSTR, WORD(391), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(19), 'A', 'p', 'a', 'g', 'a', 'r', ' ', 'a', ' ', 'I', 'n', 's', 't', 'a', 'l', 'a', '�', '�', 'o', 0,
	ID(OutputString_11136), TYPEID_OUTSTR, WORD(386), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(43), 'T', 'e', 'm', ' ', 'c', 'e', 'r', 't', 'e', 'z', 'a', ' ', 'q', 'u', 'e', ' ', 'd', 'e', 's', 'e', 'j', 'a', ' ', 'a', 'p', 'a', 'g', 'a', 'r', ' ', 'a', ' ', 'i', 'n', 's', 't', 'a', 'l', 'a', '�', '�', 'o', '?', 0,
	ID(OutputString_11139), TYPEID_OUTSTR, WORD(51), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(3), 'T', 'E', 'V', 0,
	ID(OutputString_11140), TYPEID_OUTSTR, WORD(68), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(4), 'M', 'T', 'E', 'V', 0,
	ID(OutputString_11141), TYPEID_OUTSTR, WORD(68), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(4), 'V', 'm', 'a', 'x', 0,
	ID(OutputString_11143), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 'V', 'e', 'l', 'o', 'c', '.', 0,
	ID(OutputString_11144), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(4), 'I', 'n', 'f', 'o', 0,
	ID(OutputString_11145), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 2, WORD(13), 'I', 'g', 'n', 'o', 'r', 'a', 'r', ' ', 'L', 'i', 'n', 'h', 'a', 0,
	ID(OutputString_11146), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(12), 'S', 'e', 'm', ' ', 'a', 'r', 'r', 'e', 'm', 'a', 't', 'e', 0,
	ID(OutputString_11147), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(13), 'L', 'a', 'd', 'o', ' ', 'e', 's', 'q', 'u', 'e', 'r', 'd', 'o', 0,
	ID(OutputString_11148), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(12), 'L', 'a', 'd', 'o', ' ', 'd', 'i', 'r', 'e', 'i', 't', 'o', 0,
	ID(OutputString_11149), TYPEID_OUTSTR, WORD(408), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(24), 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', '�', '�', 'o', ' ', 'd', 'e', ' ', 'a', 'r', 'r', 'e', 'm', 'a', 't', 'e', 0,
	ID(OutputString_11150), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(60), 'T', 'e', 'm', ' ', 'c', 'e', 'r', 't', 'e', 'z', 'a', ' ', 'q', 'u', 'e', ' ', 'd', 'e', 's', 'e', 'j', 'a', ' ', 'a', 'l', 't', 'e', 'r', 'a', 'r', ' ', 'a', 's', ' ', 'c', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', '�', '�', 'e', 's', ' ', 'd', 'e', ' ', 'a', 'r', 'r', 'e', 'm', 'a', 't', 'e', '?', 0,
	ID(OutputString_11151), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR(230), ID(FontAttributes_23001), 0,
		ID(ID_NULL), 0, WORD(15), 'M', 'o', 'n', 'i', 't', 'o', 'r', ' ', 'd', 'e', ' ', '�', 'r', 'e', 'a', 0,
	ID(OutputString_11152), TYPEID_OUTSTR, WORD(184), WORD(56), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(23), '�', 'r', 'e', 'a', ' ', 'T', 'r', 'a', 'b', 'a', 'l', 'h', 'a', 'd', 'a', ' ', 'P', 'a', 'r', 'c', 'i', 'a', 'l', 0,
	ID(OutputString_11153), TYPEID_OUTSTR, WORD(85), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(5), 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11156), TYPEID_OUTSTR, WORD(78), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 'P', 'a', 'u', 's', 'a', 'r', 0,
	ID(OutputString_11157), TYPEID_OUTSTR, WORD(136), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 1, WORD(5), 'S', 'e', 'n', 'h', 'a', 0,
	ID(OutputString_11158), TYPEID_OUTSTR, WORD(228), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(15), 'D', 'i', 'g', 'i', 't', 'e', ' ', 'a', ' ', 's', 'e', 'n', 'h', 'a', '.', 0,
	ID(OutputString_11169), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(22), 'A', ' ', 's', 'e', 'n', 'h', 'a', ' ', 'p', 'a', 'd', 'r', '�', 'o', ' ', '�', ' ', '0', '0', '0', '0', '.', 0,
	ID(OutputString_11170), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(21), 'D', 'i', 'g', 'i', 't', 'e', ' ', 'a', ' ', 's', 'e', 'n', 'h', 'a', ' ', 'a', 't', 'u', 'a', 'l', '.', 0,
	ID(OutputString_11171), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(20), 'D', 'i', 'g', 'i', 't', 'e', ' ', 'a', ' ', 'n', 'o', 'v', 'a', ' ', 's', 'e', 'n', 'h', 'a', '.', 0,
	ID(OutputString_11172), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(22), 'A', ' ', 's', 'e', 'n', 'h', 'a', ' ', 'p', 'a', 'd', 'r', '�', 'o', ' ', '�', ' ', '0', '0', '0', '0', '.', 0,
	ID(OutputString_11173), TYPEID_OUTSTR, WORD(255), WORD(24), COLOR_GREY, ID(FontAttributes_23001), 0,
		ID(ID_NULL), 1, WORD(13), 'A', 'l', 't', 'e', 'r', 'a', 'r', ' ', 's', 'e', 'n', 'h', 'a', 0,
	ID(OutputString_11174), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(16), 'S', 'e', 'n', 'h', 'a', ' ', 'i', 'n', 'c', 'o', 'r', 'r', 'e', 't', 'a', '!', 0,
	ID(OutputString_11175), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(25), 'D', 'i', 'g', 'i', 't', 'e', ' ', 'n', 'o', 'v', 'a', 'm', 'e', 'n', 't', 'e', ' ', 'a', ' ', 's', 'e', 'n', 'h', 'a', '.', 0,
	ID(OutputString_11176), TYPEID_OUTSTR, WORD(274), WORD(24), COLOR_WHITE, ID(FontAttributes_23010), 1,
		ID(ID_NULL), 0, WORD(13), 'T', 'r', 'o', 'c', 'a', 'r', ' ', 's', 'e', 'n', 's', 'o', 'r', 0,
	ID(OutputString_11177), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(19), 'P', 'o', 'r', ' ', 'f', 'a', 'v', 'o', 'r', ',', ' ', 'a', 'g', 'u', 'a', 'r', 'd', 'e', '.', 0,
	ID(OutputString_11178), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(75), 'S', 'u', 'b', 's', 't', 'i', 't', 'i', 't', 'u', 'i', '�', '�', 'o', ' ', 'n', '�', 'o', ' ', 'p', 'e', 'r', 'm', 'i', 't', 'i', 'd', 'a', '.', '\r', '\n',
		'T', 'o', 'd', 'a', 's', ' ', 'a', 's', ' ', 'l', 'i', 'n', 'h', 'a', 's', ' ', 'p', 'o', 's', 's', 'u', 'e', 'm', ' ', 's', 'e', 'n', 's', 'o', 'r', 'e', 's', ' ', 'i', 'n', 's', 't', 'a', 'l', 'a', 'd', 'o', 's', '.', 0,
	ID(OutputString_11179), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(73), 'S', 'u', 'b', 's', 't', 'i', 't', 'i', 't', 'u', 'i', '�', '�', 'o', ' ', 'n', '�', 'o', ' ', 'p', 'e', 'r', 'm', 'i', 't', 'i', 'd', 'a', '.', '\r', '\n',
		'N', 'e', 'n', 'h', 'u', 'm', ' ', 's', 'e', 'n', 's', 'o', 'r', ' ', 'n', 'o', 'v', 'o', ' ', 'f', 'o', 'i', ' ', 'e', 'n', 'c', 'o', 'n', 't', 'r', 'a', 'd', 'o', ' ', 'n', 'a', ' ', 'r', 'e', 'd', 'e', '.', 0,
	ID(OutputString_11180), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(70), 'D', 'e', 's', 'e', 'j', 'a', ' ', 's', 'u', 'b', 's', 't', 'i', 't', 'u', 'i', 'r', ' ', 'o', ' ', 's', 'e', 'n', 's', 'o', 'r', ' ', 'd', 'a', ' ', 'l', 'i', 'n', 'h', 'a', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'p', 'e', 'l', 'o', ' ', 'n', 'o', 'v', 'o', ' ', 's', 'e', 'n', 's', 'o', 'r', ' ', 'a', 'd', 'i', 'c', 'i', 'o', 'n', 'a', 'd', 'o', '?', 0,
	ID(OutputString_11181), TYPEID_OUTSTR, WORD(153), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 1, WORD(9), 'A', 'u', 't', 'e', 'q', ' ', 'M', '2', 'G', 0,
	ID(OutputString_11182), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(8), 'H', 'a', 'r', 'd', 'w', 'a', 'r', 'e', 0,
	ID(OutputString_11183), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(8), 'F', 'i', 'r', 'm', 'w', 'a', 'r', 'e', 0,
	ID(OutputString_11184), TYPEID_OUTSTR, WORD(99), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(13), 'M', 'e', 'm', '�', 'r', 'i', 'a', ' ', 'U', 's', 'a', 'd', 'a', 0,
	ID(OutputString_11185), TYPEID_OUTSTR, WORD(364), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(16), 'C', 'o', 'n', 't', '.', ' ', 'B', 'u', 's', ' ', 'D', 'e', 's', 'l', '.', ':', 0,
	ID(OutputString_11186), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(3), 'B', 'u', 's', 0,
	ID(OutputString_11187), TYPEID_OUTSTR, WORD(52), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(4), 'k', 'b', 'p', 's', 0,
	ID(OutputString_11188), TYPEID_OUTSTR, WORD(400), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(19), 'T', 'o', 't', 'a', 'l', ' ', 'd', 'e', ' ', 'm', 'e', 'n', 's', 'a', 'g', 'e', 'n', 's', ':', 0,
	ID(OutputString_11189), TYPEID_OUTSTR, WORD(214), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(10), 'C', 'A', 'N', ' ', 'I', 's', 'o', 'b', 'u', 's', 0,
	ID(OutputString_11190), TYPEID_OUTSTR, WORD(214), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(12), 'C', 'A', 'N', ' ', 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', 0,
	ID(OutputString_11191), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'P', 'D', 'O', 'P', 0,
	ID(OutputString_11192), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(3), 'N', 'S', 'V', 0,
	ID(OutputString_11193), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'E', 'R', 'R', 'P', 0,
	ID(OutputString_11194), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'E', 'R', 'R', 'V', 0,
	ID(OutputString_11195), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'M', 'O', 'D', 'O', 0,
	ID(OutputString_11196), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(3), 'A', 'N', 'T', 0,
	ID(OutputString_11197), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(10), 'V', 'e', 'l', 'o', 'c', 'i', 'd', 'a', 'd', 'e', 0,
	ID(OutputString_11198), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(5), 'B', 'B', 'R', 'A', 'M', 0,
	ID(OutputString_11199), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(6), 'V', 'E', 'R', ' ', 'F', 'W', 0,
	ID(OutputString_11200), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(10), 'I', 'n', 'f', 'o', 'r', 'm', 'a', '�', '�', 'o', 0,
	ID(OutputString_11201), TYPEID_OUTSTR, WORD(85), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 1, WORD(6), 'S', 't', 'a', 't', 'u', 's', 0,
	ID(OutputString_11202), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(8), 'L', 'a', 't', 'i', 't', 'u', 'd', 'e', 0,
	ID(OutputString_11203), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(9), 'L', 'o', 'n', 'g', 'i', 't', 'u', 'd', 'e', 0,
		ID(OutputString_11209), TYPEID_OUTSTR, WORD(50), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 1, WORD(8), 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', 0,
	ID(OutputString_11210), TYPEID_OUTSTR, WORD(50), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 1, WORD(6), 'V', 'e', 'r', 's', '�', 'o', 0,
	ID(OutputString_11211), TYPEID_OUTSTR, WORD(100), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 1, WORD(2), 'I', 'D', 0,
}; //isoOP_M2GPlus_pt

const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_pt_unit_metric[] = {
	17,
	ID(OutputString_11055), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(5), 's', 'e', 'm', '/', 'm', 0,
	ID(OutputString_11088), TYPEID_OUTSTR, WORD(126), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(14), '(', '1', ' ', '-', ' ', '6', '.', '0', '0', '0', ' ', 'c', 'm', ')', 0,
	ID(OutputString_11089), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(18), 'S', 'e', 'm', 'e', 'n', 't', 'e', 's', ' ', 'p', 'o', 'r', ' ', 'm', 'e', 't', 'r', 'o', 0,
	ID(OutputString_11090), TYPEID_OUTSTR, WORD(171), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(19), '(', '1', ',', '5', ' ', '-', ' ', '1', '0', '0', ',', '0', ' ', 's', 'e', 'm', '/', 'm', ')', 0,
	ID(OutputString_11095), TYPEID_OUTSTR, WORD(162), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(18), '(', '1', ',', '0', ' ', '-', ' ', '1', '.', '0', '0', '0', ',', '0', ' ', 'c', 'm', ')', 0,
	ID(OutputString_11097), TYPEID_OUTSTR, WORD(144), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(16), '(', '1', '0', ',', '0', ' ', '-', ' ', '1', '5', '0', ',', '0', ' ', 'm', ')', 0,
	ID(OutputString_11101), TYPEID_OUTSTR, WORD(153), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(17), '(', '0', ',', '5', ' ', '-', ' ', '2', '0', ',', '0', ' ', 'k', 'm', '/', 'h', ')', 0,
	ID(OutputString_11113), TYPEID_OUTSTR, WORD(85), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(5), 's', 'e', 'm', '/', 'm', 0,
	ID(OutputString_11115), TYPEID_OUTSTR, WORD(102), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'h', 'a', 0,
	ID(OutputString_11123), TYPEID_OUTSTR, WORD(62), WORD(18), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(5), 's', 'e', 'm', '/', 'm', 0,
	ID(OutputString_11124), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'h', 'a', 0,
	ID(OutputString_11125), TYPEID_OUTSTR, WORD(24), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'h', 'a', ' ', ' ', 0,
	ID(OutputString_11126), TYPEID_OUTSTR, WORD(20), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'm', ' ', ' ', ' ', 0,
	ID(OutputString_11127), TYPEID_OUTSTR, WORD(24), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'h', 'a', ' ', ' ', 0,
	ID(OutputString_11128), TYPEID_OUTSTR, WORD(19), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'm', ' ', ' ', ' ', 0,
	ID(OutputString_11129), TYPEID_OUTSTR, WORD(48), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'h', 'a', '/', 'h', 0,
	ID(OutputString_11137), TYPEID_OUTSTR, WORD(68), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(4), 'k', 'm', '/', 'h', 0,
	ID(OutputString_11138), TYPEID_OUTSTR, WORD(48), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'h', 'a', '/', 'h', 0,
	ID(OutputString_11142), TYPEID_OUTSTR, WORD(48), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'k', 'm', '/', 'h', 0,
	ID(OutputString_11154), TYPEID_OUTSTR, WORD(32), WORD(24), COLOR_WHITE, ID(FontAttributes_23008), 1,
		ID(ID_NULL), 0, WORD(4), 'h', 'a', ' ', ' ', 0,
	ID(OutputString_11155), TYPEID_OUTSTR, WORD(24), WORD(24), COLOR_WHITE, ID(FontAttributes_23008), 1,
		ID(ID_NULL), 0, WORD(4), 'm', ' ', ' ', ' ', 0,
};

const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_pt_unit_imperial[] = {
	17,
	ID(OutputString_11055), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'f', 't', 0,
	ID(OutputString_11088), TYPEID_OUTSTR, WORD(126), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(18), '(', '0', ',', '4', ' ', '-', ' ', '2', '.', '3', '6', '2', ',', '2', ' ', 'i', 'n', ')', 0,
	ID(OutputString_11089), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(15), 'S', 'e', 'm', 'e', 'n', 't', 'e', 's', ' ', 'p', 'o', 'r', ' ', 'p', '�', 0,
	ID(OutputString_11090), TYPEID_OUTSTR, WORD(171), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(19), '(', '0', ',', '4', ' ', '-', ' ', '3', '0', ',', '4', ' ', 's', 'e', 'm', '/', 'f', 't', ')', 0,
	ID(OutputString_11095), TYPEID_OUTSTR, WORD(162), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(16), '(', '0', ',', '4', ' ', '-', ' ', '3', '9', '3', ',', '7', ' ', 'i', 'n', ')', 0,
	ID(OutputString_11097), TYPEID_OUTSTR, WORD(144), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(17), '(', '3', '2', ',', '8', ' ', '-', ' ', '4', '9', '2', ',', '1', ' ', 'f', 't', ')', 0,
	ID(OutputString_11101), TYPEID_OUTSTR, WORD(153), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(16), '(', '0', ',', '3', ' ', '-', ' ', '1', '2', ',', '4', ' ', 'm', 'p', 'h', ')', 0,
	ID(OutputString_11113), TYPEID_OUTSTR, WORD(85), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'f', 't', 0,
	ID(OutputString_11115), TYPEID_OUTSTR, WORD(102), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'a', 'c', 0,
	ID(OutputString_11123), TYPEID_OUTSTR, WORD(62), WORD(18), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'f', 't', 0,
	ID(OutputString_11124), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'e', 'm', '/', 'a', 'c', 0,
	ID(OutputString_11125), TYPEID_OUTSTR, WORD(24), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'a', 'c', ' ', ' ', 0,
	ID(OutputString_11126), TYPEID_OUTSTR, WORD(20), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'f', 't', ' ', ' ', 0,
	ID(OutputString_11127), TYPEID_OUTSTR, WORD(24), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'a', 'c', ' ', ' ', 0,
	ID(OutputString_11128), TYPEID_OUTSTR, WORD(19), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'f', 't', ' ', ' ', 0,
	ID(OutputString_11129), TYPEID_OUTSTR, WORD(48), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'a', 'c', '/', 'h', 0,
	ID(OutputString_11137), TYPEID_OUTSTR, WORD(68), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(3), 'm', 'p', 'h', 0,
	ID(OutputString_11138), TYPEID_OUTSTR, WORD(48), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 'a', 'c', '/', 'h', 0,
	ID(OutputString_11142), TYPEID_OUTSTR, WORD(48), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(3), 'm', 'p', 'h', 0,
	ID(OutputString_11154), TYPEID_OUTSTR, WORD(32), WORD(24), COLOR_WHITE, ID(FontAttributes_23008), 1,
		ID(ID_NULL), 0, WORD(4), 'a', 'c', ' ', ' ', 0,
	ID(OutputString_11155), TYPEID_OUTSTR, WORD(24), WORD(24), COLOR_WHITE, ID(FontAttributes_23008), 1,
		ID(ID_NULL), 0, WORD(4), 'f', 't', ' ', ' ', 0,
};

unsigned int PT_PACKAGE_SIZE = sizeof(isoOP_M2GPlus_pt);
unsigned int PT_UNIT_METRIC_PKG_SIZE = sizeof(isoOP_M2GPlus_pt_unit_metric);
unsigned int PT_UNIT_IMPERIAL_PKG_SIZE = sizeof(isoOP_M2GPlus_pt_unit_imperial);