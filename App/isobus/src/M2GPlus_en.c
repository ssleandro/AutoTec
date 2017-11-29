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

const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_en[] = {
	17,
	ID(OutputString_11001), TYPEID_OUTSTR, WORD(78), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(3), 'S', 'e', 't', 0,
	ID(OutputString_11002), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(5), 'S', 'e', 't', 'u', 'p', 0,
	ID(OutputString_11003), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(13), 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', 0,
	ID(OutputString_11004), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(9), 'T', 'e', 's', 't', ' ', 'M', 'o', 'd', 'e', 0,
	ID(OutputString_11005), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(8), 'T', 'r', 'i', 'm', 'm', 'i', 'n', 'g', 0,
	ID(OutputString_11006), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 'S', 'y', 's', 't', 'e', 'm', 0,
	ID(OutputString_11007), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(7), 'P', 'l', 'a', 'n', 't', 'e', 'r', 0,
	ID(OutputString_11008), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(4), 'H', 'o', 'm', 'e', 0,
	ID(OutputString_11009), TYPEID_OUTSTR, WORD(104), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(7), 'P', 'l', 'a', 'n', 't', 'e', 'r', 0,
	ID(OutputString_11010), TYPEID_OUTSTR, WORD(104), WORD(24), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(15), 'A', 'l', 't', 'e', 'r', 'n', 'a', 't', 'e', 'd', ' ', 'R', 'o', 'w', 's', 0,
	ID(OutputString_11011), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(3), 'G', 'P', 'S', 0,
	ID(OutputString_11012), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(3), 'C', 'A', 'N', 0,
	ID(OutputString_11013), TYPEID_OUTSTR, WORD(76), WORD(14), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(7), 'S', 'e', 'n', 's', 'o', 'r', 's', 0,
	ID(OutputString_11014), TYPEID_OUTSTR, WORD(104), WORD(24), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(6), 'M', 'e', 'm', 'o', 'r', 'y', 0,
	ID(OutputString_11015), TYPEID_OUTSTR, WORD(74), WORD(26), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(10), 'E', 'r', 'a', 's', 'e', ' ', 'd', 'a', 't', 'a', 0,
	ID(OutputString_11052), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 's', 't', 'a', 't', 'u', 's', 0,
	ID(OutputString_11053), TYPEID_OUTSTR, WORD(68), WORD(26), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(11), 'R', 'e', 'p', 'e', 'a', 't', ' ', 'T', 'e', 's', 't', 0,
	ID(OutputString_11054), TYPEID_OUTSTR, WORD(90), WORD(28), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(11), 'C', 'l', 'e', 'a', 'r', ' ', 'S', 'e', 't', 'u', 'p', 0,
	ID(OutputString_11056), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(20), 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', ' ', 'I', 'n', 's', 't', 'a', 'l', 'a', 'd', 'o', 's', ':', 0,
	ID(OutputString_11057), TYPEID_OUTSTR, WORD(286), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(22), 'S', 'e', 'n', 's', 'o', 'r', 'e', 's', ' ', 'C', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 'd', 'o', 's', ':', 0,
	ID(OutputString_11058), TYPEID_OUTSTR, WORD(204), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(11), 'C', 'l', 'e', 'a', 'r', ' ', 'C', 'o', 'u', 'n', 't', 0,
	ID(OutputString_11059), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(45), 'A', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 's', 'u', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'r', 'e', 's', 'e', 't', ' ', 't', 'h', 'e', ' ', 's', 'u', 'b', ' ', 't', 'o', 't', 'a', 'l', '?', 0,
	ID(OutputString_11060), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(41), 'A', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 's', 'u', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'r', 'e', 's', 'e', 't', ' ', 't', 'h', 'e', ' ', 't', 'o', 't', 'a', 'l', '?', 0,
	ID(OutputString_11061), TYPEID_OUTSTR, WORD(100), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(6), 'C', 'a', 'n', 'c', 'e', 'l', 0,
	ID(OutputString_11062), TYPEID_OUTSTR, WORD(84), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(6), 'A', 'c', 'c', 'e', 'p', 't', 0,
	ID(OutputString_11063), TYPEID_OUTSTR, WORD(340), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(15), 'C', 'h', 'a', 'n', 'g', 'e', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 's', 0,
	ID(OutputString_11064), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(50), 'A', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 's', 'u', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'c', 'h', 'a', 'n', 'g', 'e', ' ', 't', 'h', 'e', ' ', 'c', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', '?', 0,
	ID(OutputString_11065), TYPEID_OUTSTR, WORD(392), WORD(68), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(110), 'T', 'h', 'i', 's', ' ', 'c', 'h', 'a', 'n', 'g', 'e', ' ', 'w', 'i', 'l', 'l', ' ', 'd', 'e', 'l', 'e', 't', 'e', ' ', 't', 'h', 'e', ' ', 'a', 'c', 'c', 'u', 'm', 'u', 'l', 'a', 't', 'e', 'd', ' ', 't', 'o', 't', 'a', 'l', ' ', 'a', 'n', 'd', ' ', 's', 'u', 'b', 't', 'o', 't', 'a', 'l', '.', ' ', 'A', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 's', 'u', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'c', 'h', 'a', 'n', 'g', 'e', ' ', 't', 'h', 'e', ' ', 'c', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', '?', 0,
	ID(OutputString_11066), TYPEID_OUTSTR, WORD(91), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(7), 'P', 'l', 'a', 'n', 't', 'e', 'r', 0,
	ID(OutputString_11067), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(8), 'T', 'r', 'i', 'm', 'm', 'i', 'n', 'g', 0,
	ID(OutputString_11068), TYPEID_OUTSTR, WORD(80), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(11), 'C', 'l', 'e', 'a', 'r', ' ', 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11069), TYPEID_OUTSTR, WORD(96), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(14), 'C', 'l', 'e', 'a', 'r', ' ', 'S', 'u', 'b', 't', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11070), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(9), 'T', 'e', 's', 't', ' ', 'M', 'o', 'd', 'e', 0,
	ID(OutputString_11071), TYPEID_OUTSTR, WORD(91), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 'S', 'y', 's', 't', 'e', 'm', 0,
	ID(OutputString_11072), TYPEID_OUTSTR, WORD(94), WORD(34), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(14), 'R', 'e', 'p', 'l', 'a', 'c', 'e', ' ', 'S', 'e', 'n', 's', 'o', 'r', 0,
	ID(OutputString_11073), TYPEID_OUTSTR, WORD(78), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(4), 'B', 'a', 'c', 'k', 0,
	ID(OutputString_11074), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(5), 'S', 'e', 't', 'u', 'p', 0,
	ID(OutputString_11075), TYPEID_OUTSTR, WORD(118), WORD(35), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(8), 'E', 'n', 'd', ' ', 'T', 'e', 's', 't', 0,
	ID(OutputString_11076), TYPEID_OUTSTR, WORD(306), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 1, WORD(14), 'A', 'c', 'c', 'e', 's', 's', ' ', 'M', 'a', 'n', 'a', 'g', 'e', 'r', 0,
	ID(OutputString_11077), TYPEID_OUTSTR, WORD(160), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(10), 'B', 'a', 'u', 'd', ' ', 'R', 'a', 't', 'e', ':', 0,
	ID(OutputString_11078), TYPEID_OUTSTR, WORD(378), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(18), 'T', 'X', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', 's', ' ', 'c', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11079), TYPEID_OUTSTR, WORD(378), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(18), 'R', 'X', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', 's', ' ', 'c', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11080), TYPEID_OUTSTR, WORD(286), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 2, WORD(15), 'E', 'r', 'r', 'o', 'r', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r', 's', ':', 0,
	ID(OutputString_11081), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(20), 'O', 'v', 'e', 'r', 'r', 'u', 'n', ' ', 'E', 'r', 'r', 'o', 'r', ' ', 'C', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11082), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(20), 'P', 'a', 's', 's', 'i', 'v', 'e', ' ', 'E', 'r', 'r', 'o', 'r', ' ', 'C', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11083), TYPEID_OUTSTR, WORD(378), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(18), 'B', 'u', 's', ' ', 'W', 'a', 'r', 'n', 'i', 'n', 'g', ' ', 'C', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11084), TYPEID_OUTSTR, WORD(216), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(11), 'V', 'e', 'h', 'i', 'c', 'l', 'e', ' ', 'I', 'D', ':', 0,
	ID(OutputString_11085), TYPEID_OUTSTR, WORD(74), WORD(26), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(15), 'C', 'h', 'a', 'n', 'g', 'e', ' ', 'P', 'a', 's', 's', 'w', 'o', 'r', 'd', 0,
	ID(OutputString_11086), TYPEID_OUTSTR, WORD(255), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(12), 'A', 'r', 'e', 'a', ' ', 'M', 'o', 'n', 'i', 't', 'o', 'r', 0,
	ID(OutputString_11087), TYPEID_OUTSTR, WORD(272), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(15), 'I', 'm', 'p', 'l', 'e', 'm', 'e', 'n', 't', ' ', 'w', 'i', 'd', 't', 'h', 0,
	ID(OutputString_11091), TYPEID_OUTSTR, WORD(272), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(14), 'N', 'u', 'm', 'b', 'e', 'r', ' ', 'o', 'f', ' ', 'r', 'o', 'w', 's', 0,
	ID(OutputString_11092), TYPEID_OUTSTR, WORD(135), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(13), '(', '1', ' ', '-', ' ', '3', '6', ' ', 'r', 'o', 'w', 's', ')', 0,
	ID(OutputString_11093), TYPEID_OUTSTR, WORD(357), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(15), 'C', 'e', 'n', 't', 'e', 'r', ' ', 'r', 'o', 'w', ' ', 's', 'i', 'd', 'e', 0,
	ID(OutputString_11094), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(11), 'R', 'o', 'w', ' ', 's', 'p', 'a', 'c', 'i', 'n', 'g', 0,
	ID(OutputString_11096), TYPEID_OUTSTR, WORD(323), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(19), 'E', 'v', 'a', 'l', 'u', 'a', 't', 'i', 'o', 'n', ' ', 'd', 'i', 's', 't', 'a', 'n', 'c', 'e', 0,
	ID(OutputString_11098), TYPEID_OUTSTR, WORD(170), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(9), 'T', 'o', 'l', 'e', 'r', 'a', 'n', 'c', 'e', 0,
	ID(OutputString_11099), TYPEID_OUTSTR, WORD(255), WORD(16), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(10), '(', '0', ' ', '-', ' ', '9', '9', ' ', '%', ')', 0,
	ID(OutputString_11100), TYPEID_OUTSTR, WORD(289), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(13), 'M', 'a', 'x', 'i', 'm', 'u', 'm', ' ', 's', 'p', 'e', 'e', 'd', 0,
	ID(OutputString_11102), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(2), 'N', 'o', 0,
	ID(OutputString_11103), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'Y', 'e', 's', 0,
	ID(OutputString_11104), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(4), 'L', 'e', 'f', 't', 0,
	ID(OutputString_11105), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(5), 'R', 'i', 'g', 'h', 't', 0,
	ID(OutputString_11106), TYPEID_OUTSTR, WORD(255), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(14), 'A', 'l', 't', 'e', 'r', 'n', 'a', 't', 'e', ' ', 'r', 'o', 'w', 's', 0,
	ID(OutputString_11107), TYPEID_OUTSTR, WORD(289), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(11), 'R', 'a', 'i', 's', 'e', 'd', ' ', 'r', 'o', 'w', 's', 0,
	ID(OutputString_11108), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(2), 'N', 'o', 0,
	ID(OutputString_11109), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'Y', 'e', 's', 0,
	ID(OutputString_11110), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(4), 'E', 'v', 'e', 'n', 0,
	ID(OutputString_11111), TYPEID_OUTSTR, WORD(74), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(3), 'O', 'd', 'd', 0,
	ID(OutputString_11112), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 2, WORD(16), 'L', 'i', 'n', 'e', ' ', 'I', 'n', 'f', 'o', 'r', 'm', 'a', 't', 'i', 'o', 'n', 0,
	ID(OutputString_11114), TYPEID_OUTSTR, WORD(85), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(5), 'x', '1', '0', '0', '0', 0,
	ID(OutputString_11116), TYPEID_OUTSTR, WORD(51), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(3), 's', 'd', 's', 0,
	ID(OutputString_11117), TYPEID_OUTSTR, WORD(164), WORD(39), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(18), 'P', 'a', 'r', 't', 'i', 'a', 'l', ' ', 'P', 'o', 'p', 'u', 'l', 'a', 't', 'i', 'o', 'n', 0,
	ID(OutputString_11118), TYPEID_OUTSTR, WORD(156), WORD(56), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(19), 'W', 'o', 'r', 'k', 'e', 'd', ' ', 'a', 'r', 'e', 'a', ' ', 'P', 'a', 'r', 't', 'i', 'a', 'l', 0,
	ID(OutputString_11119), TYPEID_OUTSTR, WORD(85), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(5), 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11120), TYPEID_OUTSTR, WORD(170), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(12), 'P', 'r', 'o', 'd', 'u', 'c', 't', 'i', 'v', 'i', 't', 'y', 0,
	ID(OutputString_11121), TYPEID_OUTSTR, WORD(174), WORD(36), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(11), 'W', 'o', 'r', 'k', 'e', 'd', ' ', 'T', 'i', 'm', 'e', 0,
	ID(OutputString_11122), TYPEID_OUTSTR, WORD(142), WORD(30), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(11), 'T', 'o', 't', 'a', 'l', ' ', 'S', 'e', 'e', 'd', 's', 0,
	ID(OutputString_11130), TYPEID_OUTSTR, WORD(16), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(4), 's', ' ', ' ', ' ', 0,
	ID(OutputString_11131), TYPEID_OUTSTR, WORD(36), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(3), 's', 'd', 's', 0,
	ID(OutputString_11132), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(5), 'x', '1', '0', '0', '0', 0,
	ID(OutputString_11133), TYPEID_OUTSTR, WORD(84), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(6), 'A', 'c', 'c', 'e', 'p', 't', 0,
	ID(OutputString_11134), TYPEID_OUTSTR, WORD(100), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 1, WORD(6), 'C', 'a', 'n', 'c', 'e', 'l', 0,
	ID(OutputString_11135), TYPEID_OUTSTR, WORD(391), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(23), 'D', 'e', 'l', 'e', 't', 'e', ' ', 't', 'h', 'e', ' ', 'i', 'n', 's', 't', 'a', 'l', 'l', 'a', 't', 'i', 'o', 'n', 0,
	ID(OutputString_11136), TYPEID_OUTSTR, WORD(386), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(49), 'A', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 's', 'u', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'd', 'e', 'l', 'e', 't', 'e', ' ', 't', 'h', 'e', ' ', 'i', 'n', 's', 't', 'a', 'l', 'l', 'a', 't', 'i', 'o', 'n', '?', 0,
	ID(OutputString_11139), TYPEID_OUTSTR, WORD(51), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(3), 'T', 'E', 'V', 0,
	ID(OutputString_11140), TYPEID_OUTSTR, WORD(68), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(4), 'M', 'T', 'E', 'V', 0,
	ID(OutputString_11141), TYPEID_OUTSTR, WORD(68), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(4), 'V', 'm', 'a', 'x', 0,
	ID(OutputString_11143), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(5), 'S', 'p', 'e', 'e', 'd', 0,
	ID(OutputString_11144), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(4), 'I', 'n', 'f', 'o', 0,
	ID(OutputString_11145), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 2, WORD(11), 'I', 'g', 'n', 'o', 'r', 'e', ' ', 'L', 'i', 'n', 'e', 0,
	ID(OutputString_11146), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(11), 'N', 'o', ' ', 'T', 'r', 'i', 'm', 'm', 'i', 'n', 'g', 0,
	ID(OutputString_11147), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(9), 'L', 'e', 'f', 't', ' ', 's', 'i', 'd', 'e', 0,
	ID(OutputString_11148), TYPEID_OUTSTR, WORD(260), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 3,
		ID(ID_NULL), 0, WORD(10), 'R', 'i', 'g', 'h', 't', ' ', 's', 'i', 'd', 'e', 0,
	ID(OutputString_11149), TYPEID_OUTSTR, WORD(408), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(17), 'T', 'r', 'i', 'm', 'm', 'i', 'n', 'g', ' ', 's', 'e', 't', 't', 'i', 'n', 'g', 's', 0,
	ID(OutputString_11150), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(59), 'A', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 's', 'u', 'r', 'e', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'c', 'h', 'a', 'n', 'g', 'e', ' ', 't', 'h', 'e', ' ', 't', 'r', 'i', 'm', 'm', 'i', 'n', 'g', ' ', 'c', 'o', 'n', 'f', 'i', 'g', 'u', 'r', 'a', 't', 'i', 'o', 'n', '?', 0,
	ID(OutputString_11151), TYPEID_OUTSTR, WORD(334), WORD(28), COLOR(230), ID(FontAttributes_23001), 0,
		ID(ID_NULL), 0, WORD(12), 'A', 'r', 'e', 'a', ' ', 'M', 'o', 'n', 'i', 't', 'o', 'r', 0,
	ID(OutputString_11152), TYPEID_OUTSTR, WORD(184), WORD(56), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(19), 'W', 'o', 'r', 'k', 'e', 'd', ' ', 'a', 'r', 'e', 'a', ' ', 'P', 'a', 'r', 't', 'i', 'a', 'l', 0,
	ID(OutputString_11153), TYPEID_OUTSTR, WORD(85), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 3,
		ID(ID_NULL), 0, WORD(5), 'T', 'o', 't', 'a', 'l', 0,
	ID(OutputString_11156), TYPEID_OUTSTR, WORD(78), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(5), 'P', 'a', 'u', 's', 'e', 0,
	ID(OutputString_11157), TYPEID_OUTSTR, WORD(136), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 1, WORD(8), 'P', 'a', 's', 's', 'w', 'o', 'r', 'd', 0,
	ID(OutputString_11158), TYPEID_OUTSTR, WORD(228), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(19), 'E', 'n', 't', 'e', 'r', ' ', 't', 'h', 'e', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '.', 0,
	ID(OutputString_11169), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(29), 'T', 'h', 'e', ' ', 'd', 'e', 'f', 'a', 'u', 'l', 't', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', ' ', 'i', 's', ' ', '0', '0', '0', '0', '.', 0,
	ID(OutputString_11170), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(27), 'E', 'n', 't', 'e', 'r', ' ', 't', 'h', 'e', ' ', 'c', 'u', 'r', 'r', 'e', 'n', 't', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '.', 0,
	ID(OutputString_11171), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(23), 'E', 'n', 't', 'e', 'r', ' ', 't', 'h', 'e', ' ', 'n', 'e', 'w', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '.', 0,
	ID(OutputString_11172), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(29), 'T', 'h', 'e', ' ', 'd', 'e', 'f', 'a', 'u', 'l', 't', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', ' ', 'i', 's', ' ', '0', '0', '0', '0', '.', 0,
	ID(OutputString_11173), TYPEID_OUTSTR, WORD(255), WORD(24), COLOR_GREY, ID(FontAttributes_23001), 0,
		ID(ID_NULL), 1, WORD(15), 'C', 'h', 'a', 'n', 'g', 'e', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', 0,
	ID(OutputString_11174), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(19), 'I', 'n', 'c', 'o', 'r', 'r', 'e', 'c', 't', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '!', 0,
	ID(OutputString_11175), TYPEID_OUTSTR, WORD(348), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 1, WORD(17), 'R', 'e', 'e', 'n', 't', 'e', 'r', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '.', 0,
	ID(OutputString_11176), TYPEID_OUTSTR, WORD(274), WORD(24), COLOR_WHITE, ID(FontAttributes_23010), 1,
		ID(ID_NULL), 0, WORD(14), 'R', 'e', 'p', 'l', 'a', 'c', 'e', ' ', 's', 'e', 'n', 's', 'o', 'r', 0,
	ID(OutputString_11177), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(12), 'W', 'a', 'i', 't', ' ', 'p', 'l', 'e', 'a', 's', 'e', '.', 0,
	ID(OutputString_11178), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(58), 'R', 'e', 'p', 'l', 'a', 'c', 'e', 'm', 'e', 'n', 't', ' ', 'n', 'o', 't', ' ', 'a', 'l', 'l', 'o', 'w', 'e', 'd', '.', '\r', '\n',
		'A', 'l', 'l', ' ', 'r', 'o', 'w', 's', ' ', 'h', 'a', 'v', 'e', ' ', 'i', 'n', 's', 't', 'a', 'l', 'l', 'e', 'd', ' ', 's', 'e', 'n', 's', 'o', 'r', 's', '.', 0,
	ID(OutputString_11179), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(70), 'R', 'e', 'p', 'l', 'a', 'c', 'e', 'm', 'e', 'n', 't', ' ', 'n', 'o', 't', ' ', 'a', 'l', 'l', 'o', 'w', 'e', 'd', '.', '\r', '\n',
		'N', 'o', ' ', 'n', 'e', 'w', ' ', 's', 'e', 'n', 's', 'o', 'r', ' ', 'h', 'a', 's', ' ', 'b', 'e', 'e', 'n', ' ', 'f', 'o', 'u', 'n', 'd', ' ', 'i', 'n', ' ', 't', 'h', 'e', ' ', 'n', 'e', 't', 'w', 'o', 'r', 'k', '.', 0,
	ID(OutputString_11180), TYPEID_OUTSTR, WORD(374), WORD(70), COLOR_WHITE, ID(FontAttributes_23003), 3,
		ID(ID_NULL), 0, WORD(68), 'D', 'o', ' ', 'y', 'o', 'u', ' ', 'w', 'a', 'n', 't', ' ', 't', 'o', ' ', 'r', 'e', 'p', 'l', 'a', 'c', 'e', ' ', 's', 'e', 'n', 's', 'o', 'r', ' ', 'o', 'f', ' ', 'r', 'o', 'w', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'f', 'o', 'r', ' ', 't', 'h', 'e', ' ', 'n', 'e', 'w', ' ', 's', 'e', 'n', 's', 'o', 'r', ' ', 'a', 'd', 'd', 'e', 'd', '?', 0,
	ID(OutputString_11181), TYPEID_OUTSTR, WORD(153), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 1, WORD(9), 'A', 'u', 't', 'e', 'q', ' ', 'M', '2', 'G', 0,
	ID(OutputString_11182), TYPEID_OUTSTR, WORD(117), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(9), 'H', 'a', 'r', 'd', 'w', 'a', 'r', 'e', ':', 0,
	ID(OutputString_11183), TYPEID_OUTSTR, WORD(117), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(9), 'F', 'i', 'r', 'm', 'w', 'a', 'r', 'e', ':', 0,
	ID(OutputString_11184), TYPEID_OUTSTR, WORD(99), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 3,
		ID(ID_NULL), 1, WORD(11), 'M', 'e', 'm', 'o', 'r', 'y', ' ', 'U', 's', 'e', 'd', 0,
	ID(OutputString_11185), TYPEID_OUTSTR, WORD(364), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(14), 'B', 'u', 's', ' ', 'O', 'f', 'f', ' ', 'C', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11186), TYPEID_OUTSTR, WORD(104), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(3), 'B', 'u', 's', 0,
	ID(OutputString_11187), TYPEID_OUTSTR, WORD(52), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(4), 'k', 'b', 'p', 's', 0,
	ID(OutputString_11188), TYPEID_OUTSTR, WORD(400), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(20), 'T', 'o', 't', 'a', 'l', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', ' ', 'c', 'o', 'u', 'n', 't', ':', 0,
	ID(OutputString_11189), TYPEID_OUTSTR, WORD(214), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(10), 'C', 'A', 'N', ' ', 'I', 's', 'o', 'b', 'u', 's', 0,
	ID(OutputString_11190), TYPEID_OUTSTR, WORD(214), WORD(34), COLOR_WHITE, ID(FontAttributes_23003), 0,
		ID(ID_NULL), 0, WORD(11), 'C', 'A', 'N', ' ', 'S', 'e', 'n', 's', 'o', 'r', 's', 0,
	ID(OutputString_11191), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'P', 'D', 'O', 'P', 0,
	ID(OutputString_11192), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(3), 'N', 'S', 'V', 0,
	ID(OutputString_11193), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'E', 'R', 'R', 'P', 0,
	ID(OutputString_11194), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'E', 'R', 'R', 'V', 0,
	ID(OutputString_11195), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(4), 'M', 'O', 'D', 'E', 0,
	ID(OutputString_11196), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(3), 'A', 'N', 'T', 0,
	ID(OutputString_11197), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(5), 'S', 'p', 'e', 'e', 'd', 0,
	ID(OutputString_11198), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(5), 'B', 'B', 'R', 'A', 'M', 0,
	ID(OutputString_11199), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(6), 'V', 'E', 'R', ' ', 'F', 'W', 0,
	ID(OutputString_11200), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(11), 'I', 'n', 'f', 'o', 'r', 'm', 'a', 't', 'i', 'o', 'n', 0,
	ID(OutputString_11201), TYPEID_OUTSTR, WORD(85), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 1, WORD(6), 'S', 't', 'a', 't', 'u', 's', 0,
	ID(OutputString_11202), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(8), 'L', 'a', 't', 'i', 't', 'u', 'd', 'e', 0,
	ID(OutputString_11203), TYPEID_OUTSTR, WORD(156), WORD(20), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 2, WORD(9), 'L', 'o', 'n', 'g', 'i', 't', 'u', 'd', 'e', 0,
	ID(OutputString_11209), TYPEID_OUTSTR, WORD(50), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 1, WORD(7), 'S', 'e', 'n', 's', 'o', 'r', 's', 0,
	ID(OutputString_11210), TYPEID_OUTSTR, WORD(50), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 1, WORD(7), 'V', 'e', 'r', 's', 'i', 'o', 'n', 0,
	ID(OutputString_11211), TYPEID_OUTSTR, WORD(100), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 1, WORD(2), 'I', 'D', 0,
}; //isoOP_M2GPlus_en

const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_en_unit_metric[] = {
	17,
	ID(OutputString_11055), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(5), 's', 'd', 's', '/', 'm', 0,
	ID(OutputString_11088), TYPEID_OUTSTR, WORD(126), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(14), '(', '1', ' ', '-', ' ', '6', ',', '0', '0', '0', ' ', 'c', 'm', ')', 0,
	ID(OutputString_11089), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(15), 'S', 'e', 'e', 'd', 's', ' ', 'p', 'e', 'r', ' ', 'm', 'e', 't', 'e', 'r', 0,
	ID(OutputString_11090), TYPEID_OUTSTR, WORD(171), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(19), '(', '1', '.', '5', ' ', '-', ' ', '1', '0', '0', '.', '0', ' ', 's', 'd', 's', '/', 'm', ')', 0,
	ID(OutputString_11095), TYPEID_OUTSTR, WORD(162), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(18), '(', '1', '.', '0', ' ', '-', ' ', '1', ',', '0', '0', '0', '.', '0', ' ', 'c', 'm', ')', 0,
	ID(OutputString_11097), TYPEID_OUTSTR, WORD(144), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(16), '(', '1', '0', '.', '0', ' ', '-', ' ', '1', '5', '0', '.', '0', ' ', 'm', ')', 0,
	ID(OutputString_11101), TYPEID_OUTSTR, WORD(153), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(17), '(', '0', '.', '5', ' ', '-', ' ', '2', '0', '.', '0', ' ', 'k', 'm', '/', 'h', ')', 0,
	ID(OutputString_11113), TYPEID_OUTSTR, WORD(85), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(5), 's', 'd', 's', '/', 'm', 0,
	ID(OutputString_11115), TYPEID_OUTSTR, WORD(102), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'h', 'a', 0,
	ID(OutputString_11123), TYPEID_OUTSTR, WORD(62), WORD(18), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(5), 's', 'd', 's', '/', 'm', 0,
	ID(OutputString_11124), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'h', 'a', 0,
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

const unsigned char ISO_OP_MEMORY_CLASS isoOP_M2GPlus_en_unit_imperial[] = {
	17,
	ID(OutputString_11055), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23002), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'f', 't', 0,
	ID(OutputString_11088), TYPEID_OUTSTR, WORD(126), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(18), '(', '0', '.', '4', ' ', '-', ' ', '2', ',', '3', '6', '2', '.', '2', ' ', 'i', 'n', ')', 0,
	ID(OutputString_11089), TYPEID_OUTSTR, WORD(306), WORD(16), COLOR_WHITE, ID(FontAttributes_23004), 1,
		ID(ID_NULL), 0, WORD(14), 'S', 'e', 'e', 'd', 's', ' ', 'p', 'e', 'r', ' ', 'f', 'o', 'o', 't', 0,
	ID(OutputString_11090), TYPEID_OUTSTR, WORD(171), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(19), '(', '0', '.', '4', ' ', '-', ' ', '3', '0', '.', '4', ' ', 's', 'e', 'm', '/', 'f', 't', ')', 0,
	ID(OutputString_11095), TYPEID_OUTSTR, WORD(162), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(16), '(', '0', '.', '4', ' ', '-', ' ', '3', '9', '3', '.', '7', ' ', 'i', 'n', ')', 0,
	ID(OutputString_11097), TYPEID_OUTSTR, WORD(144), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(17), '(', '3', '2', '.', '8', ' ', '-', ' ', '4', '9', '2', '.', '1', ' ', 'f', 't', ')', 0,
	ID(OutputString_11101), TYPEID_OUTSTR, WORD(153), WORD(12), COLOR_WHITE, ID(FontAttributes_23000), 1,
		ID(ID_NULL), 0, WORD(16), '(', '0', '.', '3', ' ', '-', ' ', '1', '2', '.', '4', ' ', 'm', 'p', 'h', ')', 0,
	ID(OutputString_11113), TYPEID_OUTSTR, WORD(85), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'f', 't', 0,
	ID(OutputString_11115), TYPEID_OUTSTR, WORD(102), WORD(24), COLOR_WHITE, ID(FontAttributes_23001), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'a', 'c', 0,
	ID(OutputString_11123), TYPEID_OUTSTR, WORD(62), WORD(18), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'f', 't', 0,
	ID(OutputString_11124), TYPEID_OUTSTR, WORD(72), WORD(16), COLOR_WHITE, ID(FontAttributes_23003), 1,
		ID(ID_NULL), 0, WORD(6), 's', 'd', 's', '/', 'a', 'c', 0,
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

unsigned int EN_PACKAGE_SIZE = sizeof(isoOP_M2GPlus_en);
unsigned int EN_UNIT_METRIC_PKG_SIZE = sizeof(isoOP_M2GPlus_en_unit_metric);
unsigned int EN_UNIT_IMPERIAL_PKG_SIZE = sizeof(isoOP_M2GPlus_en_unit_imperial);