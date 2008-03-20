
/*
 *
 * (C) Copyright IBM Corp. 1998-2008 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "LEScripts.h"
#include "LELanguages.h"

#include "LayoutEngine.h"
#include "CanonShaping.h"
#include "OpenTypeLayoutEngine.h"
#include "ScriptAndLanguageTags.h"
#include "CharSubstitutionFilter.h"

#include "GlyphSubstitutionTables.h"
#include "GlyphDefinitionTables.h"
#include "GlyphPositioningTables.h"

#include "LEGlyphStorage.h"
#include "GlyphPositionAdjustments.h"

#include "GDEFMarkFilter.h"

U_NAMESPACE_BEGIN

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(OpenTypeLayoutEngine)

#define ccmpFeatureTag LE_CCMP_FEATURE_TAG
#define ligaFeatureTag LE_LIGA_FEATURE_TAG
#define cligFeatureTag LE_CLIG_FEATURE_TAG
#define kernFeatureTag LE_KERN_FEATURE_TAG
#define markFeatureTag LE_MARK_FEATURE_TAG
#define mkmkFeatureTag LE_MKMK_FEATURE_TAG
#define loclFeatureTag LE_LOCL_FEATURE_TAG

// 'dlig' not used at the moment
#define dligFeatureTag 0x646C6967

// 'palt'
#define paltFeatureTag 0x70616C74

#define ccmpFeatureMask 0x80000000UL
#define ligaFeatureMask 0x40000000UL
#define cligFeatureMask 0x20000000UL
#define kernFeatureMask 0x10000000UL
#define paltFeatureMask 0x08000000UL
#define markFeatureMask 0x04000000UL
#define mkmkFeatureMask 0x02000000UL
#define loclFeatureMask 0x01000000UL

#define minimalFeatures     (ccmpFeatureMask | markFeatureMask | mkmkFeatureMask | loclFeatureMask)
#define ligaFeatures        (ligaFeatureMask | cligFeatureMask | minimalFeatures)
#define kernFeatures        (kernFeatureMask | paltFeatureMask | minimalFeatures)
#define kernAndLigaFeatures (ligaFeatures    | kernFeatures)
 
static const FeatureMap featureMap[] =
{
    {ccmpFeatureTag, ccmpFeatureMask},
    {ligaFeatureTag, ligaFeatureMask},
    {cligFeatureTag, cligFeatureMask}, 
    {kernFeatureTag, kernFeatureMask},
    {paltFeatureTag, paltFeatureMask},
    {markFeatureTag, markFeatureMask},
    {mkmkFeatureTag, mkmkFeatureMask},
    {loclFeatureTag, loclFeatureMask}
};

static const le_int32 featureMapCount = LE_ARRAY_SIZE(featureMap);

OpenTypeLayoutEngine::OpenTypeLayoutEngine(const LEFontInstance *fontInstance, le_int32 scriptCode, le_int32 languageCode,
                        le_int32 typoFlags, const GlyphSubstitutionTableHeader *gsubTable)
    : LayoutEngine(fontInstance, scriptCode, languageCode, typoFlags), fFeatureMask(minimalFeatures),
      fFeatureMap(featureMap), fFeatureParamList(NULL), fFeatureMapCount(featureMapCount), fFeatureOrder(FALSE),
      fGSUBTable(gsubTable), fGDEFTable(NULL), fGPOSTable(NULL), fSubstitutionFilter(NULL)
{
    static const le_uint32 gdefTableTag = LE_GDEF_TABLE_TAG;
    static const le_uint32 gposTableTag = LE_GPOS_TABLE_TAG;
    const GlyphPositioningTableHeader *gposTable = (const GlyphPositioningTableHeader *) getFontTable(gposTableTag);

    // todo: switch to more flags and bitfield rather than list of feature tags?
    switch (typoFlags & ~0x80000000L) {
    case 0: break; // default
    case 1: fFeatureMask = kernFeatures; break;
    case 2: fFeatureMask = ligaFeatures; break;
    case 3: fFeatureMask = kernAndLigaFeatures; break;
    default: break;
    }

    if (typoFlags & 0x80000000L) {
        fSubstitutionFilter = new CharSubstitutionFilter(fontInstance);
    }

    setScriptAndLanguageTags();

    fGDEFTable = (const GlyphDefinitionTableHeader *) getFontTable(gdefTableTag);
    
    if (gposTable != NULL && gposTable->coversScriptAndLanguage(fScriptTag, fLangSysTag)) {
        fGPOSTable = gposTable;
    }
}

void OpenTypeLayoutEngine::reset()
{
    // NOTE: if we're called from
    // the destructor, LayoutEngine;:reset()
    // will have been called already by
    // LayoutEngine::~LayoutEngine()
    LayoutEngine::reset();
}

OpenTypeLayoutEngine::OpenTypeLayoutEngine(const LEFontInstance *fontInstance, le_int32 scriptCode, le_int32 languageCode,
                       le_int32 typoFlags)
    : LayoutEngine(fontInstance, scriptCode, languageCode, typoFlags), fFeatureOrder(FALSE),
      fGSUBTable(NULL), fGDEFTable(NULL), fGPOSTable(NULL), fSubstitutionFilter(NULL)
{
    setScriptAndLanguageTags();
}

OpenTypeLayoutEngine::~OpenTypeLayoutEngine()
{
    if (fTypoFlags & 0x80000000L) {
        delete fSubstitutionFilter;
    }

    reset();
}

LETag OpenTypeLayoutEngine::getScriptTag(le_int32 scriptCode)
{
    if (scriptCode < 0 || scriptCode >= scriptCodeCount) {
        return 0xFFFFFFFF;
    }

    return scriptTags[scriptCode];
}

LETag OpenTypeLayoutEngine::getLangSysTag(le_int32 languageCode)
{
    if (languageCode < 0 || languageCode >= languageCodeCount) {
        return 0xFFFFFFFF;
    }

    return languageTags[languageCode];
}

void OpenTypeLayoutEngine::setScriptAndLanguageTags()
{
    fScriptTag  = getScriptTag(fScriptCode);
    fLangSysTag = getLangSysTag(fLanguageCode);
}

le_int32 OpenTypeLayoutEngine::characterProcessing(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft,
                LEUnicode *&outChars, LEGlyphStorage &glyphStorage, LEErrorCode &success)
{
    if (LE_FAILURE(success)) {
        return 0;
    }

    if (offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max) {
        success = LE_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    // This is the cheapest way to get mark reordering only for Hebrew.
    // We could just do the mark reordering for all scripts, but most
    // of them probably don't need it... Another option would be to
    // add a HebrewOpenTypeLayoutEngine subclass, but the only thing it
    // would need to do is mark reordering, so that seems like overkill.
    if (fScriptCode == hebrScriptCode) {
        outChars = LE_NEW_ARRAY(LEUnicode, count);

        if (outChars == NULL) {
            success = LE_MEMORY_ALLOCATION_ERROR;
            return 0;
        }

        if (LE_FAILURE(success)) {
            LE_DELETE_ARRAY(outChars);
            return 0;
        }

        CanonShaping::reorderMarks(&chars[offset], count, rightToLeft, outChars, glyphStorage);
    }
    if (LE_FAILURE(success)) {
        return 0;
    }

    glyphStorage.allocateGlyphArray(count, rightToLeft, success);
    glyphStorage.allocateAuxData(success);

    for (le_int32 i = 0; i < count; i += 1) {
        glyphStorage.setAuxData(i, fFeatureMask, (void *) fFeatureParamList, success);
    }

    return count;
}

// Input: characters, tags
// Output: glyphs, char indices
le_int32 OpenTypeLayoutEngine::glyphProcessing(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft,
                                               LEGlyphStorage &glyphStorage, LEErrorCode &success)
{
    if (LE_FAILURE(success)) {
        return 0;
    }

    if (chars == NULL || offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max) {
        success = LE_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    mapCharsToGlyphs(chars, offset, count, rightToLeft, rightToLeft, glyphStorage, success);

    if (LE_FAILURE(success)) {
        return 0;
    }

    if (fGSUBTable != NULL) {
        count = fGSUBTable->process(glyphStorage, rightToLeft, fScriptTag, fLangSysTag, fGDEFTable, fSubstitutionFilter,
                                    fFeatureMap, fFeatureMapCount, fFeatureOrder);
    }

    return count;
}

le_int32 OpenTypeLayoutEngine::glyphPostProcessing(LEGlyphStorage &tempGlyphStorage, LEGlyphStorage &glyphStorage, LEErrorCode &success)
{
    if (LE_FAILURE(success)) {
        return 0;
    }

    glyphStorage.adoptGlyphArray(tempGlyphStorage);
    glyphStorage.adoptCharIndicesArray(tempGlyphStorage);
    glyphStorage.adoptAuxDataArrays(tempGlyphStorage);
    glyphStorage.adoptGlyphCount(tempGlyphStorage);

    return glyphStorage.getGlyphCount();
}

le_int32 OpenTypeLayoutEngine::computeGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft, LEGlyphStorage &glyphStorage, LEErrorCode &success)
{
    LEUnicode *outChars = NULL;
    LEGlyphStorage fakeGlyphStorage;
    le_int32 outCharCount, outGlyphCount, fakeGlyphCount;

    if (LE_FAILURE(success)) {
        return 0;
    }

    if (chars == NULL || offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max) {
        success = LE_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }

    outCharCount = characterProcessing(chars, offset, count, max, rightToLeft, outChars, fakeGlyphStorage, success);

    if (LE_FAILURE(success)) {
        return 0;
    }

    if (outChars != NULL) {
        fakeGlyphCount = glyphProcessing(outChars, 0, outCharCount, outCharCount, rightToLeft, fakeGlyphStorage, success);
        LE_DELETE_ARRAY(outChars); // FIXME: a subclass may have allocated this, in which case this delete might not work...
        //adjustGlyphs(outChars, 0, outCharCount, rightToLeft, fakeGlyphs, fakeGlyphCount);
    } else {
        fakeGlyphCount = glyphProcessing(chars, offset, count, max, rightToLeft, fakeGlyphStorage, success);
        //adjustGlyphs(chars, offset, count, rightToLeft, fakeGlyphs, fakeGlyphCount);
    }

    if (LE_FAILURE(success)) {
        return 0;
    }

    outGlyphCount = glyphPostProcessing(fakeGlyphStorage, glyphStorage, success);

    return outGlyphCount;
}

// apply GPOS table, if any
void OpenTypeLayoutEngine::adjustGlyphPositions(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse,
                                                LEGlyphStorage &glyphStorage, LEErrorCode &success)
{
    if (LE_FAILURE(success)) {
        return;
    }

    if (chars == NULL || offset < 0 || count < 0) {
        success = LE_ILLEGAL_ARGUMENT_ERROR;
        return;
    }

    le_int32 glyphCount = glyphStorage.getGlyphCount();
    if (glyphCount == 0)
        return;

    if (fGPOSTable != NULL) {
        GlyphPositionAdjustments *adjustments = new GlyphPositionAdjustments(glyphCount);
        le_int32 i;

        if (adjustments == NULL) {
            success = LE_MEMORY_ALLOCATION_ERROR;
            return;
        }

#if 0
        // Don't need to do this if we allocate
        // the adjustments array w/ new...
        for (i = 0; i < glyphCount; i += 1) {
            adjustments->setXPlacement(i, 0);
            adjustments->setYPlacement(i, 0);

            adjustments->setXAdvance(i, 0);
            adjustments->setYAdvance(i, 0);

            adjustments->setBaseOffset(i, -1);
        }
#endif

        fGPOSTable->process(glyphStorage, adjustments, reverse, fScriptTag, fLangSysTag, fGDEFTable, fFontInstance,
                            fFeatureMap, fFeatureMapCount, fFeatureOrder);

        float xAdjust = 0, yAdjust = 0;

        for (i = 0; i < glyphCount; i += 1) {
            float xAdvance   = adjustments->getXAdvance(i);
            float yAdvance   = adjustments->getYAdvance(i);
            float xPlacement = 0;
            float yPlacement = 0;


#if 0
            // This is where separate kerning adjustments
            // should get applied.
            xAdjust += xKerning;
            yAdjust += yKerning;
#endif

            for (le_int32 base = i; base >= 0; base = adjustments->getBaseOffset(base)) {
                xPlacement += adjustments->getXPlacement(base);
                yPlacement += adjustments->getYPlacement(base);
            }

            xPlacement = fFontInstance->xUnitsToPoints(xPlacement);
            yPlacement = fFontInstance->yUnitsToPoints(yPlacement);
            glyphStorage.adjustPosition(i, xAdjust + xPlacement, -(yAdjust + yPlacement), success);

            xAdjust += fFontInstance->xUnitsToPoints(xAdvance);
            yAdjust += fFontInstance->yUnitsToPoints(yAdvance);
        }

        glyphStorage.adjustPosition(glyphCount, xAdjust, -yAdjust, success);

        delete adjustments;
    }
    else {
        // if there was no GPOS table, maybe there's non-OpenType kerning we can use
        LayoutEngine::adjustGlyphPositions(chars, offset, count, reverse, glyphStorage, success);
    }

    LEGlyphID zwnj  = fFontInstance->mapCharToGlyph(0x200C);
#if 0
    // The nbsp translation was only here to make one
    // broken font work. Not a good idea in general... 
    LEGlyphID nbsp  = fFontInstance->mapCharToGlyph(0x00A0);
    LEGlyphID space = fFontInstance->mapCharToGlyph(0x0020);
#endif

    if (zwnj != 0x0000) {
        for (le_int32 g = 0; g < glyphCount; g += 1) {
            LEGlyphID glyph = glyphStorage[g];

            if (glyph == zwnj) {
                glyphStorage[g] = LE_SET_GLYPH(glyph, 0xFFFF);
    #if 0
            } else if (glyph == nbsp) {
                glyphStorage[g] = LE_SET_GLYPH(glyph, space);
    #endif
            }
        }
    }

#if 0
    // Don't know why this is here...
    LE_DELETE_ARRAY(fFeatureTags);
    fFeatureTags = NULL;
#endif
}

U_NAMESPACE_END
