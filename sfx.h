/* 光路反射的音效。純資料，實機才會播。 */
#ifndef LP_SFX_H
#define LP_SFX_H

#include "audio.h"

static const rd_note SFX_MOVE[]   = { {1400, 12}, {0, 0} };
static const rd_note SFX_ROTATE[] = { {880, 25}, {1320, 35}, {0, 0} };
static const rd_note SFX_TAKE[]   = { {990, 30}, {660, 40}, {0, 0} };
static const rd_note SFX_PLACE[]  = { {660, 30}, {990, 40}, {0, 0} };
static const rd_note SFX_DENY[]   = { {160, 70}, {0, 0} };
static const rd_note SFX_LIT[]    = { {1320, 40}, {1760, 90}, {0, 0} };
static const rd_note SFX_CLEAR[]  = {
    {523, 90}, {659, 90}, {784, 90}, {1047, 220}, {0, 0}
};
static const rd_note SFX_ALL[]    = {
    {523, 110}, {659, 110}, {784, 110}, {1047, 110},
    {0, 60}, {784, 110}, {1047, 400}, {0, 0}
};

#endif
