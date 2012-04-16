/* Header for module lyrics, generated by p2c 1.21alpha-07.Dec.93 */
#ifndef LYRICS_H
#define LYRICS_H


/* Everything related to lyrics */

#ifndef GLOBALS_H
#include "globals.h"
#endif


#ifdef LYRICS_G
# define vextern
#else
# define vextern extern
#endif

extern Void initLyrics PV();
extern Void clearTags PV();
extern Void reviseLyrics PV();
extern Void lyrTranslate PP((Char *P, boolean *numbered));
extern Void getSyllable PP((int voice, Char *pretex));
extern Char *lyricsReport PP((Char *Result, int voice));
extern Void maybeLyrics PP((int voice, int parline, Char *w));
extern Void extractLyrtag PP((int voice, Char *note));
extern Void lyricsAdjust PP((int voice, Char *note));
extern Void assignLyrics PP((int stave, Char *lyrassign));
extern Void lyricsParagraph PV();
extern boolean hasVerseNumber PP((int voice));


#undef vextern

#endif /*LYRICS_H*/

/* End. */
